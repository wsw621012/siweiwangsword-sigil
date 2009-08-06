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
#ifndef SIGILMARKUP_H
#define SIGILMARKUP_H

// Contains an interface for adding Sigil-specific markup to
// an XHTML source... Sigil-specific markup is for instance
// our special CSS for our chapter breaks
class SigilMarkup
{

public:

    // Adds all Sigil-specific markup
    static QString AddSigilMarkup( const QString &source );

private:

    // Adds our CSS style tags
    static QString AddSigilStyleTags( const QString &source );
};

#endif // SIGILMARKUP_H