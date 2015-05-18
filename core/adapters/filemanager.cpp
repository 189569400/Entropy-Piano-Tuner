/*****************************************************************************
 * Copyright 2015 Haye Hinrichsen, Christoph Wick
 *
 * This file is part of Entropy Piano Tuner.
 *
 * Entropy Piano Tuner is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * Entropy Piano Tuner is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Entropy Piano Tuner. If not, see http://www.gnu.org/licenses/.
 *****************************************************************************/

//=============================================================================
//                          File manager adapter
//=============================================================================

#include "filemanager.h"
#include "../system/eptexception.h"
#include "../system/log.h"

///////////////////////////////////////////////////////////////////////////////
/// \brief Singleton pointer.
///
/// The FileManager is a singleton class, meaning that it admits only a single
/// instance. The unique pointer mSingleton points to this instance.
///////////////////////////////////////////////////////////////////////////////

std::unique_ptr<FileManager> FileManager::mSingleton;

//-----------------------------------------------------------------------------
//                    Get a reference to the singleton
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief FileManager::getSingleton: Get a reference to the singleton
/// \return Pointer to the instance of the singleton.
///////////////////////////////////////////////////////////////////////////////

FileManager &FileManager::getSingleton()
{
    EptAssert(mSingleton,"FileManager should have a singleton pointer");
    return *mSingleton.get();
}


//-----------------------------------------------------------------------------
//                          Open an input stream
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Open an input stream
/// \param stream : Reference to standard C11 stream
/// \param absolute : Absolute path and file name
/// \param mode : Standard C11 reading mode
/// \returns true if the stream could be opened
///////////////////////////////////////////////////////////////////////////////

bool FileManager::open(std::ifstream &stream,
                       const std::string &absolute,
                       std::ios_base::openmode mode)
{
    stream.open(absolute, mode);
    if (!stream)
    {
        WARNING("Stream at '%s' could not be opened.", absolute.c_str());
        return false;
    }
    else
    {
        INFORMATION("Stream at '%s' is open.", absolute.c_str());
        return true;
    }
}


//-----------------------------------------------------------------------------
//                          Open an output stream
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Open an output file stream.
/// \param Reference to the C11 standard output stream.
/// \param Absolute path and filename.
/// \param Standard C11 output mode.
/// \returns true if the stream could be opened successfully
///////////////////////////////////////////////////////////////////////////////

bool FileManager::open(std::ofstream &stream,
                       const std::string &absolute,
                       std::ios_base::openmode mode)
{
    stream.open(absolute, mode);
    if (!stream) {
        WARNING("Stream at '%s' could not be opened.", absolute.c_str());
        return false;
    } else {
        INFORMATION("Stream at '%s' is open.", absolute.c_str());
        return true;
    }
}

