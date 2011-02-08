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
#ifndef EXPORTERFACTORY_H
#define EXPORTERFACTORY_H

#include "Exporter.h"
#include "BookManipulation/Book.h"

class ExporterFactory
{

public:

    // Constructor
    ExporterFactory();

    // Destructor
    ~ExporterFactory();

    // Returns a reference to the exporter
    // appropriate for the given filename
    Exporter& GetExporter( const QString &filename, QSharedPointer< Book > book );

private:

    // The exporter created
    Exporter* m_Exporter;

};

#endif // EXPORTERFACTORY_H

