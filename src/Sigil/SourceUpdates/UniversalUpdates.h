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
#ifndef UNIVERSALUPDATES_H
#define UNIVERSALUPDATES_H

class HTMLResource;
class CSSResource;
class OPFResource;
class NCXResource;
class Resource;


class UniversalUpdates
{

public:

    static void PerformUniversalUpdates( bool resources_already_loaded,
                                         const QList< Resource* > &resources,
                                         const QHash< QString, QString > &updates );

    static tuple< QHash< QString, QString >, 
                  QHash< QString, QString >,
                  QHash< QString, QString > > SeparateHtmlCssXmlUpdates( const QHash< QString, QString > &updates );

    // Made public so that ImportHTML can use it
    static void LoadAndUpdateOneCSSFile( CSSResource* css_resource, 
                                         const QHash< QString, QString > &css_updates );

private:

    static void UpdateOneHTMLFile( HTMLResource* html_resource, 
                                   const QHash< QString, QString > &html_updates,
                                   const QHash< QString, QString > &css_updates );

    static void UpdateOneCSSFile( CSSResource* css_resource, 
                                  const QHash< QString, QString > &css_updates );

    static void LoadAndUpdateOneHTMLFile( HTMLResource* html_resource, 
                                          const QHash< QString, QString > &html_updates,
                                          const QHash< QString, QString > &css_updates );

    static void UpdateOPFFile( OPFResource* opf_resource,
                               const QHash< QString, QString > &xml_updates );

    static void UpdateNCXFile( NCXResource* ncx_resource,
                               const QHash< QString, QString > &xml_updates );
};

#endif // UNIVERSALUPDATES_H