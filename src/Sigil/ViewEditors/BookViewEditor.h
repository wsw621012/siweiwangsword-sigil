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
#ifndef BOOKVIEWEDITOR_H
#define BOOKVIEWEDITOR_H

#include <QWebView>
#include "ViewEditor.h"
#include <QDomNode>
#include <QMap>
#include <QWebElement>

class QShortcut;

/**
 * A WYSIWYG editor for XHTML flows. 
 * Also called the "Book View", because it shows a
 * chapter of a book in its final, rendered state
 * (the way it will look like in epub Reading Systems).
 */
class BookViewEditor : public QWebView, public ViewEditor
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent The object's parent.
     */
    BookViewEditor( QWidget *parent = 0 );

    /**
     * Sets a custom webpage for the editor.
     */
    void CustomSetWebPage( QWebPage &webpage );

    /**
     * Splits the chapter and returns the "upper" content.
     * The current flow is split at the caret point.
     *
     * @return The content of the chapter up to the chapter break point.
     * 
     * @note What we actually do when the user wants to split the loaded chapter
     * is create a new tab with the XHTML content \em above the split point.
     * The new tab is actually the "old" chapter, and this tab becomes the
     * "new" chapter.
     * \par 
     * Why? Because we can only avoid a tab render in the tab from which
     * we remove elements. Since the users move from the top of a large HTML
     * file down, the new chapter will be the one with the most content.
     * So this way we \em try to avoid the painful render time on the biggest
     * chapter, but there is still some render time left...
     */
    QString SplitChapter();

    /**
     * Executes a contentEditable command.
     * The command is executed through JavaScript.
     *
     * @param command The command to execute.
     */
    void ExecCommand( const QString &command );

    /**
     * Executes a contentEditable command.
     * The command is executed through JavaScript.
     *
     * @param command The command to execute.
     * @param parameter The parameter that should be passed to the command.
     */
    void ExecCommand( const QString &command, const QString &parameter );

    /**
     * Returns the state of the contentEditable command.
     * The query is performed through JavaScript.
     */
    bool QueryCommandState( const QString &command );

    /**
     *  Workaround for a crappy setFocus implementation in QtWebKit.
     */
    void GrabFocus();

    /**
     * Scrolls the editor to the top.
     */
    void ScrollToTop();

    /**
     * Scrolls the editor to the specified fragment.
     *
     * @param fragment The fragment ID to scroll to. 
     *                 It should have the "#" character as the first character. 
     */
    void ScrollToFragment( const QString &fragment );

    /**
     * Scrolls the editor to the specified fragment after the document is loaded.
     *
     * @param fragment The fragment ID to scroll to. 
     *                 It should have the "#" character as the first character. 
     */
    void ScrollToFragmentAfterLoad( const QString &fragment );

    /**
     * Implements the "formatBlock" execCommand because
     * WebKit's default one has bugs.
     * It takes an element name as an argument (e.g. "p"),
     * and replaces the element the cursor is located in with it.
     *
     * @param element_name The name of the element to format the block to.
     */
    void FormatBlock( const QString &element_name );

    /**
     * Returns the name of the element the caret is located in.
     * If text is selected, returns the name of the element
     * where the selection \em starts.
     *
     * @return The name of the caret element.
     */
    QString GetCaretElementName();

    // inherited
    QList< ViewEditor::ElementIndex > GetCaretLocation(); 

    /**
     * @copydoc ViewEditor::StoreCaretLocationUpdate
     *
     * The BookView implementation initiates the update in
     * the JavascriptOnDocumentLoad() function.
     * This should \em always be called \em before the page is updated
     * to avoid loading race conditions.
     */
    void StoreCaretLocationUpdate( const QList< ViewEditor::ElementIndex > &hierarchy );

    // inherited
    bool IsLoadingFinished();

    void SetZoomFactor( float factor );

    float GetZoomFactor() const;

    bool FindNext( const QRegExp &search_regex, 
                   Searchable::Direction search_direction,
                   bool ignore_selection_offset = false );

    int Count( const QRegExp &search_regex );

    bool ReplaceSelected( const QRegExp &search_regex, const QString &replacement );

    int ReplaceAll( const QRegExp &search_regex, const QString &replacement );
    
