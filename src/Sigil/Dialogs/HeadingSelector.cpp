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
#include "HeadingSelector.h"
#include "BookManipulation/Book.h"
#include "Misc/Utility.h"
#include "ResourceObjects/HTMLResource.h"
#include "BookManipulation/XhtmlDoc.h"
#include "BookManipulation/XercesCppUse.h"
#include "BookManipulation/FolderKeeper.h"

static const QString SETTINGS_GROUP   = "heading_selector";
static const int FIRST_COLUMN_PADDING = 30;

// Constructor;
// the first parameter is the book whose TOC
// is being edited, the second is the dialog's parent
HeadingSelector::HeadingSelector( QSharedPointer< Book > book, QWidget *parent )
    : 
    QDialog( parent ),
    m_Book( book )
{
    ui.setupUi( this );
    ConnectSignalsToSlots();

    ui.tvTOCDisplay->setModel( &m_TableOfContents );

    LockHTMLResources();

    m_Headings = Headings::GetHeadingList(
        m_Book->GetFolderKeeper().GetResourceTypeList< HTMLResource >( true ), true );
    m_Headings = Headings::MakeHeadingHeirarchy( m_Headings );

    CreateTOCModel();

    // Set the initial display state
    if ( ui.cbTOCItemsOnly->checkState() == Qt::Checked ) 

        RemoveExcludedItems( m_TableOfContents.invisibleRootItem() );

    UpdateTreeViewDisplay();
    ReadSettings();
}


// Destructor
HeadingSelector::~HeadingSelector()
{
    WriteSettings();

    UnlockHTMLResources();
}


// We need to filter the calls to functions that would normally
// connect directly to the itemChanged( QStandardItem* ) signal
// because some would-be slots delete the item in question.
// So, the signal connects here and this function calls the 
// appropriate item-handling functions. 
void HeadingSelector::ModelItemFilter( QStandardItem *item )
{
    Q_ASSERT( item );

    if ( item->isCheckable() == true )
    
        UpdateHeadingInclusion( item );
}


// Switches the display between showing all headings
// and showing only headings that are to be included in the TOC
void HeadingSelector::ChangeDisplayType(  int new_check_state  )
{
    // If checked, show only TOC items
    if ( new_check_state == Qt::Checked )
    {
        RemoveExcludedItems( m_TableOfContents.invisibleRootItem() );
    }

    // If unchecked, show all items
    else
    {
        CreateTOCModel();
        UpdateTreeViewDisplay();       
    }
}


void HeadingSelector::UpdateHeadingElements()
{
    // We recreate the model to make sure even those
    // headings marked as "don't include" are in the model.
    CreateTOCModel();

    UpdateOneHeadingElement( m_TableOfContents.invisibleRootItem() );
}

void HeadingSelector::UpdateOneHeadingElement( QStandardItem *item )
{
    Headings::Heading *heading = GetItemHeading( item );

    if ( heading != NULL )
    {       
        // Update heading inclusion: if a heading element
        // has the NOT_IN_TOC_CLASS class, then it's not in the TOC
        QString class_attribute = XtoQ( heading->element->getAttribute( QtoX( "class" ) ) )
                                  .remove( NOT_IN_TOC_CLASS )
                                  .simplified();

        if ( !heading->include_in_toc )
       
            class_attribute = class_attribute.append( " " + NOT_IN_TOC_CLASS ).simplified();

        if ( !class_attribute.isEmpty() )

            heading->element->setAttribute( QtoX( "class" ), QtoX( class_attribute ) );

        else
            
            heading->element->removeAttribute( QtoX( "class" ) );
        
        heading->resource_file->MarkSecondaryCachesAsOld();
    }

    if ( item->hasChildren() == true )
    {
        for ( int i = 0; i < item->rowCount(); ++i )
        {
            UpdateOneHeadingElement( item->child( i ) );                
        }
    }
}


// Updates the inclusion of the heading in the TOC
// whenever that heading's "include in TOC" checkbox
// is checked/unchecked. 
void HeadingSelector::UpdateHeadingInclusion( QStandardItem *checkbox_item )
{
    Q_ASSERT( checkbox_item );

    QStandardItem *item_parent = GetActualItemParent( checkbox_item );
    Headings::Heading *heading = GetItemHeading( item_parent->child( checkbox_item->row(), 0 ) );   
    Q_ASSERT( heading );

    if ( checkbox_item->checkState() == Qt::Unchecked )

        heading->include_in_toc = false;

    else

        heading->include_in_toc = true;

    if ( ui.cbTOCItemsOnly->checkState() == Qt::Checked ) 

        RemoveExcludedItems( m_TableOfContents.invisibleRootItem() );    
}



