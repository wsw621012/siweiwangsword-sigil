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
#include "../BookManipulation/Metadata.h"

static const QString PATH_TO_LANGUAGES  = ":/data/languages.csv";
static const QString PATH_TO_BASICMETA  = ":/data/basicmeta.csv";
static const QString PATH_TO_RELATORS   = ":/data/relator.csv";

Metadata & Metadata::Instance()
{
    // We use a static local variable
    // to hold our singleton instance; using a pointer member
    // variable creates problems with object destruction;
    // Sigil is single-threaded so this is ok
    static Metadata meta;

    return meta;
}

const QMap< QString, QString > & Metadata::GetLanguageMap()
{
    return m_Languages;
}


const QMap< QString, Metadata::MetaInfo > & Metadata::GetRelatorMap()
{
    return m_Relators;
}


const QMap< QString, Metadata::MetaInfo > & Metadata::GetBasicMetaMap()
{
    return m_Basic;
}


const QHash< QString, QString > & Metadata::GetFullRelatorNameHash()
{
    return m_FullRelators;
}


const QHash< QString, QString > & Metadata::GetFullLanguageNameHash()
{
    return m_FullLanguages;
}


// Maps Dublic Core metadata to internal book meta format
Metadata::MetaElement Metadata::MapToBookMetadata( const Metadata::MetaElement &meta, const QString &type )
{
    QString name = meta.name.toLower();

    if ( ( type == "HTML" ) && ( !name.startsWith( "dc." ) ) && ( !name.startsWith( "dcterms." ) ) )  

        return FreeFormMetadata( meta );

    // Dublin Core

    MetaElement working_copy_meta;

    if ( type == "HTML" )
    {
        // transform html based dublin core to opf style metaelement
        working_copy_meta = HtmlToOpfDC( meta );
    }

    else
    {
        working_copy_meta = meta;
    }

    name = working_copy_meta.name.toLower();

    if ( ( name == "creator" ) || ( name == "contributor" ) )

        return CreateContribMetadata( working_copy_meta );

    if ( name == "date" )

        return DateMetadata( working_copy_meta );

    if ( name == "identifier" )

        return IdentifierMetadata( working_copy_meta );

    QString value = meta.value.toString();

    if ( name == "language" )
    {
        // We convert ISO 639-1 language code into full language name (e.g. en -> English)
        value = GetFullLanguageNameHash()[ value ];
        // fall through
    }

    MetaElement book_meta;

    if ( ( !name.isEmpty() ) && ( !value.isEmpty() ) )
    {
        book_meta.name  = name[ 0 ].toUpper() + name.mid( 1 );
        book_meta.value = value;
    }

    return book_meta;
}


Metadata::Metadata()
{
    LoadLanguages();
    LoadBasicMetadata();
    LoadRelatorCodes();
}


// Loads the languages and their codes from disk
void Metadata::LoadLanguages()
{
    // If the languages have already been loaded
    // by a previous Meta Editor, then don't load them again
    if ( !m_Languages.isEmpty() )

        return;

    QFile file( PATH_TO_LANGUAGES );

    // Check if we can open the file
    if ( !file.open( QFile::ReadOnly | QFile::Text ) )
    {
        QMessageBox::warning(	0,
            QObject::tr( "Sigil" ),
            QObject::tr( "Cannot read file %1:\n%2." )
            .arg( PATH_TO_LANGUAGES )
            .arg( file.errorString() ) 
            );

        return;
    }

    QTextStream in( &file );

    in.setCodec( "UTF-8" );

    // This will automatically switch reading from
    // UTF-8 to UTF-16 if a BOM is detected
    in.setAutoDetectUnicode( true );

    while ( in.atEnd() == false )
    {
        QString line = in.readLine();

        QStringList fields = line.split( "|" );

        m_Languages[ fields[ 0 ] ]      = fields[ 1 ];
        m_FullLanguages[ fields[ 1 ] ]  = fields[ 0 ];
    }	
}