signals:

    /**
     * Emitted when the text changes.
     * The contentsChanged QWebPage signal is wired to this one,
     * and contentsChangedExtra is wired to contentsChanged.
     */
    void textChanged();

    /**
     * Extends the QWebPage contentsChanged signal.
     * Use textChanged to know when the BookView has been modified.
     *
     * The QWebPage contentsChanged signal is not emitted on every
     * occasion we want it to, so we emit this when necessary.
     * This signal is in turn wired to contentsChanged. Why?
     * Because we want others connected to our QWebPage but not to 
     * the Book View textChanged signal to be aware of these changes.
     * Thus, the wired extension.
     */
    void contentsChangedExtra();

    /**
     * Emitted whenever the zoom factor changes.
     *
     * @param new_zoom_factor The new zoom factor of the View.
     */
    void ZoomFactorChanged( float new_zoom_factor );

    /**
     * A filtered version of the QWebPage::linkClicked signal.
     * Should be used in place of that one, since this one 
     * performs custom logic on the QUrl.
     *
     * @param url The URL of the clicked link.
     */
    void FilteredLinkClicked( const QUrl& url );

protected:

    /**
     * The main event handler.
     *
     * @param event The event to process.
     */
    bool event( QEvent *event );

private slots:

    /**
     * Executes javascript that needs to be run when 
     * the document has finished loading.
     */ 
    void JavascriptOnDocumentLoad();
    
    /**
     * Tracks the loading progress.
     * Updates the state of the m_isLoadFinished variable 
     * depending on the received loading progress; if the 
     * progress equals 100, the state is true, otherwise false.
     * 
     * @param progress The value of the loading progress (0-100).
     */
    void UpdateFinishedState( int progress );

    /**
     * Filters the linkCLicked signal.
     *
     * @param url The URL of the clicked link.
     */
    void LinkClickedFilter( const QUrl& url );

    /**
     * Wrapper slot for the Page Up shortcut.
     */
    void PageUp();

    /**
     * Wrapper slot for the Page Down shortcut.
     */
    void PageDown();
    
    /**
     * Wrapper slot for the Scroll One Line Up shortcut.
     */
    void ScrollOneLineUp();

    /**
     * Wrapper slot for the Scroll One Line Down shortcut.
     */
    void ScrollOneLineDown();

