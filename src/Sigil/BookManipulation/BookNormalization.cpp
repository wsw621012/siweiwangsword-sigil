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
#include "Misc/Utility.h"
#include "BookManipulation/Book.h"
#include "BookNormalization.h"
#include "BookManipulation/CleanSource.h"
#include "BookManipulation/Headings.h"
#include "BookManipulation/XhtmlDoc.h"
#include "BookManipulation/GuideSemantics.h"
#include "BookManipulation/XercesCppUse.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/OPFResource.h"
#include "BookManipulation/FolderKeeper.h"

static const QString SIGIL_HEADING_ID_PREFIX = "heading_id_";
static const QString SIGIL_HEADING_ID_REG    = SIGIL_HEADING_ID_PREFIX + "(\\d+)";
static const int FLOW_SIZE_THRESHOLD         = 1000;


void BookNormalization::Normalize( QSharedPointer< Book > book )
{
    QList< HTMLResource* > html_resources = book->GetFolderKeeper().GetResourceTypeList< HTMLResource >( true );

    GiveIDsToHeadings( html_resources );

    if ( !CoverPageExists( *book ) )
    
        TryToSetCoverPage( html_resources, *book );

    QList< ImageResource* > image_resources = book->GetFolderKeeper().GetResourceTypeList< ImageResource >();

    if ( !book->GetOPF().CoverImageExists() )

        TryToSetCoverImage( html_resources, image_resources, *book );
}


void BookNormalization::GiveIDsToHeadings( const QList< HTMLResource* > &html_resources )
{
    QtConcurrent::blockingMap( html_resources, GiveIDsToHeadingsInResource );
}


void BookNormalization::GiveIDsToHeadingsInResource( HTMLResource *html_resource )
{
    QWriteLocker locker( &html_resource->GetLock() );

    QList< Headings::Heading > headings = Headings::GetHeadingListForOneFile( html_resource );

    int heading_id_index = MaxSigilHeadingIDIndex( headings ) + 1;

    for ( int index = 0; index < headings.count(); index++ )
    {
        xc::DOMElement &element = *headings.at( index ).element;
        
        if ( !element.hasAttribute( QtoX( "id" ) ) )
        {
            element.setAttribute( 
                QtoX( "id" ), 
                QtoX( SIGIL_HEADING_ID_PREFIX + QString::number( heading_id_index ) ) );
            
            heading_id_index++;
        }
    }    
}


int BookNormalization::MaxSigilHeadingIDIndex( const QList< Headings::Heading > headings )
{
    int maxindex = 1;
    
    for ( int index = 0; index < headings.count(); index++ )
    {
        xc::DOMElement &element = *headings.at( index ).element;

        QRegExp suffix( SIGIL_HEADING_ID_REG );

        if ( XtoQ( element.getAttribute( QtoX( "id" ) ) ).contains( suffix ) )
        {
            int index = suffix.cap( 1 ).toInt();

            if ( index > maxindex )

                maxindex = index;
        }
    }

    return maxindex;
}


HTMLResource* BookNormalization::GetCoverPage( const QList< HTMLResource* > &html_resources, Book &book )
{
    QString oebps_path = book.GetOPF().GetCoverPageOEBPSPath();
    foreach( HTMLResource* html_resource, html_resources )
    {
        if ( html_resource->GetRelativePathToOEBPS() == oebps_path )

            return html_resource;
    }

    return NULL;
}


bool BookNormalization::CoverPageExists( Book &book )
{
    return !book.GetOPF().GetCoverPageOEBPSPath().isEmpty();
}


void BookNormalization::TryToSetCoverPage( const QList< HTMLResource* > &html_resources, Book &book )
{
    HTMLResource *first_html = html_resources[ 0 ];

    if ( IsFlowUnderThreshold( first_html, FLOW_SIZE_THRESHOLD ) &&
         FlowHasOnlyOneImage( first_html )
       )
    {
        book.GetOPF().AddGuideSemanticType( *first_html, GuideSemantics::Cover );
    }
}


void BookNormalization::TryToSetCoverImage( const QList< HTMLResource* > &html_resources, 
                                            const QList< ImageResource* > &image_resources,
                                            Book &book )
{
    HTMLResource *cover_page = GetCoverPage( html_resources, book );

    if ( !cover_page )

        return;

    QStringList image_paths;

    {
        QReadLocker locker( &cover_page->GetLock() );

        image_paths = XhtmlDoc::GetImagePathsFromImageChildren( cover_page->GetDomDocumentForReading() );
    }

    if ( image_paths.count() == 0 )
         
        return;

    QString first_image_name = QFileInfo( image_paths[ 0 ] ).fileName();

    foreach( ImageResource *image_resource, image_resources )
    {
        if ( image_resource->Filename() == first_image_name )
        {
            book.GetOPF().SetResourceAsCoverImage( *image_resource );
            break;
        }
    }
}


bool BookNormalization::IsFlowUnderThreshold( HTMLResource *html_resource, int threshold )
{
    QReadLocker locker( &html_resource->GetLock() );

    xc::DOMElement &doc_element = *html_resource->GetDomDocumentForReading().getDocumentElement();
    return XtoQ( doc_element.getTextContent() ).count() < threshold;
}

bool BookNormalization::FlowHasOnlyOneImage( HTMLResource* html_resource )
{
    return XhtmlDoc::GetImagePathsFromImageChildren( html_resource->GetDomDocumentForReading() ).count() == 1;
}


