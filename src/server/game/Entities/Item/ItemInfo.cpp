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

#include "ItemInfo.h"

ItemInfo::ItemInfo(ItemEntry const* itemEntry)
{
    ItemId = itemEntry->ID;
    Class  = itemEntry->Class;
    SubClass = itemEntry->SubClass;
    Unk0 = itemEntry->Unk0;
    Material = itemEntry->Material;
    DisplayId = itemEntry->DisplayId;
    InventoryType = itemEntry->InventoryType;
    Sheath = itemEntry->Sheath;

    // There are many items not in the Item-sparse.db2, so we need to check
    ItemSparseEntry const* _item = GetItemSparse();
    Quality = _item ? _item->Quality : 0;
    Flags = _item ? _item->Flags : 0;
    Flags2 = _item ? _item->Flags2 : 0;
    BuyPrice = _item ? _item->BuyPrice : 0;
    SellPrice = _item ? _item->SellPrice : 0;
    AllowableClass = _item ? _item->AllowableClass : -1;
    AllowableRace = _item ? _item->AllowableRace : -1;
    ItemLevel = _item ? _item->ItemLevel : 0;
    RequiredLevel = _item ? _item->RequiredLevel : 0;
    RequiredSkill = _item ? _item->RequiredSkill : 0;
    RequiredSkillRank = _item ? _item->RequiredSkillRank : 0;
    RequiredSpell = _item ? _item->RequiredSpell : 0;
    RequiredHonorRank = _item ? _item->RequiredHonorRank : 0;
    RequiredCityRank = _item ? _item->RequiredCityRank : 0;
    RequiredReputationFaction = _item ? _item->RequiredReputationFaction : 0;
    RequiredReputationRank = _item ? _item->RequiredReputationRank : 0;
    MaxCount = _item ? _item->MaxCount : 0;
    Stackable = _item ? _item->Stackable : 0;
    ContainerSlots = _item ? _item->ContainerSlots : 0;
    for (uint8 i = 0; i < MAX_ITEM_PROTO_STATS; i++)
    {
        ItemStatType[i] = _item ? _item->ItemStatType[i] : 0;
        ItemStatValue[i] = _item ? _item->ItemStatValue[i] : 0;
    }
    ScalingStatDistribution = _item ? _item->ScalingStatDistribution : 0;
    DamageType = _item ? _item->DamageType : 0;
    Delay = _item ? _item->Delay : 0;
    RangedModRange = _item ? _item->RangedModRange : 0.0f;
    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; i++)
    {
         SpellId[i] = _item ? _item->SpellId[i] : 0;
         SpellTrigger[i] = _item ? _item->SpellTrigger[i] : 0;
         SpellCharges[i] = _item ? _item->SpellCharges[i] : 0;
         SpellCooldown[i] = _item ? _item->SpellCooldown[i] : 0;
         SpellCategory[i] = _item ? _item->SpellCategory[i] : 0;
         SpellCategoryCooldown[i] = _item ? _item->SpellCategoryCooldown[i] : 0;
    }
    Bonding = _item ? _item->Bonding : 0;
    Name = _item ? _item->Name : "";
    Description = _item ? _item->Description : "";
    PageText = _item ? _item->PageText : 0;
    LanguageID = _item ? _item->LanguageID : 0;
    PageMaterial = _item ? _item->PageMaterial : 0;
    StartQuest = _item ? _item->StartQuest : 0;
    LockID = _item ? _item->LockID : 0;
    RandomProperty = _item ? _item->RandomProperty : 0;
    RandomSuffix = _item ? _item->RandomSuffix : 0;
    ItemSet = _item ? _item->ItemSet : 0;
    MaxDurability = _item ? _item->MaxDurability : 0;
    Area = _item ? _item->Area : 0;
    Map = _item ? _item->Map : 0;
    BagFamily = _item ? _item->BagFamily : 0;
    TotemCategory = _item ? _item->TotemCategory : 0;
    for (uint8 i = 0; i < MAX_ITEM_PROTO_SOCKETS; i++)
    {
        Color[i] = _item ? _item->Color[i] : 0;
        Content[i] = _item ? _item->Content[i] : 0;
    }
    SocketBonus = _item ? _item->SocketBonus : 0;
    GemProperties = _item ? _item->GemProperties : 0;
    ArmorDamageModifier = _item ? _item->ArmorDamageModifier : 0.0f;
    Duration = _item ? _item->Duration : 0;
    ItemLimitCategory = _item ? _item->ItemLimitCategory : 0;
    HolidayId = _item ? _item->HolidayId : 0;
    StatScalingFactor = _item ? _item->StatScalingFactor : 0;

    // There are many items not in the ItemTemplate, so we need to check
    ItemTemplate const* _proto = sObjectMgr->GetItemTemplate(ItemId);
    BuyCount = _proto ? _proto->BuyCount : 0;
    Block = _proto ? _proto->Block : 0;
    RequiredDisenchantSkill = _proto ? _proto->RequiredDisenchantSkill : 0;
    ScriptId = _proto ? _proto->ScriptId : 0;
    DisenchantID = _proto ? _proto->DisenchantID : 0;
    FoodType = _proto ? _proto->FoodType : 0;
    MinMoneyLoot = _proto ? _proto->MinMoneyLoot : 0;
    MaxMoneyLoot = _proto ? _proto->MaxMoneyLoot : 0;
}

ItemSparseEntry const* ItemInfo::GetItemSparse() const
{
    return ItemId ? sItemSparseStore.LookupEntry(ItemId) : NULL;
}
