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
#include "TableOfContents.h"
#include <QTreeView>
#include "NCXModel.h"
#include "BookManipulation/FolderKeeper.h"

static const int COLUMN_INDENTATION = 10;


TableOfContents::TableOfContents( QWidget *parent )
    : 
    QDockWidget( tr( "Table of Contents" ), parent ),
    m_Book( NULL ),
    m_TreeView( *new QTreeView( this ) ),
    m_NCXModel( *new NCXModel( this ) )
{
    setWidget( &m_TreeView );
    SetupTreeView();

    connect( &m_TreeView,  SIGNAL( clicked(            const QModelIndex& ) ), 
             this,         SLOT(   ItemClickedHandler( const QModelIndex& ) ) );
}


void TableOfContents::SetBook( QSharedPointer< Book > book )
{
    m_Book = book;
    m_NCXModel.SetBook( book );

    Refresh();
}


void TableOfContents::Refresh()
{
    m_NCXModel.Refresh();
}


void TableOfContents::ItemClickedHandler( const QModelIndex &index )
{
    QUrl url         = m_NCXModel.GetUrlForIndex( index );
    QString filename = QFileInfo( url.path() ).fileName();

    try
    {
        Resource &resource = m_Book->GetFolderKeeper().GetResourceByFilename( filename );

        emit OpenResourceRequest( resource, false, url.fragment() );
    }

    catch ( const ResourceDoesNotExist& )
    {
        QMessageBox::critical( 0,
                               tr( "Sigil" ),
                               tr( "The file \"%1\" does not exist." )
                               .arg( filename )
                             );
    }       
}


void TableOfContents::SetupTreeView()
{
    m_TreeView.setEditTriggers( QAbstractItemView::NoEditTriggers );
    m_TreeView.setSortingEnabled( false );
    m_TreeView.sortByColumn( -1 );
    m_TreeView.setUniformRowHeights( true );
    m_TreeView.setDragEnabled( false );
    m_TreeView.setAcceptDrops( false );
    m_TreeView.setDropIndicatorShown( false );
    m_TreeView.setDragDropMode( QAbstractItemView::NoDragDrop );

    m_TreeView.setModel( &m_NCXModel ); 

    m_TreeView.setIndentation( COLUMN_INDENTATION );
    m_TreeView.setHeaderHidden( true );
}

