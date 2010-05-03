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
#ifndef SEARCHOPERATIONS_H
#define SEARCHOPERATIONS_H

class Resource;
class TextResource;
class HTMLResource;

class SearchOperations
{

public:

    enum SearchType
    {
        BookViewSearch,
        CodeViewSearch
    };

    /**
     * Returns the number of matching occurrences.
     *
     * @param search_regex The regex to match with.
     * @return The number of matching occurrences.
     */
    static int CountInFiles( const QRegExp &search_regex,
                             QList< Resource* > resources,
                             SearchType search_type );

    static int ReplaceInAllFIles( const QRegExp &search_regex,
                                  const QString &replacement,
                                  QList< Resource* > resources,
                                  SearchType search_type );

private:

    static int CountInFile( const QRegExp &search_regex,
                            Resource* resource,
                            SearchType search_type );

    static int CountInHTMLFile( const QRegExp &search_regex,
                                HTMLResource* html_resource,
                                SearchType search_type  );

    static int CountInTextFile( const QRegExp &search_regex,
                                TextResource* text_resource );

    static int ReplaceInFile( const QRegExp &search_regex,
                              const QString &replacement,
                              Resource* resource,
                              SearchType search_type );

    static int ReplaceHTMLInFile( const QRegExp &search_regex,
                                  const QString &replacement,
                                  HTMLResource* html_resource,
                                  SearchType search_type );

    static int ReplaceTextInFile( const QRegExp &search_regex,
                                  const QString &replacement,
                                  TextResource* text_resource );

    static tuple< QString, int > PerformGlobalReplace( const QString &text,
                                         const QRegExp &search_regex,
                                         const QString &replacement );

    static void Accumulate( int &first, const int &second );
};

#endif // SEARCHOPERATIONS_H
