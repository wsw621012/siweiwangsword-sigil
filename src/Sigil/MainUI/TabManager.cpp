/************************************************************************
**
**  Copyright (C) 2009  Strahinja Markovic
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
#include "TabManager.h"
#include "ContentTab.h"
#include "FlowTab.h"
#include "CSSTab.h"
#include "ImageTab.h"
#include "XPGTTab.h"
#include "../ResourceObjects/Resource.h"

TabManager::TabManager( QWidget *parent )
    : 
    QTabWidget( parent )
{
    connect( this, SIGNAL( currentChanged( int ) ),     this, SLOT( EmitTabChanged() ) );
    connect( this, SIGNAL( tabCloseRequested( int ) ),  this, SLOT( CloseTab( int ) ) );

    setDocumentMode( true );
    setMovable( true );
    setTabsClosable( true );
}

    
ContentTab& TabManager::GetCurrentContentTab()
{
    QWidget* widget = currentWidget();

    // TODO: turn on this assert after you make sure a tab
    // is created before this is called in MainWindow constructor
    //Q_ASSERT( widget != NULL );

    return *qobject_cast< ContentTab* >( widget );
}

void TabManager::OpenResource( Resource& resource, const QUrl &fragment )
{
    if ( SwitchedToExistingTab( resource, fragment ) )

        return;    

    AddNewContentTab( CreateTabForResource( resource, fragment ) );

    // TODO: loading bar update    
}


void TabManager::EmitTabChanged()
{
    ContentTab *current_tab = qobject_cast< ContentTab* >( currentWidget() );

    emit TabChanged( m_LastContentTab.data(), current_tab );

    m_LastContentTab = QWeakPointer< ContentTab >( current_tab );
}

void TabManager::DeleteTab( ContentTab *tab_to_delete )
{
    Q_ASSERT( tab_to_delete );

    removeTab( indexOf( tab_to_delete ) );

    tab_to_delete->deleteLater();
}

void TabManager::CloseTab( int tab_index )
{   
    if ( count() <= 1 )

        return;

    Q_ASSERT( tab_index >= 0 );

    qobject_cast< ContentTab* >( widget( tab_index ) )->Close();
}


// Returns the index of the tab the index is loaded in, -1 if it isn't
int TabManager::ResourceTabIndex( const Resource& resource ) const
{
    QString filename( resource.Filename() );

    int index = -1;

    for ( int i = 0; i < count(); ++i )
    {
        if ( tabText( i ) == filename )
        {
            index = i;
            break;
        }
    }

    return index;
}


bool TabManager::SwitchedToExistingTab( Resource& resource, const QUrl &fragment )
{
    int resource_index = ResourceTabIndex( resource );

    // If the resource is already opened in
    // some tab, then we just switch to it
    if ( resource_index != -1 )
    {
        setCurrentIndex( resource_index );

        FlowTab *flow_tab = qobject_cast< FlowTab* >( widget( resource_index ) );

        if ( flow_tab != NULL )

            flow_tab->ScrollToFragment( fragment.toString() );

        return true;
    }

    return false;
}


ContentTab* TabManager::CreateTabForResource( Resource& resource, const QUrl &fragment )
{
    ContentTab *tab = NULL;

    if ( resource.Type() == Resource::HTMLResource )
    {
        tab = new FlowTab( resource, fragment, this );

        connect( tab, SIGNAL( LinkClicked( const QUrl& ) ), this, SIGNAL( OpenUrlRequest( const QUrl& ) ) );
    }

    else if ( resource.Type() == Resource::CSSResource )
    {
        tab = new CSSTab( resource, this );
    }

    else if ( resource.Type() == Resource::XPGTResource )
    {
        tab = new XPGTTab( resource, this );
    }

    else if ( resource.Type() == Resource::ImageResource )
    {
        tab = new ImageTab( resource, this );
    }

    return tab;    
}


bool TabManager::AddNewContentTab( ContentTab *new_tab )
{
    if ( new_tab == NULL )

        return false;

    addTab( new_tab, new_tab->GetIcon(), new_tab->GetFilename() );
    setCurrentWidget( new_tab );
    connect( new_tab, SIGNAL( DeleteMe( ContentTab* ) ), this, SLOT( DeleteTab( ContentTab* ) ) );

    return true;
}   

