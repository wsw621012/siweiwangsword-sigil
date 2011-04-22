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
#include "ImportOEBPS.h"
#include "Misc/Utility.h"
#include "ResourceObjects/Resource.h"
#include "ResourceObjects/OPFResource.h"
#include "ResourceObjects/NCXResource.h"
#include "BookManipulation/FolderKeeper.h"
#include <ZipArchive.h>

const QString DUBLIN_CORE_NS             = "http://purl.org/dc/elements/1.1/";
static const QString OEBPS_MIMETYPE      = "application/oebps-package+xml";
static const QString UPDATE_ERROR_STRING = "SG_ERROR";
const QString NCX_MIMETYPE               = "application/x-dtbncx+xml";
static const QString NCX_EXTENSION       = "ncx";



ImportOEBPS::ImportOEBPS( const QString &fullfilepath )
    : 
    Importer( fullfilepath ),
    m_ExtractedFolderPath( m_TempFolder.GetPath() )
{

}



// TODO: create a wrapper lib for this CZipArchive POS
void ImportOEBPS::ExtractContainer()
{
    CZipArchive zip;

    try
    {
#ifdef Q_WS_WIN
        zip.Open( m_FullFilePath.utf16(), CZipArchive::zipOpenReadOnly );
#else
        zip.Open( m_FullFilePath.toUtf8().data(), CZipArchive::zipOpenReadOnly );
#endif

        int file_count = (int) zip.GetCount();
        QString folder_path = m_ExtractedFolderPath;

#ifdef Q_WS_WIN
        const ushort *win_path = folder_path.utf16();
#else
        QByteArray utf8_path( folder_path.toUtf8() );
        char *nix_path = utf8_path.data();
#endif
        for ( int i = 0; i < file_count; ++i )
        {
#ifdef Q_WS_WIN
            bool success = zip.ExtractFile( i, win_path );
#else
            bool success = zip.ExtractFile( i, nix_path );
#endif
            if ( !success )
            {
                CZipFileHeader* file_header = zip.GetFileInfo( i );
                #ifdef Q_WS_WIN
                std::string filename = QString::fromStdWString( file_header->GetFileName() ).toStdString();
                #else
                std::string filename = QString::fromAscii( file_header->GetFileName().c_str() ).toStdString();
                #endif

                zip.Close(); 

                boost_throw( CannotExtractFile() << errinfo_file_fullpath( filename ) );
            }
        }

        zip.Close(); 
    }

    // We have to to do this here: if we don't wrap
    // this exception and try to catch it "raw" in MainWindow,
    // we get a header name clash from ZipArchive. 
    // The headers are Windows-specific so we can't just rename them.
    catch ( CZipException &exception )
    {
        zip.Close( CZipArchive::afAfterException ); 

        // The error description is always ASCII
#ifdef Q_WS_WIN
        std::string error_description = QString::fromStdWString( exception.GetErrorDescription() ).toStdString();
#else
        std::string error_description = QString::fromAscii( exception.GetErrorDescription().c_str() ).toStdString();
#endif
        boost_throw( CZipExceptionWrapper()
            << errinfo_zip_info_msg( error_description )
            << errinfo_zip_error_id( exception.m_iCause ) );
    }
}


void ImportOEBPS::LocateOPF()
{
    QString fullpath = m_ExtractedFolderPath + "/META-INF/container.xml";
    QXmlStreamReader container( Utility::ReadUnicodeTextFile( fullpath ) );

    while ( !container.atEnd() ) 
    {
        container.readNext();

        if ( container.isStartElement() && 
             container.name() == "rootfile"
           ) 
        {
            if ( container.attributes().hasAttribute( "media-type" ) &&
                 container.attributes().value( "", "media-type" ) == OEBPS_MIMETYPE 
               )
            {
                m_OPFFilePath = m_ExtractedFolderPath + "/" + container.attributes().value( "", "full-path" ).toString();

                // As per OCF spec, the first rootfile element
                // with the OEBPS mimetype is considered the "main" one.
                break;
            }
        }
    }

    if ( container.hasError() )
    {
        boost_throw( ErrorParsingContentXml() 
                     << errinfo_XML_parsing_error_string(  container.errorString().toStdString() )
                     << errinfo_XML_parsing_line_number(   container.lineNumber() )
                     << errinfo_XML_parsing_column_number( container.columnNumber() )
                   );
    }

    if ( m_OPFFilePath.isEmpty() )
    {
        boost_throw( NoAppropriateOPFFileFound() );    
    }
}


