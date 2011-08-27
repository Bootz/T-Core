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

#ifndef TRILLIUM_DBCSTORES_H
#define TRILLIUM_DBCSTORES_H

#include "Common.h"
#include "Storages.h"
#include "DataStructure.h"

#include <list>

typedef std::list<uint32> SimpleFactionsList;
SimpleFactionsList const* GetFactionTeamList(uint32 faction);

char const* GetPetName(uint32 petfamily, uint32 dbclang);
uint32 GetTalentSpellCost(uint32 spellId);
TalentSpellPos const* GetTalentSpellPos(uint32 spellId);
SpellEffectEntry const* GetSpellEffectEntry(uint32 spellId, uint32 effect);

int32 GetAreaFlagByAreaID(uint32 area_id);                  // -1 if not found
MapEntry const* GetMapEntry(uint32 map_id);
AreaTableEntry const* GetAreaEntryByAreaID(uint32 area_id);
AreaTableEntry const* GetAreaEntryByAreaFlagAndMap(uint32 area_flag, uint32 map_id);
uint32 GetAreaFlagByMapId(uint32 mapid);

WMOAreaTableEntry const* GetWMOAreaTableEntryByTripple(int32 rootid, int32 adtid, int32 groupid);

uint32 GetVirtualMapForMapAndZone(uint32 mapid, uint32 zoneId);

float GetGtSpellScalingValue(int8 class_, uint8 level);

enum ContentLevels
{
    CONTENT_1_60 = 0,
    CONTENT_61_70,
    CONTENT_71_80,
    CONTENT_81_85
};
ContentLevels GetContentLevelsForMapAndZone(uint32 mapid, uint32 zoneId);

bool IsTotemCategoryCompatiableWith(uint32 itemTotemCategoryId, uint32 requiredTotemCategoryId);

void Zone2MapCoordinates(float &x, float &y, uint32 zone);
void Map2ZoneCoordinates(float &x, float &y, uint32 zone);

typedef std::map<uint32/*pair32(map, diff)*/, MapDifficulty> MapDifficultyMap;
MapDifficulty const* GetMapDifficultyData(uint32 mapId, Difficulty difficulty);
MapDifficulty const* GetDownscaledMapDifficultyData(uint32 mapId, Difficulty &difficulty);

uint32 const* /*[MAX_TALENT_TABS]*/ GetTalentTabPages(uint8 cls);

PvPDifficultyEntry const* GetBattlegroundBracketByLevel(uint32 mapid, uint32 level);
PvPDifficultyEntry const* GetBattlegroundBracketById(uint32 mapid, BattlegroundBracketId id);

extern DataStorage <AchievementEntry>             sAchievementStore;
extern DataStorage <AchievementCriteriaEntry>     sAchievementCriteriaStore;
extern DataStorage <AreaTableEntry>               sAreaStore;// recommend access using functions
extern DataStorage <AreaGroupEntry>               sAreaGroupStore;
extern DataStorage <AreaPOIEntry>                 sAreaPOIStore;
extern DataStorage <AreaTriggerEntry>             sAreaTriggerStore;
extern DataStorage <ArmorLocationEntry>           sArmorLocationStore;
extern DataStorage <AuctionHouseEntry>            sAuctionHouseStore;
extern DataStorage <BankBagSlotPricesEntry>       sBankBagSlotPricesStore;
extern DataStorage <BarberShopStyleEntry>         sBarberShopStyleStore;
extern DataStorage <BattlemasterListEntry>        sBattlemasterListStore;
extern DataStorage <ChatChannelsEntry>            sChatChannelsStore;
extern DataStorage <CharStartOutfitEntry>         sCharStartOutfitStore;
extern DataStorage <CharTitlesEntry>              sCharTitlesStore;
extern DataStorage <ChrClassesEntry>              sChrClassesStore;
extern DataStorage <ChrRacesEntry>                sChrRacesStore;
extern DataStorage <ChrPowerTypesEntry>           sChrPowerTypesStore;
extern DataStorage <CinematicSequencesEntry>      sCinematicSequencesStore;
extern DataStorage <CreatureDisplayInfoEntry>     sCreatureDisplayInfoStore;
extern DataStorage <CreatureFamilyEntry>          sCreatureFamilyStore;
extern DataStorage <CreatureSpellDataEntry>       sCreatureSpellDataStore;
extern DataStorage <CreatureTypeEntry>            sCreatureTypeStore;
extern DataStorage <CurrencyTypesEntry>           sCurrencyTypesStore;
extern DataStorage <DestructibleModelDataEntry>   sDestructibleModelDataStore;
extern DataStorage <DungeonEncounterEntry>        sDungeonEncounterStore;
extern DataStorage <DurabilityCostsEntry>         sDurabilityCostsStore;
extern DataStorage <DurabilityQualityEntry>       sDurabilityQualityStore;
extern DataStorage <EmotesEntry>                  sEmotesStore;
extern DataStorage <EmotesTextEntry>              sEmotesTextStore;
extern DataStorage <FactionEntry>                 sFactionStore;
extern DataStorage <FactionTemplateEntry>         sFactionTemplateStore;
extern DataStorage <GameObjectDisplayInfoEntry>   sGameObjectDisplayInfoStore;
extern DataStorage <GemPropertiesEntry>           sGemPropertiesStore;
extern DataStorage <GlyphPropertiesEntry>         sGlyphPropertiesStore;
extern DataStorage <GlyphSlotEntry>               sGlyphSlotStore;

