/************************************************************************
**
**  Copyright (C) 2009  Strahinja Markovic, Nokia Corporation
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
#include "CodeViewEditor.h"
#include "LineNumberArea.h"
#include "../BookManipulation/Book.h"
#include "../BookManipulation/XHTMLDoc.h"
#include "Misc/XHTMLHighlighter.h"
#include <QDomDocument>

static const int COLOR_FADE_AMOUNT = 175;
static const int TAB_SPACES_WIDTH  = 4;
static const int BASE_FONT_SIZE    = 10;
                  
static const QString XML_OPENING_TAG = "(<[^>/][^>]*[^>/]>|<[^>/]>)";


// Constructor;
// the parameters is the object's parent
CodeViewEditor::CodeViewEditor( QWidget *parent )
    : 
    QPlainTextEdit( parent ),
    m_LineNumberArea( new LineNumberArea( this ) ),
    m_Highlighter( new XHTMLHighlighter( document() ) ),
    m_CurrentZoomFactor( 1.0 )
{
    connect( this, SIGNAL( blockCountChanged( int ) ),             this, SLOT( UpdateLineNumberAreaMargin() ) );
    connect( this, SIGNAL( updateRequest( const QRect &, int) ),   this, SLOT( UpdateLineNumberArea( const QRect &, int) ) );
    connect( this, SIGNAL( cursorPositionChanged() ),              this, SLOT( HighlightCurrentLine()       ) );

    UpdateLineNumberAreaMargin();
    HighlightCurrentLine();

    // Let's try to use Consolas as our font
    QFont font( "Consolas", BASE_FONT_SIZE );

    // But just in case, say we want a fixed width font
    // if Consolas is not on the system
    font.setStyleHint( QFont::TypeWriter );
    setFont( font );
    setTabStopWidth( TAB_SPACES_WIDTH * QFontMetrics( font ).width( ' ' ) );
}

// Sets the content of the View to the specified book
void CodeViewEditor::SetBook( const Book &book )
{
    setPlainText( book.source );
}


// Paints the line number area;
// receives the event directly 
// from the area's paintEvent() handler
void CodeViewEditor::LineNumberAreaPaintEvent( QPaintEvent *event )
{
    QPainter painter( m_LineNumberArea );

    // Paint the background first
    painter.fillRect( event->rect(), Qt::lightGray );

    // A "block" represents a line of text
    QTextBlock block = firstVisibleBlock();

    // Blocks are numbered from zero,
    // but we count lines of text from one
    int blockNumber  = block.blockNumber() + 1;

    // Getting the Y coordinates for
    // the top and bottom of a block
    int topY    = (int) blockBoundingGeometry( block ).translated( contentOffset() ).top();
    int bottomY = topY + (int) blockBoundingRect( block ).height();

    // We loop through all the visible and
    // unobscured blocks and paint line numbers for each
    while ( block.isValid() && ( topY <= event->rect().bottom() ) )
    {
        if ( block.isVisible() && ( bottomY >= event->rect().top() ) )
        {
            QString number_to_paint = QString::number( blockNumber );

            // Draw the line number
            painter.setPen( Qt::black );
            painter.drawText(   0,
                                topY,
                                m_LineNumberArea->width(),
                                fontMetrics().height(),
                                Qt::AlignRight,
                                number_to_paint
                            );
        }

        block    = block.next();
        topY     = bottomY;
        bottomY  = topY + (int) blockBoundingRect( block ).height();

        blockNumber++;
    }
}

// Returns the width the LinuNumberArea
// should take (in pixels)
int CodeViewEditor::CalculateLineNumberAreaWidth() const
{
    int num_digits       = 1;
    int last_line_number = blockCount();

    // We count the number of digits
    // for the line number of the last line
    while ( last_line_number >= 10 )
    {
        last_line_number /= 10;
        num_digits++;
    }

    int needed_width = 3 + fontMetrics().width( QChar( '0' ) ) * num_digits;

    return needed_width;
}


// Returns a list of elements representing a "chain"
// or "walk" through the XHTML document with which one
// can identify a single element in the document.
// This list identifies the element in which the 
// keyboard caret is currently located.
QList< ViewEditor::ElementIndex > CodeViewEditor::GetCaretLocation()
{
    QRegExp tag( XML_OPENING_TAG );

    // We search for the first opening tag behind the caret.
    // This specifies the element the caret is located in.
    int offset = toPlainText().lastIndexOf( tag, textCursor().position() );

    return ConvertStackToHierarchy( GetCaretLocationStack( offset + tag.matchedLength() ) );
}


// Accepts a list returned by a view's GetCaretLocation
// and creates and stores an update that sends the caret
// in this view to the specified element.
// The CodeView implementation initiates the update in
// the main event handler.
void CodeViewEditor::StoreCaretLocationUpdate( const QList< ViewEditor::ElementIndex > &hierarchy )
{
    m_CaretUpdate = hierarchy;
}


// Sets a zoom factor for the view,
// thus zooming in (factor > 1.0) or out (factor < 1.0). 
void CodeViewEditor::SetZoomFactor( float factor )
{
    m_CurrentZoomFactor = factor;

    QFont current_font = font();
    current_font.setPointSizeF( BASE_FONT_SIZE * m_CurrentZoomFactor );
    setFont( current_font );
    
    // We update size of the line number area
    m_LineNumberArea->MyUpdateGeometry();
    UpdateLineNumberAreaMargin();

    emit ZoomFactorChanged( factor );
}


// Returns the View's current zoom factor
float CodeViewEditor::GetZoomFactor() const
{
    return m_CurrentZoomFactor;
}


// Finds the next occurrence of the search term in the document,
// and selects the matched string. The first argument is the matching
// regex, the second is the direction of the search.
bool CodeViewEditor::FindNext( const QRegExp &search_regex, Searchable::Direction search_direction )
{
    int selection_offset = GetSelectionOffset( search_direction );

    QRegExp result_regex = search_regex;
    RunSearchRegex( result_regex, toPlainText(), selection_offset, search_direction ); 

    if ( result_regex.pos() != -1 )
    {
        QTextCursor cursor = textCursor();

        cursor.setPosition( result_regex.pos() );
        cursor.setPosition( result_regex.pos() + result_regex.matchedLength(), QTextCursor::KeepAnchor );

        setTextCursor( cursor );

        return true;
    } 

    return false;
}


// Returns the number of times that the specified
// regex matches in the document.
int CodeViewEditor::Count( const QRegExp &search_regex )
{
    return toPlainText().count( search_regex );
}


// If the currently selected text matches the specified regex, 
// it is replaced by the specified replacement string.
bool CodeViewEditor::ReplaceSelected( const QRegExp &search_regex, const QString &replacement )
{
    QRegExp result_regex  = search_regex;
    QTextCursor cursor    = textCursor();
    QString selected_text = cursor.selectedText();

    // If we are currently sitting at the start 
    // of a matching substring, we replace it.
    if ( result_regex.exactMatch( selected_text ) )
    {
        QString final_replacement = FillWithCapturedTexts( result_regex.capturedTexts(), replacement );
        cursor.insertText( final_replacement );

        return true;
    }

    return false;
}

// Replaces all occurrences of the specified regex in 
// the document with the specified replacement string.
int CodeViewEditor::ReplaceAll( const QRegExp &search_regex, const QString &replacement )
{
    QRegExp result_regex  = search_regex;
    QTextCursor cursor    = textCursor();

    int index = 0;
    int count = 0;

    QProgressDialog progress( tr( "Replacing search term..." ), QString(), 0, Count( search_regex ) );
    progress.setMinimumDuration( PROGRESS_BAR_MINIMUM_DURATION );

    while ( toPlainText().indexOf( result_regex, index ) != -1 )
    {
        // Update the progress bar
        progress.setValue( count );

        cursor.setPosition( result_regex.pos() );
        cursor.setPosition( result_regex.pos() + result_regex.matchedLength(), QTextCursor::KeepAnchor );

        QString final_replacement = FillWithCapturedTexts( result_regex.capturedTexts(), replacement );
        cursor.insertText( final_replacement );

        index = result_regex.pos() + final_replacement.length();
        ++count;
    }

    return count;
}


// The base class implementation of the print()
// method is not a slot, and we need it as a slot
// for print preview support; so this is just
// a slot wrapper around that function 
void CodeViewEditor::print( QPrinter* printer )
{
    QPlainTextEdit::print( printer );
}

// Overridden because we need to update the cursor
// location if a cursor update (from BookView) 
// is waiting to be processed
bool CodeViewEditor::event( QEvent *event )
{
    // We just return whatever the "real" event handler returns
    bool real_return = QPlainTextEdit::event( event );
    
    // Executing the caret update inside the paint event
    // handler causes artifacts on mac. So we do it after
    // the event is processed and accepted.
    if ( event->type() == QEvent::Paint )
    {
        ExecuteCaretUpdate();
    }
    
    return real_return;
}


// Overridden because after updating itself on a resize event,
// the editor needs to update its line number area too
void CodeViewEditor::resizeEvent( QResizeEvent *event )
{
    // Update self normally
    QPlainTextEdit::resizeEvent( event );

    QRect contents_area = contentsRect();

    // Now update the line number area
    m_LineNumberArea->setGeometry( QRect(   contents_area.left(), 
                                            contents_area.top(), 
                                            CalculateLineNumberAreaWidth(), 
                                            contents_area.height() 
                                        ) 
                                 );
}


// Overridden because we want the ExecuteCaretUpdate()
// to be called from here when the user clicks inside
// this widget in SplitView. Leaving it up to our event()
// override causes graphical artifacts for SplitView.
// So in those conditions, this handler takes over.
void CodeViewEditor::mousePressEvent( QMouseEvent *event )
{
    // Propagate to base class
    QPlainTextEdit::mousePressEvent( event );   

    // Run the caret update if it's pending
    ExecuteCaretUpdate();
}


// Called whenever the number of lines changes;
// sets a margin where the line number area can be displayed
void CodeViewEditor::UpdateLineNumberAreaMargin()
{ 
    // The left margin width depends on width of the line number area
    setViewportMargins( CalculateLineNumberAreaWidth(), 0, 0, 0 );
}

// The first parameter represents the area 
// that the editor needs an update of, and the second
// is the amount of pixels the viewport is vertically scrolled
void CodeViewEditor::UpdateLineNumberArea( const QRect &area_to_update, int vertically_scrolled )
{
    // If the editor scrolled, scroll the line numbers too
    if ( vertically_scrolled != 0 )

        m_LineNumberArea->scroll( 0, vertically_scrolled );

    else // otherwise update the required portion
    
        m_LineNumberArea->update( 0, area_to_update.y(), m_LineNumberArea->width(), area_to_update.height() );

    if ( area_to_update.contains( viewport()->rect() ) )

        UpdateLineNumberAreaMargin();
}


// Highlights the line the user is editing
void CodeViewEditor::HighlightCurrentLine()
{
    QList< QTextEdit::ExtraSelection > extraSelections;

    QTextEdit::ExtraSelection selection;

    QColor lineColor = QColor( Qt::yellow ).lighter( COLOR_FADE_AMOUNT );

    selection.format.setBackground( lineColor );
    
    // Specifies that we want the whole line to be selected
    selection.format.setProperty( QTextFormat::FullWidthSelection, true );

    // We reset the selection of the cursor
    // so that only one line is highlighted
    selection.cursor = textCursor();
    selection.cursor.clearSelection();

    extraSelections.append( selection );    
    setExtraSelections( extraSelections );
}


// Returns a stack of elements representing the
// current location of the caret in the document.
// Accepts the number of characters to the end of
// the start tag of the element the caret is residing in. 
QStack< CodeViewEditor::StackElement > CodeViewEditor::GetCaretLocationStack( int offset ) const
{
    QString source = toPlainText();
    QXmlStreamReader reader( source );

    QStack< StackElement > stack; 

    while ( !reader.atEnd() ) 
    {
        // Get the next token from the stream
        QXmlStreamReader::TokenType type = reader.readNext();

        if ( type == QXmlStreamReader::StartElement ) 
        {
            // If we detected the start of a new element, then
            // the element currently on the top of the stack
            // has one more child element
            if ( !stack.isEmpty() )

                stack.top().num_children++;
            
            StackElement new_element;
            new_element.name         = reader.name().toString();
            new_element.num_children = 0;

            stack.push( new_element );

            // Check if this is the element start tag
            // we are looking for
            if ( reader.characterOffset() == offset  )

                break;
        }

        // If we detect the end tag of an element,
        // we remove it from the top of the stack
        else if ( type == QXmlStreamReader::EndElement )
        {
            stack.pop();
        }
    }

    if ( reader.hasError() )
    {
        // Just return an empty location.
        // Maybe we could return the stack we currently have?
        return QStack< StackElement >();
    }

    return stack;
}


// Converts the stack provided by GetCaretLocationStack()
// and converts it into the element location hierarchy
QList< ViewEditor::ElementIndex > CodeViewEditor::ConvertStackToHierarchy( const QStack< StackElement > stack ) const
{
    QList< ViewEditor::ElementIndex > hierarchy;

    foreach( StackElement stack_element, stack )
    {
        ViewEditor::ElementIndex new_element;

        new_element.name  = stack_element.name;
        new_element.index = stack_element.num_children - 1;

        hierarchy.append( new_element );
    }

    return hierarchy;
}


// Converts a ViewEditor element hierarchy to a CaretMove
CodeViewEditor::CaretMove CodeViewEditor::ConvertHierarchyToCaretMove( const QList< ViewEditor::ElementIndex > &hierarchy ) const
{
    QDomDocument dom;
    dom.setContent( toPlainText() );

    QDomNode end_node = XHTMLDoc::GetNodeFromHierarchy( dom, hierarchy );

    QTextCursor cursor( document() );
    CodeViewEditor::CaretMove caret_move;

    if ( !end_node.isNull() ) 
    {
        caret_move.vertical_lines   = end_node.lineNumber() - cursor.blockNumber();
        caret_move.horizontal_chars = end_node.columnNumber();   
    }

    else
    {   
        caret_move.vertical_lines   = 0;
        caret_move.horizontal_chars = 0;
    }

    return caret_move;
}


// Executes the caret updating code
// if an update is pending;
// returns true if update was performed
bool CodeViewEditor::ExecuteCaretUpdate()
{
    // If there's a cursor/caret update waiting (from BookView),
    // we update the caret location and reset the update variable
    if ( m_CaretUpdate.count() == 0 )
    {
        return false;
    }

    QTextCursor cursor( document() );

    // We *have* to do the conversion on-demand since the 
    // conversion uses toPlainText(), and the text needs to up-to-date.
    CodeViewEditor::CaretMove caret_move = ConvertHierarchyToCaretMove( m_CaretUpdate );

    cursor.movePosition( QTextCursor::NextBlock, QTextCursor::MoveAnchor, caret_move.vertical_lines );
    cursor.movePosition( QTextCursor::Left     , QTextCursor::MoveAnchor, caret_move.horizontal_chars );

    m_CaretUpdate.clear();

    setTextCursor( cursor );

    // Center the screen on the cursor/caret location.
    // Centering requires fresh information about the
    // visible viewport, so we usually call this after
    // the paint event has been processed.
    centerCursor();

    return true;
}


// Returns the selection offset from the start of the 
// document depending on the search direction specified
int CodeViewEditor::GetSelectionOffset( Searchable::Direction search_direction ) const
{
    if (    search_direction == Searchable::Direction_Down ||
            search_direction == Searchable::Direction_All
       )
    {
        return textCursor().selectionEnd();
    }

    else
    {
        return textCursor().selectionStart();
    }
}




