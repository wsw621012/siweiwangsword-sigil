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

#pragma once
#ifndef FLOWTAB_H
#define FLOWTAB_H

#include "ContentTab.h"

class QSplitter;
class BookViewEditor;
class CodeViewEditor;
class ViewEditor;
class HTMLResource;
class QUrl;

class FlowTab : public ContentTab
{
    Q_OBJECT

public:

    FlowTab( Resource& resource, QWidget *parent = 0 );

    // Overrides inherited from ContentTab
    bool IsModified();

    bool CutEnabled();

    bool CopyEnabled();

    bool PasteEnabled();

    bool BoldChecked();

    bool ItalicChecked();

    bool UnderlineChecked();

    bool StrikethroughChecked();

    bool BulletListChecked();

    bool NumberListChecked();

    bool BookViewChecked();

    bool SplitViewChecked();

    bool CodeViewChecked();

    QString GetCaretElementName();

    float GetZoomFactor();

    void SetZoomFactor( float new_zoom_factor );

public slots:

    // Implements Undo action functionality
    void Undo();

    // Implements Redo action functionality
    void Redo();

    // Implements Cut action functionality
    void Cut();

    // Implements Copy action functionality
    void Copy();

    // Implements Paste action functionality
    void Paste();

    // Implements Bold action functionality
    void Bold();

    // Implements Italic action functionality
    void Italic();

    // Implements Underline action functionality
    void Underline();

    // Implements Strikethrough action functionality
    void Strikethrough();

    // Implements Align Left action functionality
    void AlignLeft();

    // Implements Center action functionality
    void Center();

    // Implements Align Right action functionality
    void AlignRight();

    // Implements Justify action functionality
    void Justify();

    // Implements Insert chapter break action functionality
    void InsertChapterBreak();

    // Implements Insert image action functionality
    void InsertImage();

    // Implements Insert bulleted list action functionality
    void InsertBulletedList();

    // Implements Insert numbered list action functionality
    void InsertNumberedList();

    // Implements Decrease indent action functionality
    void DecreaseIndent();

    // Implements Increase indent action functionality
    void IncreaseIndent();

    // Implements Remove Formatting action functionality
    void RemoveFormatting();

    // Implements the heading combo box functionality
    void HeadingStyle( const QString& heading_type );

    // Implements Print Preview action functionality
    void PrintPreview();

    // Implements Print action functionality
    void Print();

    void BookView();

    void SplitView();

    void CodeView();

signals:

    void EnteringBookView();

    void EnteringCodeView();

    void ViewChanged();

    void ContentChanged();

    void ZoomFactorChanged( float factor );

    void SelectionChanged();

    void LinkClicked( const QUrl &url );

private slots:

    // Used to catch the focus changeover from one widget
    // (code or book view) to the other; needed for source synchronization
    void FocusFilter( QWidget *old_widget, QWidget *new_widget );

    void EmitContentChanged();

    // Updates the m_Source variable whenever
    // the user edits in book view
    void UpdateSourceFromBookView();

    // Updates the m_Source variable whenever
    // the user edits in code view
    void UpdateSourceFromCodeView();

    // On changeover, updates the code in code view
    void UpdateCodeViewFromSource();

    // On changeover, updates the code in book view
    void UpdateBookViewFromSource();    

private:

    void ReadSettings();
    
    void WriteSettings();

    ViewEditor& GetActiveViewEditor() const;

    // Runs HTML Tidy on sSource variable
    void TidyUp();

    // Removes every occurrence of "signing" classes
    // with which webkit litters our source code 
    void RemoveWebkitClasses();

    void ConnectSignalsToSlots();


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    HTMLResource &m_HTMLResource;

    QSplitter &m_Splitter;

    // The webview component that renders out HTML
    BookViewEditor &m_wBookView;

    // The plain text code editor 
    CodeViewEditor &m_wCodeView;

    // true if the last view the user edited in was book view
    bool m_isLastViewBook; 

    // We should be able to return a ref to this string to callers,
    // and keep track whether the source has been tidied... if it
    // hasn't, then tidy it first and then return it
    QString m_Source;

    // The value of the m_Book.sSource variable
    // after the last view change
    QString m_OldSource;

    // The last folder to which the user imported an image;
    // Static is safe since only the GUI thread can
    // open/close dialogs and thus change this value.
    static QString s_LastFolderImage;
};

#endif // FLOWTAB_H


