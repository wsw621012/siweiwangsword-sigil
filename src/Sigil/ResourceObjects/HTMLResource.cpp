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
#include "HTMLResource.h"
#include "../Misc/Utility.h"
#include "../BookManipulation/CleanSource.h"
#include "../BookManipulation/XHTMLDoc.h"

static const QString LOADED_CONTENT_MIMETYPE = "application/xhtml+xml";


HTMLResource::HTMLResource( const QString &fullfilepath, 
                            QHash< QString, Resource* > *hash_owner,
                            int reading_order,
                            QObject *parent )
    : 
    Resource( fullfilepath, hash_owner, parent ),
    m_WebPage( NULL ),
    m_TextDocument( NULL ),
    m_WebPageIsOld( true ),
    m_TextDocumentIsOld( true ),
    m_ReadingOrder( reading_order )
{

}


Resource::ResourceType HTMLResource::Type() const
{
    return Resource::HTMLResource;
}


QWebPage& HTMLResource::GetWebPage()
{
    Q_ASSERT( m_WebPage );

    return *m_WebPage;
}


QTextDocument& HTMLResource::GetTextDocument()
{
    Q_ASSERT( m_TextDocument );

    return *m_TextDocument;
}


void HTMLResource::SetDomDocument( const QDomDocument &document )
{
    QWriteLocker locker( &m_ReadWriteLock );

    m_DomDocument = document;

    m_WebPageIsOld      = true;
    m_TextDocumentIsOld = true;
}


// Make sure to get a read lock externally before calling this function!
const QDomDocument& HTMLResource::GetDomDocumentForReading()
{
    return m_DomDocument;
}


// Make sure to get a write lock externally before calling this function!
// Call MarkSecondaryCachesAsOld() if you updated the document.
QDomDocument& HTMLResource::GetDomDocumentForWriting()
{
    // We can't just mark the caches as old right here since
    // some consumers need write access but don't end up writing anything.

    return m_DomDocument;
}


void HTMLResource::MarkSecondaryCachesAsOld()
{
    m_WebPageIsOld      = true;
    m_TextDocumentIsOld = true;
}


// only ever call this from the GUI thread
void HTMLResource::UpdateDomDocumentFromWebPage()
{
    Q_ASSERT( QThread::currentThread() == QApplication::instance()->thread() );

    m_DomDocument.setContent( GetWebPageHTML() );
}


void HTMLResource::UpdateDomDocumentFromTextDocument()
{
    Q_ASSERT( QThread::currentThread() == QApplication::instance()->thread() );
    Q_ASSERT( m_TextDocument );

    m_DomDocument.setContent( CleanSource::Clean( m_TextDocument->toPlainText() ) );
}


// only ever call this from the GUI thread
void HTMLResource::UpdateWebPageFromDomDocument()
{
    Q_ASSERT( QThread::currentThread() == QApplication::instance()->thread() );

    if ( !m_WebPageIsOld && !m_CSSResourcesUpdated )

        return;

    if ( m_WebPage == NULL )

        m_WebPage = new QWebPage( this );

    SetWebPageHTML( XHTMLDoc::GetQDomNodeAsString( m_DomDocument ) );

    m_WebPageIsOld = false;
    m_CSSResourcesUpdated = false;
}


void HTMLResource::UpdateTextDocumentFromDomDocument()
{
    Q_ASSERT( QThread::currentThread() == QApplication::instance()->thread() );

    if ( !m_TextDocumentIsOld )

        return;

    if ( m_TextDocument == NULL )
    {
        m_TextDocument = new QTextDocument( this );
        m_TextDocument->setDocumentLayout( new QPlainTextDocumentLayout( m_TextDocument ) );
    }

    m_TextDocument->setPlainText( CleanSource::PrettyPrint( XHTMLDoc::GetQDomNodeAsString( m_DomDocument ) ) );

    m_TextDocumentIsOld = false;
}


void HTMLResource::UpdateWebPageFromTextDocument()
{
    Q_ASSERT( QThread::currentThread() == QApplication::instance()->thread() );
    Q_ASSERT( m_TextDocument );
    Q_ASSERT( m_WebPage );

    QString source = CleanSource::Clean( m_TextDocument->toPlainText() );

    if ( m_OldSourceCache != source || m_CSSResourcesUpdated )
    {
        SetWebPageHTML( source );
        m_OldSourceCache = source;
        m_CSSResourcesUpdated = false;
    }
}


void HTMLResource::UpdateTextDocumentFromWebPage()
{
    Q_ASSERT( QThread::currentThread() == QApplication::instance()->thread() );
    Q_ASSERT( m_TextDocument );
    Q_ASSERT( m_WebPage );

    QString source = GetWebPageHTML();

    if ( m_OldSourceCache != source )
    {
        m_TextDocument->setPlainText( source );
        m_OldSourceCache = source;
    }    
}


void HTMLResource::SaveToDisk()
{
    QWriteLocker locker( &m_ReadWriteLock );

    Utility::WriteUnicodeTextFile( CleanSource::PrettyPrint( XHTMLDoc::GetQDomNodeAsString( m_DomDocument ) ),
                                   m_FullFilePath );
}


int HTMLResource::GetReadingOrder()
{
    return m_ReadingOrder;
}


void HTMLResource::SetReadingOrder( int reading_order )
{
    m_ReadingOrder = reading_order;
}


