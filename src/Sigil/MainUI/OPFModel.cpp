/************************************************************************
**
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
**
**  This file is part of Sigil.
**
**  Sigil is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  Sigil is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Sigil.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/

#include <stdafx.h>
#include "OPFModel.h"
#include "BookManipulation/Book.h"
#include "ResourceObjects/Resource.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/OPFResource.h"
#include "ResourceObjects/NCXResource.h"
#include "SourceUpdates/UniversalUpdates.h"
#include "BookManipulation/FolderKeeper.h"
#include "Misc/Utility.h"
#include <limits>

static const int NO_READING_ORDER   = std::numeric_limits< int >::max();
static const int READING_ORDER_ROLE = Qt::UserRole + 2;
static const QList< QChar > FORBIDDEN_FILENAME_CHARS = QList< QChar >() << '<' << '>' << ':' 
                                                                        << '"' << '/' << '\\'
                                                                        << '|' << '?' << '*';

OPFModel::OPFModel( QWidget *parent )
    : 
    QStandardItemModel( parent ),
    m_RefreshInProgress( false ),
    m_Book( NULL ),
    m_TextFolderItem(   *new QStandardItem( TEXT_FOLDER_NAME  ) ),
    m_StylesFolderItem( *new QStandardItem( STYLE_FOLDER_NAME ) ),
    m_ImagesFolderItem( *new QStandardItem( IMAGE_FOLDER_NAME ) ),
    m_FontsFolderItem(  *new QStandardItem( FONT_FOLDER_NAME  ) ),
    m_MiscFolderItem(   *new QStandardItem( MISC_FOLDER_NAME  ) )
{
    connect( this, SIGNAL( rowsRemoved(        const QModelIndex&, int, int ) ),
             this, SLOT(   RowsRemovedHandler( const QModelIndex&, int, int ) ) );  

    connect( this, SIGNAL( itemChanged(        QStandardItem* ) ),
             this, SLOT(   ItemChangedHandler( QStandardItem* ) ) );   

    QList< QStandardItem* > items;

    items.append( &m_TextFolderItem   );
    items.append( &m_StylesFolderItem );
    items.append( &m_ImagesFolderItem );
    items.append( &m_FontsFolderItem  );
    items.append( &m_MiscFolderItem   );

    QIcon folder_icon = QFileIconProvider().icon( QFileIconProvider::Folder );

    foreach( QStandardItem *item, items )
    {
        item->setIcon( folder_icon );
        item->setEditable( false );
        item->setDragEnabled( false );
        item->setDropEnabled( false );
        appendRow( item );
    }    

    // We enable reordering of files in the text folder
    m_TextFolderItem.setDropEnabled( true );
    invisibleRootItem()->setDropEnabled( false );
}


void OPFModel::SetBook( QSharedPointer< Book > book )
{
    m_Book = book;

    connect( this, SIGNAL( BookContentModified() ), m_Book.data(), SLOT( SetModified() ) );

    Refresh();
}


void OPFModel::Refresh()
{
    m_RefreshInProgress = true;

    InitializeModel();

    SortFilesByFilenames();
    SortHTMLFilesByReadingOrder();

    m_RefreshInProgress = false;
}


QModelIndex OPFModel::GetFirstHTMLModelIndex()
{
    if ( !m_TextFolderItem.hasChildren() )

        boost_throw( NoHTMLFiles() );

    return m_TextFolderItem.child( 0 )->index();
}


QModelIndex OPFModel::GetTextFolderModelIndex()
{
    return m_TextFolderItem.index();
}


Resource::ResourceType OPFModel::GetResourceType( QStandardItem const *item )
{
    Q_ASSERT( item );

    if ( item == &m_TextFolderItem )

        return Resource::HTMLResourceType;

    if ( item == &m_StylesFolderItem )

        return Resource::CSSResourceType;

    if ( item == &m_ImagesFolderItem )

        return Resource::ImageResourceType;

    if ( item == &m_FontsFolderItem )

        return Resource::FontResourceType;

    if ( item == &m_MiscFolderItem )

        return Resource::GenericResourceType;  
    
    const QString &identifier = item->data().toString();
    return m_Book->GetFolderKeeper().GetResourceByIdentifier( identifier ).Type();
}


void OPFModel::sort( int column, Qt::SortOrder order )
{
    return;
}


Qt::DropActions OPFModel::supportedDropActions() const
{
    return Qt::MoveAction;
}


//   This function initiates HTML reading order updating when the user
// moves the HTML files in the Book Browser.
//   You would expect the use of QAbstractItemModel::rowsMoved, but that
// signal is never emitted because in QStandardItemModel, row moves
// are actually handled by creating a copy of the row in the new position
// and then deleting the old row. 
//   Yes, it's stupid and violates the guarantees of the QAbstractItemModel
// class. Oh and it's not documented anywhere. Nokia FTW.
//   This also handles actual HTML item deletion.
void OPFModel::RowsRemovedHandler( const QModelIndex &parent, int start, int end )
{
    if ( m_RefreshInProgress || 
         itemFromIndex( parent ) != &m_TextFolderItem )
    {
        return;
    }

    UpdateHTMLReadingOrders();
}


void OPFModel::ItemChangedHandler( QStandardItem *item )
{
    Q_ASSERT( item );

    const QString &identifier = item->data().toString(); 

    if ( identifier.isEmpty() )

        return;

    Resource *resource = &m_Book->GetFolderKeeper().GetResourceByIdentifier( identifier );
    Q_ASSERT( resource );

    const QString &old_fullpath = resource->GetFullPath();
    const QString &old_filename = resource->Filename();
    const QString &new_filename = item->text();
    
    if ( old_filename == new_filename || 
         !FilenameIsValid( old_filename, new_filename )   )
    {
        item->setText( old_filename );
        return;
    }

    bool rename_sucess = resource->RenameTo( new_filename );

    if ( !rename_sucess )
    {
        Utility::DisplayStdErrorDialog( 
            tr( "The file could not be renamed." )
            );

        item->setText( old_filename );
        return;
    }

    QHash< QString, QString > update;
    update[ old_fullpath ] = "../" + resource->GetRelativePathToOEBPS();

    QApplication::setOverrideCursor( Qt::WaitCursor );
    UniversalUpdates::PerformUniversalUpdates( true, m_Book->GetFolderKeeper().GetResourceList(), update );
    QApplication::restoreOverrideCursor();

    emit BookContentModified();
}


void OPFModel::InitializeModel()
{
    Q_ASSERT( m_Book );

    ClearModel();

    QList< Resource* > resources = m_Book->GetFolderKeeper().GetResourceList();

    foreach( Resource *resource, resources )
    {
        QStandardItem *item = new QStandardItem( resource->Icon(), resource->Filename() );
        item->setDropEnabled( false );
        item->setData( resource->GetIdentifier() );
        
        if ( resource->Type() == Resource::HTMLResourceType )
        {
            int reading_order = 
                m_Book->GetOPF().GetReadingOrder( *qobject_cast< HTMLResource* >( resource ) );

            if ( reading_order == -1 )
            
                reading_order = NO_READING_ORDER;

            item->setData( reading_order, READING_ORDER_ROLE );
            m_TextFolderItem.appendRow( item );
        }

        else if ( resource->Type() == Resource::CSSResourceType || 
                  resource->Type() == Resource::XPGTResourceType 
                )
        {
            m_StylesFolderItem.appendRow( item );
        }

        else if ( resource->Type() == Resource::ImageResourceType )
        {
            m_ImagesFolderItem.appendRow( item );
        }

        else if ( resource->Type() == Resource::FontResourceType )
        {
            m_FontsFolderItem.appendRow( item );
        }

        else if ( resource->Type() == Resource::OPFResourceType || 
                  resource->Type() == Resource::NCXResourceType )
        {
            item->setEditable( false );
            appendRow( item );
        }

        else
        {
            m_MiscFolderItem.appendRow( item );        
        }
    }           
}


void OPFModel::UpdateHTMLReadingOrders()
{
    QList< HTMLResource* > reading_order_htmls;

    for ( int i = 0; i < m_TextFolderItem.rowCount(); ++i )
    {
        QStandardItem *html_item = m_TextFolderItem.child( i );

        Q_ASSERT( html_item );

        html_item->setData( i, READING_ORDER_ROLE );
        HTMLResource *html_resource =  qobject_cast< HTMLResource* >(
            &m_Book->GetFolderKeeper().GetResourceByIdentifier( html_item->data().toString() ) );

        if ( html_resource != NULL )
                
            reading_order_htmls.append( html_resource );        
    }

    m_Book->GetOPF().UpdateSpineOrder( reading_order_htmls );
}


void OPFModel::ClearModel()
{
    while ( m_TextFolderItem.rowCount() != 0 )
    {
        m_TextFolderItem.removeRow( 0 );
    }

    while ( m_StylesFolderItem.rowCount() != 0 )
    {
        m_StylesFolderItem.removeRow( 0 );
    }

    while ( m_ImagesFolderItem.rowCount() != 0 )
    {
        m_ImagesFolderItem.removeRow( 0 );
    }

    while ( m_FontsFolderItem.rowCount() != 0 )
    {
        m_FontsFolderItem.removeRow( 0 );
    }

    while ( m_MiscFolderItem.rowCount() != 0 )
    {
        m_MiscFolderItem.removeRow( 0 );
    }

    int i = 0; 
    while ( i < invisibleRootItem()->rowCount() )
    {
        QStandardItem *child = invisibleRootItem()->child( i, 0 );

        if ( child != &m_TextFolderItem   &&
             child != &m_StylesFolderItem &&
             child != &m_ImagesFolderItem &&
             child != &m_FontsFolderItem  &&
             child != &m_MiscFolderItem )
        {
            invisibleRootItem()->removeRow( i );
        }

        else
        {
            ++i;
        }
    }
}


void OPFModel::SortFilesByFilenames()
{
    for ( int i = 0; i < invisibleRootItem()->rowCount(); ++i )
    {
        invisibleRootItem()->child( i )->sortChildren( 0 );
    }
}


void OPFModel::SortHTMLFilesByReadingOrder()
{
    int old_sort_role = sortRole();
    setSortRole( READING_ORDER_ROLE );

    m_TextFolderItem.sortChildren( 0 );

    setSortRole( old_sort_role );
}


bool OPFModel::FilenameIsValid( const QString &old_filename, const QString &new_filename )
{
    if ( new_filename.isEmpty() )
    {
        Utility::DisplayStdErrorDialog( 
            tr( "The filename cannot be empty." )
            );

        return false;
    }

    foreach( QChar character, new_filename )
    {
        if ( FORBIDDEN_FILENAME_CHARS.contains( character ) )
        {
            Utility::DisplayStdErrorDialog( 
                tr( "A filename cannot contains the character \"%1\"." )
                .arg( character )
                );

            return false;
        }
    }

    const QString &old_extension = QFileInfo( old_filename ).suffix();
    const QString &new_extension = QFileInfo( new_filename ).suffix();

    // We normally don't allow an extension change, but we
    // allow it for changes between HTML, HTM, XHTML and XML.
    if ( old_extension != new_extension &&
         !( TEXT_EXTENSIONS.contains( old_extension ) && TEXT_EXTENSIONS.contains( new_extension ) )
       )
    {
        Utility::DisplayStdErrorDialog( 
            tr( "This file's extension cannot be changed in that way.\n"
                "You used \"%1\", and the old extension was \"%2\"." )
            .arg( new_extension )
            .arg( old_extension )
            );

        return false;
    }
    
    if ( new_filename != m_Book->GetFolderKeeper().GetUniqueFilenameVersion( new_filename ) )
    {
        Utility::DisplayStdErrorDialog( 
            tr( "The filename \"%1\" is already in use.\n" )
            .arg( new_filename )
            );

        return false;
    }

    return true;
}