void ImportOEBPS::ReadOPF()
{
    QString opf_text = PrepareOPFForReading( Utility::ReadUnicodeTextFile( m_OPFFilePath ) );
    QXmlStreamReader opf_reader( opf_text );

    while ( !opf_reader.atEnd() ) 
    {
        opf_reader.readNext();

        if ( !opf_reader.isStartElement() ) 

            continue;

        if ( opf_reader.name() == "package" )

            m_UniqueIdentifierId = opf_reader.attributes().value( "", "unique-identifier" ).toString();

        else if ( opf_reader.name() == "identifier" )

            ReadIdentifierElement( opf_reader );

        // Get the list of content files that
        // make up the publication
        else if ( opf_reader.name() == "item" )

            ReadManifestItemElement( opf_reader );

        // We read this just to get the NCX id
        else if ( opf_reader.name() == "spine" )

            ReadSpineElement( opf_reader );   
    }

    if ( opf_reader.hasError() )
    {
        boost_throw( ErrorParsingOpf() 
                     << errinfo_XML_parsing_error_string( opf_reader.errorString().toStdString() )
                     << errinfo_XML_parsing_line_number( opf_reader.lineNumber() )
                     << errinfo_XML_parsing_column_number( opf_reader.columnNumber() )
                   );
    }
}


void ImportOEBPS::ReadIdentifierElement( QXmlStreamReader &opf_reader )
{
    QString id     = opf_reader.attributes().value( "", "id"     ).toString(); 
    QString scheme = opf_reader.attributes().value( "", "scheme" ).toString();
    QString value  = opf_reader.readElementText();

    if ( id == m_UniqueIdentifierId )

        m_UniqueIdentifierValue = value;

    if ( m_UuidIdentifierValue.isEmpty() &&
         ( value.contains( "urn:uuid:" ) || scheme.toLower() == "uuid" ) )
    {
        m_UuidIdentifierValue = value;
    }            
}


void ImportOEBPS::ReadManifestItemElement( QXmlStreamReader &opf_reader )
{
    QString id   = opf_reader.attributes().value( "", "id"         ).toString(); 
    QString href = opf_reader.attributes().value( "", "href"       ).toString();
    QString type = opf_reader.attributes().value( "", "media-type" ).toString();

    // Paths are percent encoded in the OPF, we use "normal" paths internally.
    href = Utility::URLDecodePath( href );
    QString extension = QFileInfo( href ).suffix().toLower();

    if ( type != NCX_MIMETYPE && extension != NCX_EXTENSION )         
    {                    
        if ( !m_MainfestFilePaths.contains( href ) )
        {
            m_Files[ id ] = href;
            m_MainfestFilePaths << href;
        }
    }

    else
    {
        m_NcxCandidates[ id ] = href;
    }
}


void ImportOEBPS::ReadSpineElement( QXmlStreamReader &opf_reader )
{
    QString ncx_id = opf_reader.attributes().value( "", "toc" ).toString();

    m_NCXFilePath = QFileInfo( m_OPFFilePath ).absolutePath() + "/" + m_NcxCandidates[ ncx_id ];
}


void ImportOEBPS::LoadInfrastructureFiles()
{
    m_Book->GetOPF().SetText( Utility::ReadUnicodeTextFile( m_OPFFilePath ) );
    m_Book->GetNCX().SetText( Utility::ReadUnicodeTextFile( m_NCXFilePath ) );
}


QHash< QString, QString > ImportOEBPS::LoadFolderStructure()
{ 
    QList< QString > keys = m_Files.keys();
    int num_files = keys.count();

    QFutureSynchronizer< tuple< QString, QString > > sync;

    for ( int i = 0; i < num_files; ++i )
    {   
        QString id = keys.at( i );
        sync.addFuture( QtConcurrent::run( 
                this, 
                &ImportOEBPS::LoadOneFile, 
                m_Files.value( id ) ) );   
    }

    sync.waitForFinished();

    QList< QFuture< tuple< QString, QString > > > futures = sync.futures();
    int num_futures = futures.count();

    QHash< QString, QString > updates;

    for ( int i = 0; i < num_futures; ++i )
    {
        tuple< QString, QString > result = futures.at( i ).result();
        updates[ result.get< 0 >() ] = result.get< 1 >();
    }

    updates.remove( UPDATE_ERROR_STRING );
    return updates;
}


tuple< QString, QString > ImportOEBPS::LoadOneFile( const QString &path )
{
    QString fullfilepath = QFileInfo( m_OPFFilePath ).absolutePath() + "/" + path;

    try
    {
        Resource &resource = m_Book->GetFolderKeeper().AddContentFileToFolder( fullfilepath, false );
        QString newpath = "../" + resource.GetRelativePathToOEBPS(); 

        return make_tuple( fullfilepath, newpath );
    }
    
    catch ( FileDoesNotExist& )
    {
    	return make_tuple( UPDATE_ERROR_STRING, UPDATE_ERROR_STRING );
    }
}


QString ImportOEBPS::PrepareOPFForReading( const QString &source )
{
    QString source_copy( source );
    QString prefix = source_copy.left( XML_DECLARATION_SEARCH_PREFIX_SIZE );
    QRegExp version( VERSION_ATTRIBUTE );
    prefix.indexOf( version );

    // MASSIVE hack for XML 1.1 "support";
    // this is only for people who specify
    // XML 1.1 when they actually only use XML 1.0
    source_copy.replace( version.pos(), version.matchedLength(), "version=\"1.0\"" );
    
    return source_copy;
}