extern DataStorage <GtBarberShopCostBaseEntry>    sGtBarberShopCostBaseStore;
extern DataStorage <GtCombatRatingsEntry>         sGtCombatRatingsStore;
extern DataStorage <GtChanceToMeleeCritBaseEntry> sGtChanceToMeleeCritBaseStore;
extern DataStorage <GtChanceToMeleeCritEntry>     sGtChanceToMeleeCritStore;
extern DataStorage <GtChanceToSpellCritBaseEntry> sGtChanceToSpellCritBaseStore;
extern DataStorage <GtChanceToSpellCritEntry>     sGtChanceToSpellCritStore;
extern DataStorage <GtOCTClassCombatRatingScalarEntry> sGtOCTClassCombatRatingScalarStore;
extern DataStorage <GtOCTHpPerStaminaEntry>       sGtOCTHpPerStaminaStore;
extern DataStorage <GtRegenMPPerSptEntry>         sGtRegenMPPerSptStore;
extern DataStorage <GtSpellScalingEntry>               sGtSpellScalingStore;
extern DataStorage <HolidaysEntry>                sHolidaysStore;
extern DataStorage <ItemArmorQualityEntry>        sItemArmorQualityStore;
extern DataStorage <ItemArmorShieldEntry>         sItemArmorShieldStore;
extern DataStorage <ItemArmorTotalEntry>          sItemArmorTotalStore;
extern DataStorage <ItemBagFamilyEntry>           sItemBagFamilyStore;
extern DataStorage <ItemDamageEntry>              sItemDamageAmmoStore;
extern DataStorage <ItemDamageEntry>              sItemDamageOneHandStore;
extern DataStorage <ItemDamageEntry>              sItemDamageOneHandCasterStore;
extern DataStorage <ItemDamageEntry>              sItemDamageRangedStore;
extern DataStorage <ItemDamageEntry>              sItemDamageThrownStore;
extern DataStorage <ItemDamageEntry>              sItemDamageTwoHandStore;
extern DataStorage <ItemDamageEntry>              sItemDamageTwoHandCasterStore;
extern DataStorage <ItemDamageEntry>              sItemDamageWandStore;
//extern DataStorage <ItemDisplayInfoEntry>      sItemDisplayInfoStore; -- not used currently
extern DataStorage <ItemLimitCategoryEntry>       sItemLimitCategoryStore;
extern DataStorage <ItemRandomPropertiesEntry>    sItemRandomPropertiesStore;
extern DataStorage <ItemRandomSuffixEntry>        sItemRandomSuffixStore;
extern DataStorage <ItemSetEntry>                 sItemSetStore;
extern DataStorage <LFGDungeonEntry>              sLFGDungeonStore;
extern DataStorage <LockEntry>                    sLockStore;
extern DataStorage <MailTemplateEntry>            sMailTemplateStore;
extern DataStorage <MapEntry>                     sMapStore;
extern DataStorage <PhaseEntry>                   sPhaseStore;
//extern DataStorage <MapDifficultyEntry>           sMapDifficultyStore; -- use GetMapDifficultyData insteed
extern MapDifficultyMap                          sMapDifficultyMap;
extern DataStorage <MovieEntry>                   sMovieStore;
extern DataStorage <OverrideSpellDataEntry>       sOverrideSpellDataStore;
extern DataStorage <QuestSortEntry>               sQuestSortStore;
extern DataStorage <QuestXPEntry>                 sQuestXPStore;
extern DataStorage <QuestFactionRewEntry>         sQuestFactionRewardStore;
extern DataStorage <RandomPropertiesPointsEntry>  sRandomPropertiesPointsStore;
extern DataStorage <ScalingStatDistributionEntry> sScalingStatDistributionStore;
extern DataStorage <ScalingStatValuesEntry>       sScalingStatValuesStore;
extern DataStorage <SkillLineEntry>               sSkillLineStore;
extern DataStorage <SkillLineAbilityEntry>        sSkillLineAbilityStore;
extern DataStorage <SoundEntriesEntry>            sSoundEntriesStore;
extern DataStorage <SpellCastTimesEntry>          sSpellCastTimesStore;
extern DataStorage <SpellDifficultyEntry>         sSpellDifficultyStore;
extern DataStorage <SpellDurationEntry>           sSpellDurationStore;
extern DataStorage <SpellFocusObjectEntry>        sSpellFocusObjectStore;
extern DataStorage <SpellItemEnchantmentEntry>    sSpellItemEnchantmentStore;
extern DataStorage <SpellItemEnchantmentConditionEntry> sSpellItemEnchantmentConditionStore;
extern SpellCategoryStore                        sSpellCategoryStore;
extern PetFamilySpellsStore                      sPetFamilySpellsStore;
extern DataStorage <SpellRadiusEntry>             sSpellRadiusStore;
extern DataStorage <SpellRangeEntry>              sSpellRangeStore;
extern DataStorage <SpellRuneCostEntry>           sSpellRuneCostStore;
extern DataStorage <SpellEntry>                   sSpellStore;
extern DataStorage <SpellShapeshiftEntry>         sSpellShapeshiftStore;
extern DataStorage <SpellShapeshiftFormEntry>     sSpellShapeshiftFormStore;
extern DataStorage <SpellAuraOptionsEntry>        sSpellAuraOptionsStore;
extern DataStorage <SpellAuraRestrictionsEntry>   sSpellAuraRestrictionsStore;
extern DataStorage <SpellCastingRequirementsEntry> sSpellCastingRequirementsStore;
extern DataStorage <SpellCategoriesEntry>         sSpellCategoriesStore;
extern DataStorage <SpellClassOptionsEntry>       sSpellClassOptionsStore;
extern DataStorage <SpellCooldownsEntry>          sSpellCooldownsStore;
extern DataStorage <SpellEffectEntry>             sSpellEffectStore;
extern DataStorage <SpellEquippedItemsEntry>      sSpellEquippedItemsStore;
extern DataStorage <SpellInterruptsEntry>         sSpellInterruptsStore;
extern DataStorage <SpellLevelsEntry>             sSpellLevelsStore;
extern DataStorage <SpellPowerEntry>              sSpellPowerStore;
extern DataStorage <SpellReagentsEntry>           sSpellReagentsStore;
extern DataStorage <SpellScalingEntry>            sSpellScalingStore;
extern DataStorage <SpellShapeshiftEntry>         sSpellShapeshiftStore;
extern DataStorage <SpellTargetRestrictionsEntry> sSpellTargetRestrictionsStore;
extern DataStorage <SpellTotemsEntry>             sSpellTotemsStore;
//extern DataStorage <StableSlotPricesEntry>        sStableSlotPricesStore;
extern DataStorage <SummonPropertiesEntry>        sSummonPropertiesStore;
extern DataStorage <TalentEntry>                  sTalentStore;
extern DataStorage <TalentTabEntry>               sTalentTabStore;
extern DataStorage <TalentTreePrimarySpellsEntry>      sTalentTreePrimarySpells;
extern DataStorage <TaxiNodesEntry>               sTaxiNodesStore;
extern DataStorage <TaxiPathEntry>                sTaxiPathStore;
extern TaxiMask                                  sTaxiNodesMask;
extern TaxiMask                                  sOldContinentsNodesMask;
extern TaxiMask                                  sHordeTaxiNodesMask;
extern TaxiMask                                  sAllianceTaxiNodesMask;
extern TaxiMask                                  sDeathKnightTaxiNodesMask;
extern TaxiPathSetBySource                       sTaxiPathSetBySource;
extern TaxiPathNodesByPath                       sTaxiPathNodesByPath;
extern DataStorage <TotemCategoryEntry>           sTotemCategoryStore;
extern DataStorage <VehicleEntry>                 sVehicleStore;
extern DataStorage <VehicleSeatEntry>             sVehicleSeatStore;
extern DataStorage <WMOAreaTableEntry>            sWMOAreaTableStore;
//extern DataStorage <WorldMapAreaEntry>           sWorldMapAreaStore; -- use Zone2MapCoordinates and Map2ZoneCoordinates
extern DataStorage <WorldMapOverlayEntry>         sWorldMapOverlayStore;
extern DataStorage <WorldSafeLocsEntry>           sWorldSafeLocsStore;

//DB2 Storage
extern DataStorage <ItemEntry>                    sItemStore;
extern DataStorage <ItemExtendedCostEntry>        sItemExtendedCostStore;
void LoadDataStorages(const std::string& dataPath);

// script support functions
DataStorage <SoundEntriesEntry>          const* GetSoundEntriesStore();
DataStorage <SpellRangeEntry>            const* GetSpellRangeStore();
DataStorage <FactionEntry>               const* GetFactionStore();
DataStorage <ItemEntry>                  const* GetItemDisplayStore();
DataStorage <CreatureDisplayInfoEntry>   const* GetCreatureDisplayStore();
DataStorage <EmotesEntry>                const* GetEmotesStore();
DataStorage <EmotesTextEntry>            const* GetEmotesTextStore();
DataStorage <AchievementEntry>           const* GetAchievementStore();

#endif
