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
#ifndef UTILITY_H
#define UTILITY_H

#include <QString>
#include <QDir>

class Utility
{	

public:

    // Uses QUuid to generate a random UUID but also removes
    // the curly braces that QUuid::createUuid() adds
    static QString CreateUUID();

    // Returns true if the string is mixed case, false otherwise.
    // For instance, "test" and "TEST" return false, "teSt" returns true.
    static bool IsMixedCase( const QString &string );

    // Returns a substring of a specified string;
    // the characters included are in the interval: 
    // [ start_index, end_index >
    static QString Substring( int start_index, int end_index, const QString &string );

    // Replace the first occurrence of string "before"
    // with string "after" in string "string"
    static QString ReplaceFirst( const QString &before, const QString &after, const QString &string );
   
    static QStringList GetAbsolutePathsToFolderDescendantFiles( const QString &fullfolderpath );

    // Copies every file and folder in the source folder 
    // to the destination folder; the paths to the folders are submitted;
    // the destination folder needs to be created in advance
    static void CopyFiles( const QString &fullfolderpath_source, const QString &fullfolderpath_destination );
    
    // Deletes the specified file if it exists
    static bool DeleteFile( const QString &fullfilepath );

    static bool RenameFile( const QString &oldfilepath, const QString &newfilepath );

    // Creates a copy of the provided file with a random name in
    // the systems TEMP directory and returns the full path to the new file.
    // The extension of the original file is preserved. If the original file
    // doesn't exist, an empty string is returned.
    static QString CreateTemporaryCopy( const QString &fullfilepath ); 

    // Returns true if the file can be read;
    // shows an error dialog if it can't
    // with a message elaborating what's wrong
    static bool IsFileReadable( const QString &fullfilepath );

    // Reads the text file specified with the full file path;
    // text needs to be in UTF-8 or UTF-16; if the file cannot
    // be read, an error dialog is shown and an empty string returned
    static QString ReadUnicodeTextFile( const QString &fullfilepath );

    // Writes the provided text variable to the specified
    // file; if the file exists, it is truncated
    static void WriteUnicodeTextFile( const QString &text, const QString &fullfilepath );

    // Converts Mac and Windows style line endings to Unix style
    // line endings that are expected throughout the Qt framework
    static QString ConvertLineEndings( const QString &text );
    
    /**
     * URL encodes the provided path string.
     * The path separator ('/') and the ID hash ('#') are left alone.
     *
     * @param path The path to encode.
     * @return The encoded path string.
     */
    static QString URLEncodePath( const QString &path );

    /**
     * URL decodes the provided path string.
     *
     * @param path The path to decode.
     * @return The decoded path string.
     */
    static QString URLDecodePath( const QString &path );

    static void DisplayStdErrorDialog( const QString &error_message, const QString &detailed_text = QString() );

    static void DisplayExceptionErrorDialog( const QString &error_info );

    static QString GetExceptionInfo( const ExceptionBase &exception );

    // Returns a value for the environment variable name passed;
    // if the env var isn't set, it returns an empty string
    static QString GetEnvironmentVar( const QString &variable_name );

    // Returns the same number, but rounded to one decimal place
    static float RoundToOneDecimal( float number );
};

#endif // UTILITY_H


