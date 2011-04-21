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
#ifndef IMPORTER_H
#define IMPORTER_H

#include <QSharedPointer>
#include "BookManipulation/Book.h"

/**
 * The abstract base class for Importers. 
 * Defines the GetBook() method that the importer subclasses need to implement. 
 */
class Importer
{

public:

    /**
     * Constructor.
     * 
     * @param fullfilepath The path to the file to be imported.
     */
    Importer( const QString &fullfilepath );

    /**
     * Destructor.
     */
    virtual ~Importer() {}

    /**
     * Loads the file as a Book and returns it.
     *
     * @return The file as a Book object, RAII wrapped.
     */
    virtual QSharedPointer< Book > GetBook() = 0;

protected:

    ///////////////////////////////
    // PROTECTED MEMBER VARIABLES
    ///////////////////////////////

    /**
     * The full path to the file to be imported.
     */
    const QString &m_FullFilePath;

    /**
     * The Book that will be created  
     * by the importing process.
     */ 
    QSharedPointer< Book > m_Book;
};

#endif // IMPORTER_H


