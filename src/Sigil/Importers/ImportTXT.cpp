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
#include "ImportTXT.h"
#include "BookManipulation/CleanSource.h"
#include "Misc/Utility.h"
#include "Misc/TempFolder.h"
#include "ResourceObjects/HTMLResource.h"
#include "BookManipulation/XhtmlDoc.h"
#include "BookManipulation/FolderKeeper.h"

const QString FIRST_CHAPTER_PREFIX = "Section";
const QString FIRST_CHAPTER_NAME   = FIRST_CHAPTER_PREFIX + "0001.xhtml";

// Constructor;
// The parameter is the file to be imported
ImportTXT::ImportTXT( const QString &fullfilepath )
    : Importer( fullfilepath )
{

}


// Reads and parses the file 
// and returns the created Book
QSharedPointer< Book > ImportTXT::GetBook()
{
    if ( !Utility::IsFileReadable( m_FullFilePath ) )
        
        boost_throw( CannotReadFile() << errinfo_file_fullpath( m_FullFilePath.toStdString() ) );
    
    QString source = LoadSource();

    InitializeHTMLResource( source, CreateHTMLResource( source ) );

    return m_Book;
}


QString ImportTXT::LoadSource() const
{
    QString source = Utility::ReadUnicodeTextFile( m_FullFilePath );   

    source = CreateParagraphs( source.split( QChar( '\n' ) ) );
    return CleanSource::Clean( source );    
}


HTMLResource* ImportTXT::CreateHTMLResource( const QString &source )
{
    TempFolder tempfolder;

    QString fullfilepath = tempfolder.GetPath() + "/" + FIRST_CHAPTER_NAME;
    Utility::WriteUnicodeTextFile( source, fullfilepath );

    m_Book->GetFolderKeeper().AddContentFileToFolder( fullfilepath, 0 );

    return m_Book->GetFolderKeeper().GetResourceTypeList< HTMLResource >()[ 0 ];
}


void ImportTXT::InitializeHTMLResource( const QString &source, HTMLResource *resource )
{
    resource->SetDomDocument( XhtmlDoc::LoadTextIntoDocument( source ) );
}


// Accepts a list of text lines and returns
// a string with paragraphs wrapped into <p> tags
QString ImportTXT::CreateParagraphs( const QStringList &lines ) const
{
    QString text = "";
    QString paragraph = "<p>";

    int num_lines = lines.count();

    for ( int i = 0; i < num_lines; ++i )
    {
        QString line = lines.at( i );

        if ( line.isEmpty() || line[ 0 ].isSpace() )
        {
            text.append( paragraph.append( "</p>\n" ) );

            paragraph = "<p>";
        }

        // We prepend a space so words on 
        // line breaks don't get merged
        paragraph.append( Qt::escape( line.prepend( " " ) ) );
    }

    text.append( paragraph.append( "</p>\n" ) );

    return text;
}


