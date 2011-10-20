/*
 * Copyright (C) 2011 TrilliumENCORE <http://www.trilliumemu.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FILELOADER_H
#define FILELOADER_H

#include "headers.h"
#include "typedefs.h"
#include "MPQFile.h"

#define FILE_FORMAT_VERSION    18

//
// File version chunk
//
struct file_MVER
{
    union
    {
        uint32 fcc;
        char   fcc_txt[4];
    };
    uint32 size;
    uint32 ver;
};

class FileLoader : public MPQFile
{
public:
    FileLoader(const char* filename, HANDLE handle): MPQFile(filename, handle) {};
    
    virtual bool prepareLoadedData();
    
    file_MVER *version;
};

#endif