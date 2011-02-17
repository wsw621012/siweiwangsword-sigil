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
#ifndef VALIDATIONRESULTSVIEW_H
#define VALIDATIONRESULTSVIEW_H

#include <QDockWidget>
#include <vector>
#include <QSharedPointer>
#include "BookManipulation/Book.h"
#include "Tabs/ContentTab.h"

namespace FlightCrew { class Result; };
namespace fc = FlightCrew;

class QTableWidget;
class QTableWidgetItem;

/**
 * Represents the pane in which all the validation results are displayed.
 */
class ValidationResultsView : public QDockWidget
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent The QObject's parent.
     */
    ValidationResultsView( QWidget *parent = 0 );

    /**
     * Validates the epub file given and displays the results.
     */
    void ValidateCurrentBook();

    /**
     * Clears the result table.
     */
    void ClearResults();

public slots:

    /**
     * Sets the book that should be validated.
     *
     * @param book The book to be validated.
     */
    void SetBook( QSharedPointer< Book > book );

signals:

    /**
     * Emitted when the user clicks a result and thus wants
     * a resource to be opened on that error.
     *
     * @param resource The resource that should be opened.
     * @param precede_current_tab Should the new tab precede the currently opened one.
     * @param fragment The fragment ID to which the new tab should be scrolled.
     * @param view_state In which View should the resource open or switch to.
     * @param line_to_scroll_to To which line should the resource scroll.
     */
    void OpenResourceRequest( Resource &resource, 
                              bool precede_current_tab, 
                              const QUrl &fragment,
                              ContentTab::ViewState view_state,
                              int line_to_scroll_to );

private slots:

    /**
     * Handles double-clicking on a result in the table.
     *
     * @param item The item that was clicked.
     */
    void ResultDoubleClicked( QTableWidgetItem *item );

private:

    /**
     * Sets up the table widget to our liking.
     */
    void SetUpTable();

    /**
     * Displays the given results in the widget's table.
     *
     * @param results A list of FlightCrew validation results.
     */
    void DisplayResults( const std::vector< fc::Result > &results );

    /**
     * Informs the user that no problems were found.
     */
    void DisplayNoProblemsMessage();

    /**
     * Configures the table for presenting validation results.
     */
    void ConfigureTableForResults();

    /**
     * Removes the epub filename prefix in the path, if any.
     *
     * @param path The relative path to clean.
     * @return The cleaned path.
     */
    static QString RemoveEpubPathPrefix( const QString &path );

    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    /**
     * The table that holds all the validation results.
     */
    QTableWidget &m_ResultTable;

    /**
     * The book being validated.
     */
    QSharedPointer< Book > m_Book;
};

#endif // VALIDATIONRESULTSVIEW_H


