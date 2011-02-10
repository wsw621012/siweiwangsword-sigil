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

#pragma once
#ifndef OPFMODEL_H
#define OPFMODEL_H

#include <QStandardItemModel>
#include <QSharedPointer>
#include "ResourceObjects/Resource.h"
#include "BookManipulation/Book.h"

class QModelIndex;
class QStandardItem;


/**
 * A hierarchical model for the OPF structure and files.
 * Meant to be used with a Qt View class (like QTreeView).
 */
class OPFModel : public QStandardItemModel
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent The model's parent.
     */
    OPFModel( QWidget *parent = 0 );

    /**
     * Sets the model's book.
     *
     * @param book The book whose model we will be building.
     */
    void SetBook( QSharedPointer< Book > book );

    /**
     * Forces the recreation of the model
     * from the information in the stored book.
     */
    void Refresh();

    /**
     * Returns the QModelIndex of the first HTML file.
     *
     * @return The QModelIndex of the first HTML file.
     * @throws NoHTMLFiles()
     */
    QModelIndex GetFirstHTMLModelIndex();

    /**
     * Returns the QModelIndex of the Text folder.
     *
     * @return The QModelIndex of the Text folder.
     */
    QModelIndex GetTextFolderModelIndex();

    /**
     * Gets an item's resource type.
     *
     * @param item THe item whose resource type we want.
     * @return The requested resource type.
     */
    Resource::ResourceType GetResourceType( QStandardItem const *item );

    /**
     * Kills the inherited sort function.
     * If this function is left standing, then the user could
     * sort the items with clicks in a View. We don't want that
     * because the HTML files are \em always sorted by their
     * reading order, and the other files by their filenames.
     * 
     * @param column The column to sort.
     * @param order The type of sorting needed.
     */
    virtual void sort( int column, Qt::SortOrder order = Qt::AscendingOrder );

    /**
     * Specifies the supported drop actions in the model.
     *
     * @return The drop actions supported.
     */
    virtual Qt::DropActions supportedDropActions() const;

private slots:

    /**
     * Handler for removed rows. Used for updating HTMLResource
     * reading orders when the user reorders them in a View.
     * 
     * @param parent The parent model index in which rows were removed.
     * @param start The start index of the removed rows,
     * @param end The end index of the removed rows.
     */
    void RowsRemovedHandler( const QModelIndex &parent, int start, int end );

    /**
     * Handler for changed items. Used for handling item renames,
     * and \em only item renames.
     *
     * @param item The item that was changed.
     */
    void ItemChangedHandler( QStandardItem *item );

private:

    /**
     * Initializes an empty model with data. It is filled 
     * using information from the stored book.
     */
    void InitializeModel();

    /**
     * Updates the reading orders of the HTMLResources
     * with their order in the model.
     */
    void UpdateHTMLReadingOrders();

    /**
     * Sorts \em all files by their filenames, alphabetically.
     */
    void SortFilesByFilenames();

    /**
     * Sorts the HTML files by their reading orders.
     */
    void SortHTMLFilesByReadingOrder();

    /**
     * Removes all data from the model.
     */
    void ClearModel();

    /**
     * Determines if a filename is valid. If it is not,
     * an error dialog is presented to the user informing
     * him of the reason why.
     *
     * @param old_filename The old filename of the file.
     * @param new_filename The requested new filename of the file.
     * @return \c true if the filename is valid.
     */
    bool FilenameIsValid( const QString &old_filename, const QString &new_filename );


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    /**
     * Used to prevent the RowsRemovedHandler
     * from messing up the reading orders.
     * Is \c true while the model is refreshing.
     */
    bool m_Refreshing;

    /**
     * The book whose model we are representing.
     */
    QSharedPointer< Book > m_Book;

    QStandardItem &m_TextFolderItem;   /**< The Text folder item. */
    QStandardItem &m_StylesFolderItem; /**< The Styles folder item. */
    QStandardItem &m_ImagesFolderItem; /**< The Images folder item. */
    QStandardItem &m_FontsFolderItem;  /**< The Fonts folder item. */
    QStandardItem &m_MiscFolderItem;   /**< The Misc folder item. */
};


#endif // OPFMODEL_H
