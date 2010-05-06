/************************************************************************
**
**  Copyright (C) 2009, 2010  Strahinja Markovic
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

#pragma once
#ifndef BOOKBROWSER_H
#define BOOKBROWSER_H

#include <QDockWidget>
#include <QSharedPointer>
#include "../BookManipulation/Book.h"
#include "../ResourceObjects/Resource.h"

class QTreeView;
class OPFModel;
class HTMLResource;
class Resource;
class QModelIndex;
class QUrl;
class QPoint;
class QMenu;
class QAction;
class QSignalMapper;


/**
 * Represents the pane with which the user can manipulate the book's files.
 * The user uses this browser to open the files in tabs. Some files can then
 * be edited, others cannot.
 */
class BookBrowser : public QDockWidget
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent The QObject's parent.
     */
    BookBrowser( QWidget *parent = 0 );

    /**
     * Destructor.
     */
    ~BookBrowser();

public slots:

    /**
     * Sets the book whose contents the browser should display.
     *
     * @param book The book to be displayed.
     */
    void SetBook( QSharedPointer< Book > book );

    /**
     * Refreshes the display of the book.
     * This is done by clearing the current content
     * and reloading the file list from the book.
     */
    void Refresh();

    /**
     * Opens the HTML resource referenced by the specified URL.
     *
     * @param url The URL to open.
     */
    void OpenUrlResource( const QUrl &url );

signals:

    /**
     * Emitted when the user double-clicks a resource in the browser.
     *
     * @param resource The clicked resource.
     */
    void ResourceDoubleClicked( Resource &resource );

    /**
     * Emitted when the browser wants a resource to be opened.
     *
     * @param resource The resource that should be opened.
     * @param precede_current_tab Should the new tab precede the currently opened one.
     * @param fragment The fragment ID to which the new tab should be scrolled.
     */
    void OpenResourceRequest( Resource &resource, bool precede_current_tab, const QUrl &fragment );

private slots:

    /**
     * Emits the ResourceDoubleClicked signal.
     *
     * @param The clicked model index in the Tree View.
     */
    void EmitResourceDoubleClicked( const QModelIndex &index );

    /**
     * Opens the context menu at the requested point.
     *
     * @param point The point at which the menu should be opened.
     */
    void OpenContextMenu( const QPoint &point );

    /**
     * Implements the Add New context menu action functionality.
     */
    void AddNew();

    /**
     * Implements the Add Existing context menu action functionality.
     */
    void AddExisting();

    /**
     * Implements the Rename context menu action functionality.
     */
    void Rename();

    /**
     * Implements the Add New context menu action functionality.
     */
    void Remove();

    /**
     * Implements the Cover Image semantic context menu action functionality.
     */
    void SetCoverImage();

    /**
     * Adds the semantic type information to the currently
     * selected resource.
     *
     * @param type The integer value of the GuideSemantics::GuideSemanticType
     *             to be added to the resource.
     */
    void AddGuideSemanticType( int type );

private:

    /**
     * Expands the Text folder so that all HTML files are shown.
     */
    void ExpandTextFolder();    

    /**
     * Reads all the stored application settings like
     * window position, geometry etc.
     */
    void ReadSettings();

    /**
     * Writes all the stored application settings like
     * window position, geometry etc.
     */
    void WriteSettings();

    /**
     * Performs the initial setup for the tree view. 
     */
    void SetupTreeView();

    /**
     * Creates all the context menu actions.
     */
    void CreateContextMenuActions();

    /**
     * Creates all the actions for the <guide>
     * element semantics.
     */
    void CreateGuideSemanticActions();

    /**
     * Tries to setup the context menu for the specified point,
     * and returns true on success.
     *
     * @param point The point at which the menu should be opened.
     * @return \c true if the menu could be set up.
     */
    bool SuccessfullySetupContextMenu( const QPoint &point );

    /**
     * Sets up the sub-menu for adding semantic information.
     *
     * @param resource The resource on which the context menu was invoked.
     */
    void SetupSemanticContextmenu( Resource *resource );

    /**
     * Sets up the sub-menu for adding semantic information,
     * when the menu is invoked for HTMLResources.
     *
     * @param resource The resource on which the context menu was invoked.
     */
    void SetupHTMLSemanticContextMenu( Resource *resource );

    /**
     * Sets up the sub-menu for adding semantic information,
     * when the menu is invoked for ImageResources.
     *
     * @param resource The resource on which the context menu was invoked.
     */
    void SetupImageSemanticContextMenu( Resource *resource ); 


    /**
     * Sets the checked state for the HTML semantic actions
     * based on the resource's current state.
     *
     * @param resource The resource on which the context menu was invoked.
     */
    void SetHTMLSemanticActionCheckState( Resource *resource );

    /**
     * Returns the currently selected resource in the tree view.
     *
     * @return The currently selected resource in the tree view,
     *         or NULL if no resource is selected. 
     */
    Resource* GetCurrentResource();

    /**
     * Connects all the required signals to their respective slots.
     */
    void ConnectSignalsToSlots();


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    /**
     * The bool currently being displayed.
     */
    QSharedPointer< Book > m_Book;

    /**
     * The tree view used to represent the book's files.
     */
    QTreeView &m_TreeView;
    
    /**
     * The data model used to feed the tree view.
     */
    OPFModel &m_OPFModel;

    /**
     * The right-click context menu.
     */
    QMenu &m_ContextMenu;

    /**
     * The sub-menu for adding semantic
     * information to resources.
     */
    QMenu &m_SemanticsContextMenu;

    // The context menu actions.

    QAction *m_AddExisting;
    QAction *m_AddNew;
    QAction *m_Rename;
    QAction *m_Remove;
    QAction *m_CoverImage;

    /**
     * All the semantic actions for the <guide>
     * element. Only present on HTMLResources.
     */
    QList< QAction* > m_GuideSemanticActions;

    /**
     * Used to translate all the triggered() signals from the
     * various guide-related semantic actions into calls to
     * AddGuideSemanticType().
     */
    QSignalMapper &m_GuideSemanticMapper;

    /**
     * The resource type of the last item on which the 
     * the context menu was invoked.
     */
    Resource::ResourceType m_LastContextMenuType;

    /**
     * The last folder from which the user
     * added an existing file.
     */
    QString m_LastFolderOpen;
};

#endif // BOOKBROWSER_H