// Updates the display of the tree view
// (resizes columns etc.)
void HeadingSelector::UpdateTreeViewDisplay()
{      
    ui.tvTOCDisplay->expandAll();
    ui.tvTOCDisplay->resizeColumnToContents( 0 );
    ui.tvTOCDisplay->setColumnWidth( 0, ui.tvTOCDisplay->columnWidth( 0 ) + FIRST_COLUMN_PADDING );    
}


// Creates the model that is displayed
// in the tree view 
void HeadingSelector::CreateTOCModel()
{
    m_TableOfContents.clear();

    QStringList header;

    header.append( tr( "TOC Entry" ) );
    header.append( tr( "Include" ) );

    m_TableOfContents.setHorizontalHeaderLabels( header );

    // Recursively inserts all headings
    for ( int i = 0; i < m_Headings.count(); ++i )
    {
        InsertHeadingIntoModel( m_Headings[ i ], m_TableOfContents.invisibleRootItem() );
    }
}


// Inserts the specified heading into the model
// as the child of the specified parent item;
// recursively calls itself on the headings children,
// thus building a TOC tree
void HeadingSelector::InsertHeadingIntoModel( Headings::Heading &heading, QStandardItem *parent_item )
{
    Q_ASSERT( parent_item );

    QStandardItem *item_heading           = new QStandardItem( heading.text );
    QStandardItem *heading_included_check = new QStandardItem();

    heading_included_check->setEditable( false );
    heading_included_check->setCheckable( true );
    item_heading->setEditable( false );
    item_heading->setDragEnabled( false );
    item_heading->setDropEnabled( false );

    if ( heading.include_in_toc == true )

        heading_included_check->setCheckState( Qt::Checked );

    else

        heading_included_check->setCheckState( Qt::Unchecked );

    // Storing a pointer to the heading that
    // is represented by this QStandardItem
    Headings::HeadingPointer wrap;
    wrap.heading = &heading;

    item_heading->setData( QVariant::fromValue( wrap ) );

    QList< QStandardItem* > items;        
    items << item_heading << heading_included_check;

    parent_item->appendRow( items );

    if ( !heading.children.isEmpty() )
    {
        for ( int i = 0; i < heading.children.count(); ++i )
        {
            InsertHeadingIntoModel( heading.children[ i ], item_heading );
        }
    }
}


// Removes from the tree items that represent headings
// that are not to be included in the TOC; the children
// of those items rise to their parent's hierarchy level
// OR (more likely) are attached as children to the first
// previous heading that is lower in level.
void HeadingSelector::RemoveExcludedItems( QStandardItem *item )
{
    Q_ASSERT( item );

    // Recursively call itself on the item's children
    if ( item->hasChildren() == true )
    {
        int row_index = 0;

        while ( row_index < item->rowCount() )
        {
            QStandardItem *oldchild = item->child( row_index );

            RemoveExcludedItems( item->child( row_index ) );

            // We only increment the row_index if the
            // RemoveExcludedItems operation didn't end up
            // removing the child at that index.. if it did,
            // the next child is now at that index.
            if ( oldchild == item->child( row_index ) )
            
                row_index++;
        }
    }

    // The root item is always present
    if ( item == m_TableOfContents.invisibleRootItem() )

        return;
        
    QStandardItem *item_parent = GetActualItemParent( item );

    // We query the "include in TOC" checkbox
    Qt::CheckState check_state = item_parent->child( item->row(), 1 )->checkState();

    // We remove the current item if it shouldn't
    // be included in the TOC
    if ( check_state == Qt::Unchecked )
    {
        if ( item->hasChildren() == true )
        {
            while ( item->rowCount() > 0 )
            {
                QList< QStandardItem* > child_row = item->takeRow( 0 ); 

                if ( !AddRowToVisiblePredecessorSucceeded( child_row, item ) )

                    item_parent->insertRow( item->row(), child_row );
            }
        }

        // Item removes itself
        item_parent->removeRow( item->row() );
    }
}