// Loads the basic metadata and their descriptions from disk
void Metadata::LoadBasicMetadata()
{
    // If the basic metadata has already been loaded
    // by a previous Meta Editor, then don't load them again
    if ( !m_Basic.isEmpty() )

        return;

    QFile file( PATH_TO_BASICMETA );

    // Check if we can open the file
    if ( !file.open( QFile::ReadOnly | QFile::Text ) )
    {
        QMessageBox::warning(	0,
            QObject::tr( "Sigil" ),
            QObject::tr( "Cannot read file %1:\n%2." )
            .arg( PATH_TO_BASICMETA )
            .arg( file.errorString() ) 
            );

        return;
    }

    QTextStream in( &file );

    in.setCodec( "UTF-8" );

    // This will automatically switch reading from
    // UTF-8 to UTF-16 if a BOM is detected
    in.setAutoDetectUnicode( true );

    while ( in.atEnd() == false )
    {
        QString line = in.readLine();

        QStringList fields = line.split( "|" );

        MetaInfo meta;

        meta.relator_code = "";
        meta.description  = fields[ 1 ];

        m_Basic[ fields[ 0 ] ] = meta;
    }
}


// Loads the relator codes, their full names,
// and their descriptions from disk
void Metadata::LoadRelatorCodes()
{
    // If the relator codes have already been loaded
    // by a previous Meta Editor, then don't load them again
    if ( !m_Relators.isEmpty() )

        return;

    QFile file( PATH_TO_RELATORS );

    // Check if we can open the file
    if ( !file.open( QFile::ReadOnly | QFile::Text ) )
    {
        QMessageBox::warning(	0,
            QObject::tr( "Sigil" ),
            QObject::tr( "Cannot read file %1:\n%2." )
            .arg( PATH_TO_RELATORS )
            .arg( file.errorString() ) 
            );

        return;
    }

    QTextStream in( &file );

    in.setCodec( "UTF-8" );

    // This will automatically switch reading from
    // UTF-8 to UTF-16 if a BOM is detected
    in.setAutoDetectUnicode( true );

    while ( in.atEnd() == false )
    {
        QString line = in.readLine();

        QStringList fields = line.split( "|" );

        MetaInfo meta;

        meta.relator_code   = fields[ 1 ];
        meta.description    = fields[ 2 ];

        m_Relators[ fields[ 0 ] ]       = meta;
        m_FullRelators[ fields[ 1 ] ]   = fields[ 0 ];
    }
}


// Converts HTML sourced Dublin Core metadata to OPF style metadata
Metadata::MetaElement Metadata::HtmlToOpfDC( const Metadata::MetaElement &meta )
{
    // Dublin Core from html file with the original 15 element namespace or
    // expanded dcterms namespace.  allows qualifiers as refinements
    // prefix.name[.refinement]

    QStringList fields = QString( meta.name.toLower() + ".." ).split( "." );
    QString name       = fields[ 1 ];
    QString refinement = fields[ 2 ];
    
    QString dc_event;

    if ( ( name == "modifed" ) || ( refinement == "modified" ) )
    {
        name     = "date";
        dc_event = "modification";
    }

    else if ( ( name == "created" ) || ( refinement == "created" ) ) 
    {
        name     = "date";
        dc_event = "creation";
    }

    else if ( ( name == "issued" ) || ( refinement == "issued" ) )
    {
        name     = "date";
        dc_event = "publication";
    }

    QString role;

    if ( ( name == "creator" ) || ( name == "contributor" ) ) 

        role = refinement;

    QString scheme = meta.attributes.value( "scheme" );

    if ( ( name == "identifier" ) && ( scheme.isEmpty() ) )

        scheme = refinement;

    if ( !scheme.isEmpty() )
    {
        QStringList scheme_list;
        scheme_list << "ISBN" << "ISSN" << "DOI" << "CustomID";

        if ( scheme_list.contains( scheme, Qt::CaseInsensitive ) )

            scheme = scheme_list.filter( scheme, Qt::CaseInsensitive )[ 0 ];
    }

    MetaElement opf_meta;
    opf_meta.name  = name;
    opf_meta.value = meta.value;

    if ( !scheme.isEmpty() )

        opf_meta.attributes[ "scheme" ] = scheme;

    if ( !dc_event.isEmpty() )

        opf_meta.attributes[ "event" ] = dc_event;

    if ( !role.isEmpty() )

        opf_meta.attributes[ "role" ] = role;

    return opf_meta;
}    


