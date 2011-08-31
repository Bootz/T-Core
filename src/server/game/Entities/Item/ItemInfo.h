/*
* Copyright (C) 2005 - 2011 MaNGOS <http://www.getmangos.org/>
*
* Copyright (C) 2008 - 2011 TrinityCore <http://www.trinitycore.org/>
*
* Copyright (C) 2011 TrilliumEMU <http://www.trilliumemu.org/>
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

#ifndef _ITEMINFO_H
#define _ITEMINFO_H

#include "SharedDefines.h"

class ItemInfo
{
public:
    // Item.db2
    uint32   ItemId;
    uint32   Class;
    uint32   SubClass;
    int32    Unk0;
    int32    Material;
    uint32   DisplayId;
    uint32   InventoryType;
    uint32   Sheath;
    // Item-sparse.db2

    // Functions
    ItemInfo(ItemEntry const* itemEntry);
};

#endif // _ITEMINFO_H
