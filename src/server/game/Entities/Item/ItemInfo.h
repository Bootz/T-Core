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

class ItemInfo;
struct ItemEntry;
struct ItemSparseEntry;

class ItemInfo
{
public:
    // Item.db2
    uint32     ItemId;
    uint32     Class;
    uint32     SubClass;
    int32      Unk0;
    int32      Material;
    uint32     DisplayId;
    uint32     InventoryType;
    uint32     Sheath;
    // Item-sparse.db2
    uint32     Quality;
    uint32     Flags;
    uint32     Flags2;
    uint32     BuyPrice;
    uint32     SellPrice;
    //uint32     InventoryType;
    int32      AllowableClass;
    int32      AllowableRace;
    uint32     ItemLevel;
    int32      RequiredLevel;
    uint32     RequiredSkill;
    uint32     RequiredSkillRank;
    uint32     RequiredSpell;
    uint32     RequiredHonorRank;
    uint32     RequiredCityRank;
    uint32     RequiredReputationFaction;
    uint32     RequiredReputationRank;
    uint32     MaxCount;
    uint32     Stackable;
    uint32     ContainerSlots;
    int32      ItemStatType[MAX_ITEM_PROTO_STATS];
    uint32     ItemStatValue[MAX_ITEM_PROTO_STATS];
    uint32     ScalingStatDistribution;
    uint32     DamageType;
    uint32     Delay;
    float      RangedModRange;
    int32      SpellId[MAX_ITEM_PROTO_SPELLS];
    int32      SpellTrigger[MAX_ITEM_PROTO_SPELLS];
    int32      SpellCharges[MAX_ITEM_PROTO_SPELLS];
    int32      SpellCooldown[MAX_ITEM_PROTO_SPELLS];
    int32      SpellCategory[MAX_ITEM_PROTO_SPELLS];
    int32      SpellCategoryCooldown[MAX_ITEM_PROTO_SPELLS];
    uint32     Bonding;
    DBC2String Name;
    DBC2String Description;
    uint32     PageText;
    uint32     LanguageID;
    uint32     PageMaterial;
    uint32     StartQuest;
    uint32     LockID;
    //int32      Material;
    //uint32     Sheath;
    uint32     RandomProperty;
    uint32     RandomSuffix;
    uint32     ItemSet;
    uint32     MaxDurability;
    uint32     Area;
    uint32     Map;
    uint32     BagFamily;
    uint32     TotemCategory;
    uint32     Color[MAX_ITEM_PROTO_SOCKETS];
    uint32     Content[MAX_ITEM_PROTO_SOCKETS];
    int32      SocketBonus;
    uint32     GemProperties;
    float      ArmorDamageModifier;
    uint32     Duration;
    uint32     ItemLimitCategory;
    uint32     HolidayId;
    float      StatScalingFactor;

    // Functions
    ItemInfo(ItemEntry const* itemEntry);
    ItemSparseEntry const* GetItemSparse() const;
};

#endif // _ITEMINFO_H