// Converts free form metadata into internal book metadata
Metadata::MetaElement Metadata::FreeFormMetadata( const Metadata::MetaElement &meta )
{
    // non - dublin core meta info from html file, if this maps to 
    // one of the metadata basic fields used internally pass it through
    // i.e. Author, Title, Publisher, Rights/CopyRight, EISBN/ISBN

    QString name = meta.name.toLower();

    // remap commonly used meta values to match internal names
    name =  name == "copyright" ? "Rights"   :
            name == "eisbn"     ? "ISBN"     :
            name == "issn"      ? "ISSN"     :
            name == "doi"       ? "DOI"      :
            name == "customid"  ? "CustomID" :
            name[ 0 ].toUpper() + name.mid( 1 );
    
    MetaElement book_meta;

    if ( m_Basic.contains( name ) || 
         name == "Author" ||
         name == "Title" 
       )
    {
        book_meta.name = name;
        book_meta.value = meta.value;
    }

    return book_meta;
}


// Converts dc:creator and dc:contributor metadata to book internal metadata
Metadata::MetaElement Metadata::CreateContribMetadata( const Metadata::MetaElement &meta )
{
    QString role = meta.attributes.value( "role", "aut" );

    // We convert the role into the new metadata name (e.g. aut -> Author)
    QString name = GetFullRelatorNameHash()[ role ];

    // If a "file-as" attribute is provided, we use that as the value
    QString file_as = meta.attributes.value( "file-as" );

    QString value = meta.value.toString();

    if ( !file_as.isEmpty() )

        value = file_as;

    name = name[ 0 ].toUpper() + name.mid( 1 );

    MetaElement book_meta;
    book_meta.name = name;
    book_meta.value = value;

    return book_meta;
}


// Converts dc:date metadata to book internal metadata
Metadata::MetaElement Metadata::DateMetadata( const Metadata::MetaElement &meta )
{
    QStringList eventList;
    eventList << "creation" << "publication" << "modification";

    QString name     = meta.name;
    QString dc_event = meta.attributes.value( "event" );

    name = "Date of publication";  // default

    if ( eventList.contains( dc_event ) )
    
        name = "Date of " + dc_event;

    // Dates are in YYYY[-MM[-DD]] format
    QStringList date_parts = meta.value.toString().split( "-", QString::SkipEmptyParts );

    if ( date_parts.count() < 1 )

        date_parts.append( QString::number( QDate::currentDate().year() ) );

    if ( date_parts.count() < 2 )

        date_parts.append( "01" );

    if ( date_parts.count() < 3 )

        date_parts.append( "01" );

    QVariant value = QDate( date_parts[ 0 ].toInt(), 
                            date_parts[ 1 ].toInt(), 
                            date_parts[ 2 ].toInt() );

    MetaElement book_meta;
    book_meta.name = name;
    book_meta.value = value;

    return book_meta;
}


// Converts dc:identifier metadata to book internal metadata
Metadata::MetaElement Metadata::IdentifierMetadata( const Metadata::MetaElement &meta )
{
    QStringList schemeList;
    schemeList << "ISBN" << "ISSN" << "DOI" << "CustomID"; 

    QString scheme = meta.attributes.value("scheme");

    MetaElement book_meta;

    if ( schemeList.contains( scheme ) )
    {
        book_meta.name = scheme;
        book_meta.value = meta.value;
    }

    return book_meta;
}



