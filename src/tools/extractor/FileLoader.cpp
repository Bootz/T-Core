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

#define _CRT_SECURE_NO_DEPRECATE

#include "FileLoader.h"

bool FileLoader::prepareLoadedData()
{
    // Check version
    version = (file_MVER *) buffer;
    if (version->fcc != 'MVER')
        return false;
    if (version->ver != FILE_FORMAT_VERSION)
        return false;
    return true;
}
