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
#ifndef METADATA_H
#define METADATA_Hc

#include <QMutex>
#include "BookManipulation/XercesHUse.h"


class Metadata
{	

public:

    struct MetaInfo
    {
        // The code for the relator (e.g "aut")
        QString relator_code;

        // The description of the relator
        QString description;
    };

    struct MetaElement
    {
        // The name of the element
        QString name;

        // The value of the element
        QVariant value;

        // The attributes of the element;
        // the keys are the attribute names,
        // the values are the attribute values
        QHash< QString, QString > attributes;
    };

    static Metadata& Instance();

    const QMap< QString, QString >&  GetLanguageMap();
    const QMap< QString, MetaInfo >& GetRelatorMap();
    const QMap< QString, MetaInfo >& GetBasicMetaMap();
    const QHash< QString, QString >& GetFullRelatorNameHash();
    const QHash< QString, QString >& GetFullLanguageNameHash();

    /**
     * Maps DC and <meta> metadata elements to "internal" MetaElements.
     * Accepts both DublinCore metadata elements like one would find in an 
     * OPF and custom <meta> elements like one would find in an HTML file.
     * 
     * @param element The element to convert.
     * @return The converted MetaElement.
     */
    MetaElement MapToBookMetadata( const xc::DOMElement &element );

private:

    // Maps Dublic Core metadata to internal book meta format
    MetaElement MapToBookMetadata( const MetaElement &meta, bool is_dc_element );

    // Constructor is private because
    // this is a singleton class
    Metadata();

    // Loads the languages and their codes from disk
    void LoadLanguages();

    // Loads the basic metadata and their descriptions from disk
    void LoadBasicMetadata();

    // Loads the relator codes, their full names,
    // and their descriptions from disk
    void LoadRelatorCodes();

    // Converts HTML sourced Dublin Core metadata to OPF style metadata
    MetaElement HtmlToOpfDC( const MetaElement &meta );

    // Converts free form metadata into internal book metadata
    MetaElement FreeFormMetadata( const MetaElement &meta );

    // Converts dc:creator and dc:contributor metadata to book internal metadata
    MetaElement CreateContribMetadata( const MetaElement &meta );

    // Converts dc:date metadata to book internal metadata
    MetaElement DateMetadata( const MetaElement &meta );

    // Converts dc:identifier metadata to book internal metadata
    MetaElement IdentifierMetadata( const MetaElement &meta );


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    static QMutex s_AccessMutex;

    static Metadata *m_Instance;

    // The keys are the full English language names
    // and the values are the ISO 639-1 language codes
    // (see http://www.loc.gov/standards/iso639-2/php/English_list.php );
    QMap< QString, QString > m_Languages;

    // The keys are the ISO 639-1 language codes
    // and the values are the full English language names
    // (see http://www.loc.gov/standards/iso639-2/php/English_list.php );
    QHash< QString, QString > m_FullLanguages;

    // The keys are the Dublin Core element names
    // and the values are the MetaInfo structures
    // (see http://www.idpf.org/2007/opf/OPF_2.0_final_spec.html#Section2.2 );
    QMap< QString, MetaInfo > m_Basic;

    // The keys are the full names of relators
    // and the values are the MetaInfo structures
    // (see http://www.loc.gov/marc/relators/relaterm.html );
    QMap< QString, MetaInfo > m_Relators;

    // The keys are the MARC relator codes and
    // the values are the full names of those relators
    // (e.g. aut -> Author )
    QHash< QString, QString > m_FullRelators;

};

#endif // METADATA_H