// Removes every occurrence of "signing" classes
// with which webkit litters our source code 
void HTMLResource::RemoveWebkitClasses()
{
    Q_ASSERT( m_WebPage );

    QWebElementCollection collection = m_WebPage->mainFrame()->findAllElements( ".Apple-style-span" );

    foreach( QWebElement element, collection )
    {
        element.toggleClass( "Apple-style-span" );
    }

    collection = m_WebPage->mainFrame()->findAllElements( ".webkit-indent-blockquote" );

    foreach( QWebElement element, collection )
    {
        element.toggleClass( "webkit-indent-blockquote" );
    }
}


bool HTMLResource::LessThan( HTMLResource* res_1, HTMLResource* res_2 )
{
    return res_1->GetReadingOrder() < res_2->GetReadingOrder();
}


void HTMLResource::LinkedCSSResourceUpdated()
{
    m_CSSResourcesUpdated = true;
}


QString HTMLResource::GetWebPageHTML()
{
    Q_ASSERT( m_WebPage );

    RemoveWebkitClasses();

    return CleanSource::Clean( RemoveCacheParamsFromLinks( m_WebPage->mainFrame()->toHtml() ) );
}


void HTMLResource::SetWebPageHTML( const QString &source )
{
    Q_ASSERT( m_WebPage );

    m_WebPage->mainFrame()->setContent( AddCacheParamsToLinks( source ).toUtf8(), 
                                        LOADED_CONTENT_MIMETYPE, 
                                        GetBaseUrl() 
                                      );

    m_WebPage->setContentEditable( true );

    // TODO: we kill external links; a dialog should be used
    // that asks the user if he wants to open this external link in a browser
    m_WebPage->setLinkDelegationPolicy( QWebPage::DelegateAllLinks );

    QWebSettings &settings = *m_WebPage->settings();
    settings.setAttribute( QWebSettings::LocalContentCanAccessRemoteUrls, false );
    settings.setAttribute( QWebSettings::JavascriptCanAccessClipboard, true );
    settings.setAttribute( QWebSettings::ZoomTextOnly, true );
}


//   We have to use this absolutely ridiculous hack to get CSS
// resources in <link> elements to reload properly. If we don't
// append a random parameter to the end of the link, the QtWebkit
// cache will *never* reload the CSS file. Not even if you call
// the QWebPage::Reload action, not even if you call the 
// QWebPage::ReloadAndBypassCache action, not event if you DESTROY
// the QWebPage object and replace it with a new one.
//   You have to use QWebSettings::clearMemoryCaches(), and that
// kills the whole cache.
//   So we hack it... 
QString HTMLResource::AddCacheParamsToLinks( const QString &source )
{
    // We can't do this with the QWebElement API since we need to return
    // a clean version to the QTextDocument, and for that we have to clean
    // the QWebPage... and then put it all back... QRegExp is the only way.

    int head_end_index = source.indexOf( QRegExp( HEAD_END ) );
    QString head = Utility::Substring( 0, head_end_index, source );

    QRegExp link( "(<\\s*link[^>]*href\\s*=\\s*\")([^\"]*)(\"[^>]*>)" );
    link.setMinimal( true );

    QStringList linked_files;

    int main_index = 0;

    while ( true )
    {
        main_index = head.indexOf( link, main_index );

        if ( main_index == -1 )

            break;

        linked_files.append( link.cap( 2 ) );
        
        QString new_link = link.cap( 1 ) + link.cap( 2 ) + "?sgrnd=" + Utility::CreateUUID() + link.cap( 3 );
        head.replace( main_index, link.matchedLength(), new_link );

        main_index += new_link.size();
    }

    TrackNewCSSResources( linked_files );

    return head + Utility::Substring( head_end_index, source.length(), source );
}


void HTMLResource::TrackNewCSSResources( const QStringList &filepaths )
{    
    foreach( Resource *resource, m_LinkedCSSResources )
    {
        disconnect( resource, SIGNAL( ResourceUpdatedOnDisk() ), this, SLOT( LinkedCSSResourceUpdated() ) );
    }

    m_LinkedCSSResources.clear();

    QStringList filenames;

    foreach( QString filepath, filepaths )
    {
        filenames.append( QFileInfo( filepath ).fileName() );
    }

    foreach( Resource *resource, m_HashOwner.values() )
    {
        if ( resource->Type() == Resource::CSSResource )
        {
            if ( filenames.contains( resource->Filename() ) )
                
                m_LinkedCSSResources.append( resource );
        }
    }

    foreach( Resource *resource, m_LinkedCSSResources )
    {
        connect( resource, SIGNAL( ResourceUpdatedOnDisk() ), this, SLOT( LinkedCSSResourceUpdated() ) );
    }
}


QString HTMLResource::RemoveCacheParamsFromLinks( const QString &source )
{
    int head_end_index = source.indexOf( QRegExp( HEAD_END ) );
    QString head = Utility::Substring( 0, head_end_index, source );

    QRegExp link( "(<\\s*link[^>]*href\\s*=\\s*\"[^\"]*)\\?sgrnd=.+(\"[^>]*>)" );
    link.setMinimal( true );

    head.replace( link, "\\1\\2" );

    return head + Utility::Substring( head_end_index, source.length(), source );
}