private:

    /**
     * Evaluates the provided javascript source code  
     * and returns the result.
     *
     * @param javascript The JavaScript source code to execute.
     * @return The result from the last executed javascript statement.
     */
    QVariant EvaluateJavascript( const QString &javascript );

    /**
     * Returns the local character offset of the selection.
     * The offset is calculated in the local text node.
     *
     * @param start_of_selection If \c true, then the offset is calculated from
     *                           the start of the selection. Otherwise, from its end.
     * @return The offset.
     */
    int GetLocalSelectionOffset( bool start_of_selection );

    /**
     * Returns the selection offset from the start of the document.
     *
     * @param document The loaded DOM document.
     * @param node_offsets The text node offset map from SearchTools.
     * @param search_direction The direction of the search.
     * @return The offset.
     */
    int GetSelectionOffset( const QDomDocument &document,
                            const QMap< int, QDomNode > &node_offsets, 
                            Searchable::Direction search_direction );

    /**
     * Returns the currently selected text string.
     * 
     * @return The currently selected search string.
     */
    QString GetSelectedText();

    /**
     * The necessary tools for searching.
     */
    struct SearchTools
    {
        /**
         * The full text of the document.
         */
        QString fulltext;
    
        /**
         * A map with text node starting offsets as keys,
         * and those text nodes as values.
         */
        QMap< int, QDomNode > node_offsets;

        /**
         *  A DOM document with the loaded text.
         */
        QDomDocument document;
    };

    /**
     * Returns the all the necessary tools for searching.
     * Reads from the QWebPage source.
     *
     * @return The necessary tools for searching.
     */
    SearchTools GetSearchTools() const;    

    /**
     * Builds the element-selecting JavaScript code, ignoring the text nodes.
     * Always just chains children() jQuery calls.
     *
     * @return The element-selecting JavaScript code.
     */
    QString GetElementSelectingJS_NoTextNodes( const QList< ViewEditor::ElementIndex > &hierarchy ) const;

    /**
     * Builds the element-selecting JavaScript code, ignoring all the
     * text nodes except the last one.
     * Chains children() jQuery calls, and then the contents() function
     * for the last element (the text node, naturally).
     *
     * @return The element-selecting JavaScript code.
     */
    QString GetElementSelectingJS_WithTextNode( const QList< ViewEditor::ElementIndex > &hierarchy ) const;

    /**
     * Converts a QDomNode from a QDom of the current page
     * into the QWebElement of that same element on tha page.
     *
     * @param node The node to covert.
     */
    QWebElement QDomNodeToQWebElement( const QDomNode &node );

    /**
     * Escapes JavaScript string special characters.
     *
     * @return The escaped string.
     */
    QString EscapeJSString( const QString &string );

    /**
     * Executes the caret updating code if an update is pending.
     *
     * @return \c true if the update was performed.
     */
    bool ExecuteCaretUpdate();

    /**
     * The inputs for a new JavaScript \c range object.
     */
    struct SelectRangeInputs
    {
        /**
         * The range start node.
         */
        QDomNode start_node;

        /**
         *  The range end node.
         */
        QDomNode end_node;

        /**
         * The char index inside the start node.
         */
        int start_node_index;

        /**
         * The char index inside the end node.
         */
        int end_node_index;
    };

    /**
     * Converts the parameters into JavaScript \c range object inputs.
     * The \c range object can then be used to select this particular string.
     *
     * @param The node offset map
     * @param string_start The index of the string in the full document text
     * @param string_length The string's length
     * @return The inputs for a \c range object.
     * @see SearchTools
     */
    SelectRangeInputs GetRangeInputs( const QMap< int, QDomNode > &node_offsets, 
                                      int string_start, 
                                      int string_length ) const;

    /**
     * Converts the \c range input struct into the \c range creating JavaScript code.
     * 
     * @param input The \c range object inputs.
     * @return The \c range creating JavaScript code.
     */
    QString GetRangeJS( const SelectRangeInputs &input ) const;

    /**
     * Selects the string identified by the \c range inputs.
     *
     * @param input The \c range inputs.
     */
    void SelectTextRange( const SelectRangeInputs &input );

    /**
     * Scrolls the view to the specified node and text offset
     * within that node. 
     *
     * @param node The node to scroll to.
     * @param character_offset The specific offset we're interested
     *                         in within the node.
     */
    void ScrollToNodeText( const QDomNode &node, int character_offset );

    /**
     * Scrolls the whole screen by one line. 
     * Used for ScrollOneLineUp and ScrollOneLineDown shortcuts.
     * 
     * @param down Specifies are we scrolling up or down.
     */
    void ScrollByLine( bool down );

    /**
     * Scrolls the whole screen a number of pixels.
     *
     * @param pixel_number The number of pixels to scroll
     * @param down Specifies are we scrolling up or down.
     */
    void ScrollByNumPixels( int pixel_number, bool down );


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    /**
     * Stores the JavaScript source code for the 
     * caret location update. Used when switching from 
     * CodeViewEditor to BookViewEditor.
     */
    QString m_CaretLocationUpdate;

    /**
      * Set to \c false whenever the page is loading content.
     */
    bool m_isLoadFinished;

    /**
     * PageUp keyboard shortcut.
     */
    QShortcut &m_PageUp;

    /**
     * PageDown keyboard shortcut.
     */
    QShortcut &m_PageDown; 

    /**
     * Keyboard shortcut for scrolling one line up.
     */
    QShortcut &m_ScrollOneLineUp;

    /**
     * Keyboard shortcut for scrolling one line down.
     */
    QShortcut &m_ScrollOneLineDown;

    /** 
     * The JavaScript source code used 
     * to get a hierarchy of elements from
     * the caret element to the top of the document.
     */
    const QString c_GetCaretLocation;

    /**
     * The JavaScript source code that
     * removes all of the current selections
     * and adds the range in the "range"
     * variable to the current selection.
     */
    const QString c_NewSelection;

    /**
     * The JavaScript source code
     * for creating DOM ranges.
     */
    const QString c_GetRange;

    /**
     * The JavaScript source code that deletes the
     * contents of the range in "range" and replaces
     * them with a new text node whose text should be inputted.
     */
    const QString c_ReplaceText;

    /**
     * The JavaScript source code that returns the XHTML source
     * from the caret to the top of the file. This code is also
     * removed from the current chapter.
     */
    const QString c_GetSegmentHTML;
};


#endif // BOOKVIEWEDITOR_H

