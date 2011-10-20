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

#ifndef HEADERS_H
#define HEADERS_H

#define __INCLUDE_CRYPTOGRAPHY__
#define __STORMLIB_SELF__
#include <stdio.h>
#include <cassert>
#include <iostream>
#include <string>
#include <fstream>
#include <set>
#include <algorithm>

#include <fcntl.h>

#ifdef _MSC_VER
#include <crtdbg.h>
#endif

#include "StormLib.h"
#include "StormCommon.h"
#include "typedefs.h"

static int const patchRev[] = {13164, 13205, 13287, 13329, 13596, 13623, 13914, 14007, 14333, 14480}; 
#define PATCH_REV_COUNT 10 
#define CLIENT_BUILD 14480
#define MAX_EXPANSION 3

static const char* const langs[] = {"enGB", "enUS", "deDE", "esES", "frFR", "koKR", "zhCN", "zhTW", "enCN", "enTW", "esMX", "ruRU" };
#define LANG_COUNT 12
static const char* input_path = ".";

typedef struct
{
    char name[64];
    uint32 id;
} map_id;

extern HANDLE localeMPQ[PATCH_REV_COUNT+1];
extern HANDLE WorldMPQ;
extern HANDLE ExpansionsMPQ[3];
extern map_id *map_ids;
extern uint16 *areas;
extern uint16 *LiqType;
extern uint32 maxAreaId;

#endif