bool HeadingSelector::AddRowToVisiblePredecessorSucceeded( const QList< QStandardItem* > &child_row,
                                                     QStandardItem* row_parent )
{
    Q_ASSERT( row_parent );
    QStandardItem *row_grandparent = GetActualItemParent( row_parent );
    
    if ( row_grandparent == NULL )

        return false;

    return AddRowToCorrectItem( row_grandparent, child_row, row_parent->row() );    
}


// Basically we're looking for the first heading of a lower level that comes
// before the child_row heading whose parent heading is disappearing. The new parent
// needs to also be marked as "include_in_toc".
bool HeadingSelector::AddRowToCorrectItem( QStandardItem* item,                                       
                                     const QList< QStandardItem* > &child_row,
                                     int child_index_limit )
{
    int child_start_index = child_index_limit != -1 ? child_index_limit - 1: item->rowCount() - 1;

    for ( int i = child_start_index; i > -1; --i )
    {
        bool row_placed = AddRowToCorrectItem( item->child( i ), child_row );
        if ( row_placed )

            return true;
    }

    Headings::Heading *heading = GetItemHeading( item );

    if ( heading == NULL )

        return false;

    Headings::Heading *child_heading = GetItemHeading( child_row[ 0 ] );
    
    if ( heading->include_in_toc &&
         heading->level < child_heading->level )
    {
        item->insertRow( child_start_index + 1, child_row );
        return true;
    }

    return false;
}


//    Unfortunately, while the invisible root of a QStandardItemModel
// can have children, those children do not have a parent set.
// Of course, their parent is the invisible root, but their
// parent() function returns 0. So now you have *two* tree levels
// for which parent() returns 0. This is clearly inconsistent
// and makes the whole idea of using the same recursive
// functions for tree traversal rather difficult to implement.
// The only item for which parent() should return 0 should be 
// the invisible root.
//    Admittedly, Qt has some design issues. The next few lines
// try to work around them by manually setting an item parent.
QStandardItem* HeadingSelector::GetActualItemParent( const QStandardItem *item )
{
    Q_ASSERT( item );

    if ( item == m_TableOfContents.invisibleRootItem() )

        return NULL;

    if ( item->parent() == 0 )
    
        return m_TableOfContents.invisibleRootItem();
    
    return item->parent();
}


// CAN RETURN NULL!
Headings::Heading* HeadingSelector::GetItemHeading( const QStandardItem *item )
{
    Q_ASSERT( item );

    if ( item == m_TableOfContents.invisibleRootItem() )

        return NULL;

    Headings::Heading *heading = item->data().value< Headings::HeadingPointer >().heading;

    return heading;
}


// Reads all the stored dialog settings like
// window position, geometry etc.
void HeadingSelector::ReadSettings()
{
    QSettings settings;
    settings.beginGroup( SETTINGS_GROUP );

    // The size of the window and it's full screen status
    QByteArray geometry = settings.value( "geometry" ).toByteArray();

    if ( !geometry.isNull() )
    
        restoreGeometry( geometry );
}


// Writes all the stored dialog settings like
// window position, geometry etc.
void HeadingSelector::WriteSettings()
{
    QSettings settings;
    settings.beginGroup( SETTINGS_GROUP );

    // The size of the window and it's full screen status
    settings.setValue( "geometry", saveGeometry() );
}


void HeadingSelector::LockHTMLResources()
{
    foreach( HTMLResource* resource, m_Book->GetFolderKeeper().GetResourceTypeList< HTMLResource >( true ) )
    {
        resource->GetLock().lockForWrite();
    }
}


void HeadingSelector::UnlockHTMLResources()
{
    foreach( HTMLResource* resource, m_Book->GetFolderKeeper().GetResourceTypeList< HTMLResource >( true ) )
    {
        resource->GetLock().unlock();
    }
}


void HeadingSelector::ConnectSignalsToSlots()
{
    connect( &m_TableOfContents, SIGNAL( itemChanged( QStandardItem* ) ),
             this,               SLOT(   ModelItemFilter( QStandardItem* ) ) 
           );

    connect( ui.cbTOCItemsOnly,  SIGNAL( stateChanged( int ) ),
             this,               SLOT(   ChangeDisplayType( int ) ) 
           );

    connect( this,               SIGNAL( accepted() ),
             this,               SLOT(   UpdateHeadingElements() ) 
             );

    connect( this,               SIGNAL( accepted() ),
             m_Book.data(),      SLOT(   SetModified() ) 
             );
}


