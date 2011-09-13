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

#include "DataStorage.h"

#include "Logging/Log.h"
#include "SharedDefines.h"
#include "SpellMgr.h"

#include "DataFormat.h"

#include <map>

typedef std::map<uint16, uint32> AreaFlagByAreaID;
typedef std::map<uint32, uint32> AreaFlagByMapID;

struct WMOAreaTableTripple
{
    WMOAreaTableTripple(int32 r, int32 a, int32 g) :  groupId(g), rootId(r), adtId(a)
    {
    }

    bool operator <(const WMOAreaTableTripple& b) const
    {
        return memcmp(this, &b, sizeof(WMOAreaTableTripple))<0;
    }

    // ordered by entropy; that way memcmp will have a minimal medium runtime
    int32 groupId;
    int32 rootId;
    int32 adtId;
};

typedef std::map<WMOAreaTableTripple, WMOAreaTableEntry const *> WMOAreaInfoByTripple;

DataStorage <AreaTableEntry> sAreaStore(AreaTableEntryfmt);
DataStorage <AreaGroupEntry> sAreaGroupStore(AreaGroupEntryfmt);
DataStorage <AreaPOIEntry> sAreaPOIStore(AreaPOIEntryfmt);
static AreaFlagByAreaID sAreaFlagByAreaID;
static AreaFlagByMapID sAreaFlagByMapID;                    // for instances without generated *.map files

static WMOAreaInfoByTripple sWMOAreaInfoByTripple;

DataStorage <AchievementEntry> sAchievementStore(Achievementfmt);
DataStorage <AchievementCriteriaEntry> sAchievementCriteriaStore(AchievementCriteriafmt);
DataStorage <AreaTriggerEntry> sAreaTriggerStore(AreaTriggerEntryfmt);
DataStorage <ArmorLocationEntry> sArmorLocationStore(ArmorLocationfmt);
DataStorage <AuctionHouseEntry> sAuctionHouseStore(AuctionHouseEntryfmt);
DataStorage <BankBagSlotPricesEntry> sBankBagSlotPricesStore(BankBagSlotPricesEntryfmt);
DataStorage <BattlemasterListEntry> sBattlemasterListStore(BattlemasterListEntryfmt);
DataStorage <BarberShopStyleEntry> sBarberShopStyleStore(BarberShopStyleEntryfmt);
DataStorage <CharStartOutfitEntry> sCharStartOutfitStore(CharStartOutfitEntryfmt);
DataStorage <CharTitlesEntry> sCharTitlesStore(CharTitlesEntryfmt);
DataStorage <ChatChannelsEntry> sChatChannelsStore(ChatChannelsEntryfmt);
DataStorage <ChrClassesEntry> sChrClassesStore(ChrClassesEntryfmt);
DataStorage <ChrRacesEntry> sChrRacesStore(ChrRacesEntryfmt);
DataStorage <ChrPowerTypesEntry> sChrPowerTypesStore(ChrClassesXPowerTypesfmt);
DataStorage <CinematicSequencesEntry> sCinematicSequencesStore(CinematicSequencesEntryfmt);
DataStorage <CreatureDisplayInfoEntry> sCreatureDisplayInfoStore(CreatureDisplayInfofmt);
DataStorage <CreatureFamilyEntry> sCreatureFamilyStore(CreatureFamilyfmt);
DataStorage <CreatureSpellDataEntry> sCreatureSpellDataStore(CreatureSpellDatafmt);
DataStorage <CreatureTypeEntry> sCreatureTypeStore(CreatureTypefmt);
DataStorage <CurrencyTypesEntry> sCurrencyTypesStore(CurrencyTypesfmt);

DataStorage <DestructibleModelDataEntry> sDestructibleModelDataStore(DestructibleModelDatafmt);
DataStorage <DungeonEncounterEntry> sDungeonEncounterStore(DungeonEncounterfmt);
DataStorage <DurabilityQualityEntry> sDurabilityQualityStore(DurabilityQualityfmt);
DataStorage <DurabilityCostsEntry> sDurabilityCostsStore(DurabilityCostsfmt);

DataStorage <EmotesEntry> sEmotesStore(EmotesEntryfmt);
DataStorage <EmotesTextEntry> sEmotesTextStore(EmotesTextEntryfmt);

typedef std::map<uint32, SimpleFactionsList> FactionTeamMap;
static FactionTeamMap sFactionTeamMap;
DataStorage <FactionEntry> sFactionStore(FactionEntryfmt);
DataStorage <FactionTemplateEntry> sFactionTemplateStore(FactionTemplateEntryfmt);

DataStorage <GameObjectDisplayInfoEntry> sGameObjectDisplayInfoStore(GameObjectDisplayInfofmt);
DataStorage <GemPropertiesEntry> sGemPropertiesStore(GemPropertiesEntryfmt);
DataStorage <GlyphPropertiesEntry> sGlyphPropertiesStore(GlyphPropertiesfmt);
DataStorage <GlyphSlotEntry> sGlyphSlotStore(GlyphSlotfmt);

DataStorage <GtBarberShopCostBaseEntry>    sGtBarberShopCostBaseStore(GtBarberShopCostBasefmt);
DataStorage <GtCombatRatingsEntry>         sGtCombatRatingsStore(GtCombatRatingsfmt);
DataStorage <GtChanceToMeleeCritBaseEntry> sGtChanceToMeleeCritBaseStore(GtChanceToMeleeCritBasefmt);
DataStorage <GtChanceToMeleeCritEntry>     sGtChanceToMeleeCritStore(GtChanceToMeleeCritfmt);
DataStorage <GtChanceToSpellCritBaseEntry> sGtChanceToSpellCritBaseStore(GtChanceToSpellCritBasefmt);
DataStorage <GtChanceToSpellCritEntry>     sGtChanceToSpellCritStore(GtChanceToSpellCritfmt);
DataStorage <GtOCTClassCombatRatingScalarEntry> sGtOCTClassCombatRatingScalarStore(GtOCTClassCombatRatingScalarfmt);
//DataStorage <GtOCTRegenMPEntry>            sGtOCTRegenMPStore(GtOCTRegenMPfmt);  -- not used currently
DataStorage <GtOCTHpPerStaminaEntry>       sGtOCTHpPerStaminaStore(gtOCTHpPerStaminafmt);
DataStorage <GtRegenMPPerSptEntry>         sGtRegenMPPerSptStore(GtRegenMPPerSptfmt);
DataStorage <GtSpellScalingEntry>          sGtSpellScalingStore(GtSpellScalingfmt);

DataStorage <HolidaysEntry>                sHolidaysStore(Holidaysfmt);

DataStorage <ItemEntry>                    sItemStore(Itemfmt);
DataStorage <ItemSparseEntry>              sItemSparseStore(ItemSparsefmt);
DataStorage <ItemArmorQualityEntry>        sItemArmorQualityStore(ItemArmorQualityfmt);
DataStorage <ItemArmorShieldEntry>         sItemArmorShieldStore(ItemArmorShieldfmt);
DataStorage <ItemArmorTotalEntry>          sItemArmorTotalStore(ItemArmorTotalfmt);
DataStorage <ItemBagFamilyEntry>           sItemBagFamilyStore(ItemBagFamilyfmt);
DataStorage <ItemDamageEntry>              sItemDamageAmmoStore(ItemDamagefmt);
DataStorage <ItemDamageEntry>              sItemDamageOneHandStore(ItemDamagefmt);
DataStorage <ItemDamageEntry>              sItemDamageOneHandCasterStore(ItemDamagefmt);
DataStorage <ItemDamageEntry>              sItemDamageRangedStore(ItemDamagefmt);
DataStorage <ItemDamageEntry>              sItemDamageThrownStore(ItemDamagefmt);
DataStorage <ItemDamageEntry>              sItemDamageTwoHandStore(ItemDamagefmt);
DataStorage <ItemDamageEntry>              sItemDamageTwoHandCasterStore(ItemDamagefmt);
DataStorage <ItemDamageEntry>              sItemDamageWandStore(ItemDamagefmt);
//DataStorage <ItemDisplayInfoEntry>         sItemDisplayInfoStore(ItemDisplayTemplateEntryfmt); -- not used currently
DataStorage <ItemExtendedCostEntry>        sItemExtendedCostStore(ItemExtendedCostEntryfmt);
DataStorage <ItemLimitCategoryEntry>       sItemLimitCategoryStore(ItemLimitCategoryEntryfmt);
DataStorage <ItemRandomPropertiesEntry>    sItemRandomPropertiesStore(ItemRandomPropertiesfmt);
DataStorage <ItemRandomSuffixEntry>        sItemRandomSuffixStore(ItemRandomSuffixfmt);
DataStorage <ItemSetEntry>                 sItemSetStore(ItemSetEntryfmt);

DataStorage <LFGDungeonEntry>              sLFGDungeonStore(LFGDungeonEntryfmt);

DataStorage <LockEntry> sLockStore(LockEntryfmt);

DataStorage <MailTemplateEntry> sMailTemplateStore(MailTemplateEntryfmt);
DataStorage <MapEntry> sMapStore(MapEntryfmt);

// DBC used only for initialization sMapDifficultyMap at startup.
DataStorage <MapDifficultyEntry> sMapDifficultyStore(MapDifficultyEntryfmt); // only for loading
MapDifficultyMap sMapDifficultyMap;

DataStorage <MovieEntry> sMovieStore(MovieEntryfmt);

DataStorage <OverrideSpellDataEntry> sOverrideSpellDataStore(OverrideSpellDatafmt);

DataStorage <PvPDifficultyEntry> sPvPDifficultyStore(PvPDifficultyfmt);

DataStorage <QuestSortEntry> sQuestSortStore(QuestSortEntryfmt);
DataStorage <QuestXPEntry>   sQuestXPStore(QuestXPfmt);
DataStorage <QuestFactionRewEntry>  sQuestFactionRewardStore(QuestFactionRewardfmt);
DataStorage <RandomPropertiesPointsEntry> sRandomPropertiesPointsStore(RandomPropertiesPointsfmt);
DataStorage <ScalingStatDistributionEntry> sScalingStatDistributionStore(ScalingStatDistributionfmt);
DataStorage <ScalingStatValuesEntry> sScalingStatValuesStore(ScalingStatValuesfmt);

DataStorage <SkillLineEntry> sSkillLineStore(SkillLinefmt);
DataStorage <SkillLineAbilityEntry> sSkillLineAbilityStore(SkillLineAbilityfmt);

DataStorage <SoundEntriesEntry> sSoundEntriesStore(SoundEntriesfmt);

DataStorage <SpellItemEnchantmentEntry> sSpellItemEnchantmentStore(SpellItemEnchantmentfmt);
DataStorage <SpellItemEnchantmentConditionEntry> sSpellItemEnchantmentConditionStore(SpellItemEnchantmentConditionfmt);
DataStorage <SpellEntry> sSpellStore(SpellEntryfmt);
SpellCategoryStore sSpellCategoryStore;
PetFamilySpellsStore sPetFamilySpellsStore;

DataStorage <SpellAuraOptionsEntry> sSpellAuraOptionsStore(SpellAuraOptionsEntryfmt);
DataStorage <SpellAuraRestrictionsEntry> sSpellAuraRestrictionsStore(SpellAuraRestrictionsEntryfmt);
DataStorage <SpellCastingRequirementsEntry> sSpellCastingRequirementsStore(SpellCastingRequirementsEntryfmt);
DataStorage <SpellCategoriesEntry> sSpellCategoriesStore(SpellCategoriesEntryfmt);
DataStorage <SpellClassOptionsEntry> sSpellClassOptionsStore(SpellClassOptionsEntryfmt);
DataStorage <SpellCooldownsEntry> sSpellCooldownsStore(SpellCooldownsEntryfmt);
DataStorage <SpellEffectEntry> sSpellEffectStore(SpellEffectEntryfmt);
DataStorage <SpellEquippedItemsEntry> sSpellEquippedItemsStore(SpellEquippedItemsEntryfmt);
DataStorage <SpellInterruptsEntry> sSpellInterruptsStore(SpellInterruptsEntryfmt);
DataStorage <SpellLevelsEntry> sSpellLevelsStore(SpellLevelsEntryfmt);
DataStorage <SpellPowerEntry> sSpellPowerStore(SpellPowerEntryfmt);
DataStorage <SpellReagentsEntry> sSpellReagentsStore(SpellReagentsEntryfmt);
DataStorage <SpellScalingEntry> sSpellScalingStore(SpellScalingEntryfmt);
DataStorage <SpellShapeshiftEntry> sSpellShapeshiftStore(SpellShapeshiftEntryfmt);
DataStorage <SpellShapeshiftFormEntry> sSpellShapeshiftFormStore(SpellShapeshiftFormfmt);
DataStorage <SpellTargetRestrictionsEntry> sSpellTargetRestrictionsStore(SpellTargetRestrictionsEntryfmt);
DataStorage <SpellTotemsEntry> sSpellTotemsStore(SpellTotemsEntryfmt);

SpellEffectMap sSpellEffectMap;

DataStorage <SpellCastTimesEntry> sSpellCastTimesStore(SpellCastTimefmt);
DataStorage <SpellDifficultyEntry> sSpellDifficultyStore(SpellDifficultyfmt);
DataStorage <SpellDurationEntry> sSpellDurationStore(SpellDurationfmt);
DataStorage <SpellFocusObjectEntry> sSpellFocusObjectStore(SpellFocusObjectfmt);
DataStorage <SpellRadiusEntry> sSpellRadiusStore(SpellRadiusfmt);
DataStorage <SpellRangeEntry> sSpellRangeStore(SpellRangefmt);
DataStorage <SpellRuneCostEntry> sSpellRuneCostStore(SpellRuneCostfmt);
//DataStorage <SpellShapeshiftEntry> sSpellShapeshiftStore(SpellShapeshiftfmt);
DataStorage <SummonPropertiesEntry> sSummonPropertiesStore(SummonPropertiesfmt);
DataStorage <TalentEntry> sTalentStore(TalentEntryfmt);
TalentSpellPosMap sTalentSpellPosMap;
DataStorage <TalentTabEntry> sTalentTabStore(TalentTabEntryfmt);
DataStorage <TalentTreePrimarySpellsEntry> sTalentTreePrimarySpells(TalentTreeSpellsfmt);

// store absolute bit position for first rank for talent inspect
static uint32 sTalentTabPages[MAX_CLASSES][3];

DataStorage <TaxiNodesEntry> sTaxiNodesStore(TaxiNodesEntryfmt);
TaxiMask sTaxiNodesMask;
TaxiMask sOldContinentsNodesMask;
TaxiMask sHordeTaxiNodesMask;
TaxiMask sAllianceTaxiNodesMask;
TaxiMask sDeathKnightTaxiNodesMask;

// DBC used only for initialization sTaxiPathSetBySource at startup.
TaxiPathSetBySource sTaxiPathSetBySource;
DataStorage <TaxiPathEntry> sTaxiPathStore(TaxiPathEntryfmt);

// DBC used only for initialization sTaxiPathNodeStore at startup.
TaxiPathNodesByPath sTaxiPathNodesByPath;
static DataStorage <TaxiPathNodeEntry> sTaxiPathNodeStore(TaxiPathNodeEntryfmt);

DataStorage <TotemCategoryEntry> sTotemCategoryStore(TotemCategoryEntryfmt);
DataStorage <VehicleEntry> sVehicleStore(VehicleEntryfmt);
DataStorage <VehicleSeatEntry> sVehicleSeatStore(VehicleSeatEntryfmt);
DataStorage <WMOAreaTableEntry> sWMOAreaTableStore(WMOAreaTableEntryfmt);
DataStorage <WorldMapAreaEntry> sWorldMapAreaStore(WorldMapAreaEntryfmt);
DataStorage <WorldMapOverlayEntry> sWorldMapOverlayStore(WorldMapOverlayEntryfmt);
DataStorage <WorldSafeLocsEntry> sWorldSafeLocsStore(WorldSafeLocsEntryfmt);
DataStorage <PhaseEntry> sPhaseStores(PhaseEntryfmt);

typedef std::list<std::string> StoreProblemList;

uint32 DataFileCount = 0;

static bool LoadData_assert_print(uint32 fsize, uint32 rsize, const std::string& filename)
{
    sLog->outError("Size of '%s' setted by format string (%u) not equal size of C++ structure (%u).", filename.c_str(), fsize, rsize);

    // ASSERT must fail after function call
    return false;
}

template<class T>
inline void LoadData(uint32& availableDbcLocales, StoreProblemList& errors, DataStorage<T>& storage, std::string const& dataPath, std::string const& filename, std::string const* customFormat = NULL, std::string const* customIndexName = NULL)
{
    // compatibility format and C++ structure sizes
    ASSERT(StorageLoader::GetFormatRecordSize(storage.GetFormat()) == sizeof(T) || LoadData_assert_print(StorageLoader::GetFormatRecordSize(storage.GetFormat()), sizeof(T), filename));

    ++DataFileCount;
    std::string dataFilename = dataPath + filename;
    SqlDbc* sql = NULL;
    if (customFormat)
        sql = new SqlDbc(&filename, customFormat, customIndexName, storage.GetFormat());

    if (storage.LoadStorage(dataFilename.c_str(), sql))
    {
        for (uint8 i = 0; i < TOTAL_LOCALES; ++i)
        {
            if (!(availableDbcLocales & (1 << i)))
                continue;

            std::string localizedName = dataPath + localeNames[i] + "/" + filename;
            if (!storage.LoadStringsFrom(localizedName.c_str()))
                availableDbcLocales &= ~(1<<i);             // mark as not available for speedup next checks
        }
    }
    else
    {
        // sort problematic dbc to (1) non compatible and (2) non-existed
        if (FILE* f = fopen(dataFilename.c_str(), "rb"))
        {
            char buf[100];
            snprintf(buf, 100, " (exists, but has %d fields instead of " SIZEFMTD ") Possible wrong client version.", storage.GetFieldCount(), strlen(storage.GetFormat()));
            errors.push_back(dataFilename + buf);
            fclose(f);
        }
        else
            errors.push_back(dataFilename);
    }

    delete sql;
}

void LoadDataStorages(const std::string& dataPath)
{
    uint32 oldMSTime = getMSTime();

    std::string storagesPath = dataPath+"storages/";

    StoreProblemList bad_dbc_files;
    uint32 availableDbcLocales = 0xFFFFFFFF;

    // DB2 Files
    LoadData(availableDbcLocales, bad_dbc_files, sItemStore,                   storagesPath, "Item.db2");
    LoadData(availableDbcLocales, bad_dbc_files, sItemSparseStore,             storagesPath, "Item-sparse.db2");
    LoadData(availableDbcLocales, bad_dbc_files, sItemExtendedCostStore,       storagesPath, "ItemExtendedCost.db2");

    for (uint32 i = 0; i < sItemStore.GetNumRows(); ++i)
    {
        const ItemEntry* itemEntry = sItemStore.LookupEntry(i);
        if (!itemEntry)
            continue;

        if (itemEntry->Class >= MAX_ITEM_CLASS)
        {
			sLog->outErrorDb("Item (Entry: %u) in Item.db2 has too high class value %u", itemEntry->ID, itemEntry->Class);
            const_cast<ItemEntry*>(itemEntry)->Class = 0;
        }
        if (itemEntry->SubClass >= MaxItemSubclassValues[itemEntry->Class])
        {
            sLog->outErrorDb("Item (Entry: %u) in Item.db2 has too high subclass value %u for class %u", itemEntry->ID, itemEntry->SubClass, itemEntry->Class);
            const_cast<ItemEntry*>(itemEntry)->SubClass = 0;
        }
    }

    //DBC Files
    LoadData(availableDbcLocales, bad_dbc_files, sAreaStore,                   storagesPath, "AreaTable.dbc");

    // must be after sAreaStore loading
    for (uint32 i = 0; i < sAreaStore.GetNumRows(); ++i)           // areaflag numbered from 0
    {
        if (AreaTableEntry const* area = sAreaStore.LookupEntry(i))
        {
            // fill AreaId->DBC records
            sAreaFlagByAreaID.insert(AreaFlagByAreaID::value_type(uint16(area->ID), area->exploreFlag));

            // fill MapId->DBC records (skip sub zones and continents)
            if (area->zone == 0 && area->mapid != 0 && area->mapid != 1 && area->mapid != 530 && area->mapid != 571)
                sAreaFlagByMapID.insert(AreaFlagByMapID::value_type(area->mapid, area->exploreFlag));
        }
    }

    LoadData(availableDbcLocales, bad_dbc_files, sAchievementStore,            storagesPath, "Achievement.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sAchievementCriteriaStore,    storagesPath, "Achievement_Criteria.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sAreaTriggerStore,            storagesPath, "AreaTrigger.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sArmorLocationStore,          storagesPath, "ArmorLocation.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sAreaGroupStore,              storagesPath, "AreaGroup.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sAreaPOIStore,                storagesPath, "AreaPOI.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sAuctionHouseStore,           storagesPath, "AuctionHouse.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sBankBagSlotPricesStore,      storagesPath, "BankBagSlotPrices.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sBattlemasterListStore,       storagesPath, "BattlemasterList.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sBarberShopStyleStore,        storagesPath, "BarberShopStyle.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sCharStartOutfitStore,        storagesPath, "CharStartOutfit.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sCharTitlesStore,             storagesPath, "CharTitles.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sChatChannelsStore,           storagesPath, "ChatChannels.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sChrClassesStore,             storagesPath, "ChrClasses.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sChrRacesStore,               storagesPath, "ChrRaces.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sChrPowerTypesStore,          storagesPath, "ChrClassesXPowerTypes.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sCinematicSequencesStore,     storagesPath, "CinematicSequences.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sCreatureDisplayInfoStore,    storagesPath, "CreatureDisplayInfo.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sCreatureFamilyStore,         storagesPath, "CreatureFamily.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sCreatureSpellDataStore,      storagesPath, "CreatureSpellData.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sCreatureTypeStore,           storagesPath, "CreatureType.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sCurrencyTypesStore,          storagesPath, "CurrencyTypes.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sDestructibleModelDataStore,  storagesPath, "DestructibleModelData.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sDungeonEncounterStore,       storagesPath, "DungeonEncounter.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sDurabilityCostsStore,        storagesPath, "DurabilityCosts.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sDurabilityQualityStore,      storagesPath, "DurabilityQuality.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sEmotesStore,                 storagesPath, "Emotes.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sEmotesTextStore,             storagesPath, "EmotesText.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sFactionStore,                storagesPath, "Faction.dbc");
    for (uint32 i=0; i<sFactionStore.GetNumRows(); ++i)
    {
        FactionEntry const * faction = sFactionStore.LookupEntry(i);
        if (faction && faction->team)
        {
            SimpleFactionsList &flist = sFactionTeamMap[faction->team];
            flist.push_back(i);
        }
    }

    LoadData(availableDbcLocales, bad_dbc_files, sFactionTemplateStore,        storagesPath, "FactionTemplate.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sGameObjectDisplayInfoStore,  storagesPath, "GameObjectDisplayInfo.dbc");
    for (uint32 i = 0; i < sGameObjectDisplayInfoStore.GetNumRows(); ++i)
    {
        if (GameObjectDisplayInfoEntry const * info = sGameObjectDisplayInfoStore.LookupEntry(i))
        {
            if (info->maxX < info->minX)
                std::swap(*(float*)(&info->maxX), *(float*)(&info->minX));
            if (info->maxY < info->minY)
                std::swap(*(float*)(&info->maxY), *(float*)(&info->minY));
            if (info->maxZ < info->minZ)
                std::swap(*(float*)(&info->maxZ), *(float*)(&info->minZ));
        }
    }

    LoadData(availableDbcLocales, bad_dbc_files, sGemPropertiesStore,          storagesPath, "GemProperties.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sGlyphPropertiesStore,        storagesPath, "GlyphProperties.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sGlyphSlotStore,              storagesPath, "GlyphSlot.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sGtBarberShopCostBaseStore,   storagesPath, "gtBarberShopCostBase.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sGtCombatRatingsStore,        storagesPath, "gtCombatRatings.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sGtChanceToMeleeCritBaseStore, storagesPath, "gtChanceToMeleeCritBase.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sGtChanceToMeleeCritStore,    storagesPath, "gtChanceToMeleeCrit.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sGtChanceToSpellCritBaseStore, storagesPath, "gtChanceToSpellCritBase.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sGtChanceToSpellCritStore,    storagesPath, "gtChanceToSpellCrit.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sGtOCTHpPerStaminaStore,      storagesPath, "gtOCTHpPerStamina.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sGtRegenMPPerSptStore,        storagesPath, "gtRegenMPPerSpt.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sGtSpellScalingStore,         storagesPath, "gtSpellScaling.dbc");

    LoadData(availableDbcLocales, bad_dbc_files, sHolidaysStore,               storagesPath, "Holidays.dbc");

    LoadData(availableDbcLocales, bad_dbc_files, sItemBagFamilyStore,          storagesPath, "ItemBagFamily.dbc");
    //LoadData(dbcCount, availableDbcLocales, bad_dbc_files, sItemDisplayInfoStore,        storagesPath, "ItemDisplayInfo.dbc");     -- not used currently
    //LoadData(dbcCount, availableDbcLocales, bad_dbc_files, sItemCondExtCostsStore,       storagesPath, "ItemCondExtCosts.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sItemLimitCategoryStore,      storagesPath, "ItemLimitCategory.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sItemRandomPropertiesStore,   storagesPath, "ItemRandomProperties.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sItemRandomSuffixStore,       storagesPath, "ItemRandomSuffix.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sItemSetStore,                storagesPath, "ItemSet.dbc");

    /*########################################################################################################################
    ###################################           Weapon and Armor            ################################################
    ########################################################################################################################*/

    LoadData(availableDbcLocales, bad_dbc_files,sItemArmorQualityStore,           storagesPath,"ItemArmorQuality.dbc");
    LoadData(availableDbcLocales, bad_dbc_files,sItemArmorShieldStore,            storagesPath,"ItemArmorShield.dbc");
    LoadData(availableDbcLocales, bad_dbc_files,sItemArmorTotalStore,             storagesPath,"ItemArmorTotal.dbc");
    LoadData(availableDbcLocales, bad_dbc_files,sItemDamageAmmoStore,             storagesPath,"ItemDamageAmmo.dbc");
    LoadData(availableDbcLocales, bad_dbc_files,sItemDamageOneHandStore,          storagesPath,"ItemDamageOneHand.dbc");
    LoadData(availableDbcLocales, bad_dbc_files,sItemDamageOneHandCasterStore,    storagesPath,"ItemDamageOneHandCaster.dbc");
    LoadData(availableDbcLocales, bad_dbc_files,sItemDamageRangedStore,           storagesPath,"ItemDamageRanged.dbc");
    LoadData(availableDbcLocales, bad_dbc_files,sItemDamageThrownStore,           storagesPath,"ItemDamageThrown.dbc");
    LoadData(availableDbcLocales, bad_dbc_files,sItemDamageTwoHandStore,          storagesPath,"ItemDamageTwoHand.dbc");
    LoadData(availableDbcLocales, bad_dbc_files,sItemDamageTwoHandCasterStore,    storagesPath,"ItemDamageTwoHandCaster.dbc");
    LoadData(availableDbcLocales, bad_dbc_files,sItemDamageWandStore,             storagesPath,"ItemDamageWand.dbc");


    LoadData(availableDbcLocales, bad_dbc_files, sLFGDungeonStore,             storagesPath, "LFGDungeons.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sLockStore,                   storagesPath, "Lock.dbc");

    LoadData(availableDbcLocales, bad_dbc_files, sPhaseStores,                 storagesPath, "Phase.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sMailTemplateStore,           storagesPath, "MailTemplate.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sMapStore,                    storagesPath, "Map.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sMapDifficultyStore,          storagesPath, "MapDifficulty.dbc");
    // fill data
    for (uint32 i = 1; i < sMapDifficultyStore.GetNumRows(); ++i)
        if (MapDifficultyEntry const* entry = sMapDifficultyStore.LookupEntry(i))
            sMapDifficultyMap[MAKE_PAIR32(entry->MapId, entry->Difficulty)] = MapDifficulty(entry->resetTime, entry->maxPlayers, entry->areaTriggerText > 0);

    sMapDifficultyMap[MAKE_PAIR32(0, 0)] = MapDifficulty(0, 0, 0 > 0);

    sMapDifficultyStore.Clear();

    LoadData(availableDbcLocales, bad_dbc_files, sMovieStore,                  storagesPath, "Movie.dbc");

    LoadData(availableDbcLocales, bad_dbc_files, sOverrideSpellDataStore,      storagesPath, "OverrideSpellData.dbc");

    LoadData(availableDbcLocales, bad_dbc_files, sPvPDifficultyStore,          storagesPath, "PvpDifficulty.dbc");
    for (uint32 i = 0; i < sPvPDifficultyStore.GetNumRows(); ++i)
        if (PvPDifficultyEntry const* entry = sPvPDifficultyStore.LookupEntry(i))
            if (entry->bracketId > MAX_BATTLEGROUND_BRACKETS)
                ASSERT(false && "Need update MAX_BATTLEGROUND_BRACKETS by DBC data");

    LoadData(availableDbcLocales, bad_dbc_files, sQuestXPStore,                storagesPath, "QuestXP.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sQuestFactionRewardStore,     storagesPath, "QuestFactionReward.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sQuestSortStore,              storagesPath, "QuestSort.dbc");

    LoadData(availableDbcLocales, bad_dbc_files, sRandomPropertiesPointsStore, storagesPath, "RandPropPoints.dbc");

    LoadData(availableDbcLocales, bad_dbc_files, sScalingStatDistributionStore, storagesPath, "ScalingStatDistribution.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sScalingStatValuesStore,      storagesPath, "ScalingStatValues.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sSkillLineStore,              storagesPath, "SkillLine.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sSkillLineAbilityStore,       storagesPath, "SkillLineAbility.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sSoundEntriesStore,           storagesPath, "SoundEntries.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sSpellStore,                  storagesPath, "Spell.dbc", &CustomSpellEntryfmt, &CustomSpellEntryIndex);
    for (uint32 i = 1; i < sSpellStore.GetNumRows(); ++i)
    {
        SpellCategoriesEntry const* spell = sSpellCategoriesStore.LookupEntry(i);
        if (spell && spell->Category)
            sSpellCategoryStore[spell->Category].insert(i);
    }

    for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
    {
        SkillLineAbilityEntry const *skillLine = sSkillLineAbilityStore.LookupEntry(j);

        if (!skillLine)
            continue;

        SpellEntry const* spellInfo = sSpellStore.LookupEntry(skillLine->spellId);

        if (spellInfo && spellInfo->Attributes & SPELL_ATTR0_PASSIVE)
        {
            for (uint32 i = 1; i < sCreatureFamilyStore.GetNumRows(); ++i)
            {
                SpellLevelsEntry const* levels = sSpellLevelsStore.LookupEntry(i);
                if (!levels)
                    continue;

                CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(i);
                if (!cFamily)
                    continue;

                if (skillLine->skillId != cFamily->skillLine[0] && skillLine->skillId != cFamily->skillLine[1])
                    continue;
                if (levels->spellLevel)
                    continue;

                if (skillLine->learnOnGetSkill != ABILITY_LEARNED_ON_GET_RACE_OR_CLASS_SKILL)
                    continue;

                sPetFamilySpellsStore[i].insert(spellInfo->Id);
            }
        }
    }

    LoadData(availableDbcLocales, bad_dbc_files, sSpellAuraOptionsStore,    storagesPath,"SpellAuraOptions.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sSpellAuraRestrictionsStore, storagesPath,"SpellAuraRestrictions.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sSpellCastingRequirementsStore, storagesPath,"SpellCastingRequirements.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sSpellCategoriesStore,     storagesPath,"SpellCategories.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sSpellClassOptionsStore,   storagesPath,"SpellClassOptions.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sSpellCooldownsStore,      storagesPath,"SpellCooldowns.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sSpellEffectStore,         storagesPath,"SpellEffect.dbc");

    for (uint32 i = 1; i < sSpellEffectStore.GetNumRows(); ++i)
    {
        if (SpellEffectEntry const *spellEffect = sSpellEffectStore.LookupEntry(i))
            sSpellEffectMap[spellEffect->EffectSpellId].effects[spellEffect->EffectIndex] = spellEffect;
    }
 
    LoadData(availableDbcLocales, bad_dbc_files, sSpellEquippedItemsStore,  storagesPath,"SpellEquippedItems.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sSpellInterruptsStore,     storagesPath,"SpellInterrupts.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sSpellLevelsStore,         storagesPath,"SpellLevels.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sSpellPowerStore,          storagesPath,"SpellPower.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sSpellReagentsStore,       storagesPath,"SpellReagents.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sSpellScalingStore,        storagesPath,"SpellScaling.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sSpellShapeshiftStore,     storagesPath,"SpellShapeshift.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sSpellTargetRestrictionsStore, storagesPath,"SpellTargetRestrictions.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sSpellTotemsStore,         storagesPath,"SpellTotems.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sSpellCastTimesStore,         storagesPath, "SpellCastTimes.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sSpellDifficultyStore,        storagesPath, "SpellDifficulty.dbc", &CustomSpellDifficultyfmt, &CustomSpellDifficultyIndex);
    LoadData(availableDbcLocales, bad_dbc_files, sSpellDurationStore,          storagesPath, "SpellDuration.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sSpellFocusObjectStore,       storagesPath, "SpellFocusObject.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sSpellItemEnchantmentStore,   storagesPath, "SpellItemEnchantment.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sSpellItemEnchantmentConditionStore, storagesPath, "SpellItemEnchantmentCondition.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sSpellRadiusStore,            storagesPath, "SpellRadius.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sSpellRangeStore,             storagesPath, "SpellRange.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sSpellRuneCostStore,          storagesPath, "SpellRuneCost.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sSpellShapeshiftStore,        storagesPath, "SpellShapeshiftForm.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sSummonPropertiesStore,       storagesPath, "SummonProperties.dbc");

    // Create Spelldifficulty searcher
    for (uint32 i = 0; i < sSpellDifficultyStore.GetNumRows(); ++i)
    {
        SpellDifficultyEntry const *spellDiff = sSpellDifficultyStore.LookupEntry(i);
        if (!spellDiff)
            continue;

        SpellDifficultyEntry newEntry;
        memset(newEntry.SpellID, 0, 4*sizeof(uint32));
        for (int x = 0; x < MAX_DIFFICULTY; ++x)
        {
            if (spellDiff->SpellID[x] <= 0 || !sSpellStore.LookupEntry(spellDiff->SpellID[x]))
            {
                if (spellDiff->SpellID[x] > 0)//don't show error if spell is <= 0, not all modes have spells and there are unknown negative values
                    sLog->outErrorDb("spelldifficulty_dbc: spell %i at field id:%u at spellid%i does not exist in SpellStore (spell.dbc), loaded as 0", spellDiff->SpellID[x], spellDiff->ID, x);
                newEntry.SpellID[x] = 0;//spell was <= 0 or invalid, set to 0
            }
            else
                newEntry.SpellID[x] = spellDiff->SpellID[x];
        }
        if (newEntry.SpellID[0] <= 0 || newEntry.SpellID[1] <= 0)//id0-1 must be always set!
            continue;

        for (int x = 0; x < MAX_DIFFICULTY; ++x)
            sSpellMgr->SetSpellDifficultyId(uint32(newEntry.SpellID[x]), spellDiff->ID);
    }

    // create talent spells set
    for (unsigned int i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const *talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo) continue;
        for (int j = 0; j < MAX_TALENT_RANK; j++)
            if (talentInfo->RankID[j])
                sTalentSpellPosMap[talentInfo->RankID[j]] = TalentSpellPos(i, j);
    }

    LoadData(availableDbcLocales, bad_dbc_files, sTalentStore,                 storagesPath, "Talent.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sTalentTabStore,              storagesPath, "TalentTab.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sTalentTreePrimarySpells,     storagesPath, "TalentTreePrimarySpells.dbc");

    // prepare fast data access to bit pos of talent ranks for use at inspecting
    {
        // now have all max ranks (and then bit amount used for store talent ranks in inspect)
        for (uint32 talentTabId = 1; talentTabId < sTalentTabStore.GetNumRows(); ++talentTabId)
        {
            TalentTabEntry const *talentTabInfo = sTalentTabStore.LookupEntry(talentTabId);
            if (!talentTabInfo)
                continue;

            // prevent memory corruption; otherwise cls will become 12 below
            if ((talentTabInfo->ClassMask & CLASSMASK_ALL_PLAYABLE) == 0)
                continue;

            // store class talent tab pages
            uint32 cls = 1;
            for (uint32 m=1; !(m & talentTabInfo->ClassMask) && cls < MAX_CLASSES; m <<= 1, ++cls) {}

            sTalentTabPages[cls][talentTabInfo->tabpage] = talentTabId;
        }
    }

    LoadData(availableDbcLocales, bad_dbc_files, sTaxiNodesStore,              storagesPath, "TaxiNodes.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sTaxiPathStore,               storagesPath, "TaxiPath.dbc");
    for (uint32 i = 1; i < sTaxiPathStore.GetNumRows(); ++i)
        if (TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(i))
            sTaxiPathSetBySource[entry->from][entry->to] = TaxiPathBySourceAndDestination(entry->ID, entry->price);
    uint32 pathCount = sTaxiPathStore.GetNumRows();

    //## TaxiPathNode.dbc ## Loaded only for initialization different structures
    LoadData(availableDbcLocales, bad_dbc_files, sTaxiPathNodeStore,           storagesPath, "TaxiPathNode.dbc");
    // Calculate path nodes count
    std::vector<uint32> pathLength;
    pathLength.resize(pathCount);                           // 0 and some other indexes not used
    for (uint32 i = 1; i < sTaxiPathNodeStore.GetNumRows(); ++i)
        if (TaxiPathNodeEntry const* entry = sTaxiPathNodeStore.LookupEntry(i))
        {
            if (pathLength[entry->path] < entry->index + 1)
                pathLength[entry->path] = entry->index + 1;
        }
    // Set path length
    sTaxiPathNodesByPath.resize(pathCount);                 // 0 and some other indexes not used
    for (uint32 i = 1; i < sTaxiPathNodesByPath.size(); ++i)
        sTaxiPathNodesByPath[i].resize(pathLength[i]);
    // fill data
    for (uint32 i = 1; i < sTaxiPathNodeStore.GetNumRows(); ++i)
        if (TaxiPathNodeEntry const* entry = sTaxiPathNodeStore.LookupEntry(i))
            sTaxiPathNodesByPath[entry->path].set(entry->index, entry);

    // Initialize global taxinodes mask
    // include existed nodes that have at least single not spell base (scripted) path
    {
        std::set<uint32> spellPaths;
        for (uint32 i = 1; i < sSpellStore.GetNumRows (); ++i)
            if (SpellEffectEntry const* sInfo = sSpellEffectStore.LookupEntry(i))
                for (int j = 0; j < MAX_SPELL_EFFECTS; ++j)
                    if (sInfo->Effect == SPELL_EFFECT_SEND_TAXI)
                        spellPaths.insert(sInfo->EffectMiscValue);

        memset(sTaxiNodesMask, 0, sizeof(sTaxiNodesMask));
        memset(sOldContinentsNodesMask, 0, sizeof(sOldContinentsNodesMask));
        memset(sHordeTaxiNodesMask, 0, sizeof(sHordeTaxiNodesMask));
        memset(sAllianceTaxiNodesMask, 0, sizeof(sAllianceTaxiNodesMask));
        memset(sDeathKnightTaxiNodesMask, 0, sizeof(sDeathKnightTaxiNodesMask));
        for (uint32 i = 1; i < sTaxiNodesStore.GetNumRows(); ++i)
        {
            TaxiNodesEntry const* node = sTaxiNodesStore.LookupEntry(i);
            if (!node)
                continue;

            TaxiPathSetBySource::const_iterator src_i = sTaxiPathSetBySource.find(i);
            if (src_i != sTaxiPathSetBySource.end() && !src_i->second.empty())
            {
                bool ok = false;
                for (TaxiPathSetForSource::const_iterator dest_i = src_i->second.begin(); dest_i != src_i->second.end(); ++dest_i)
                {
                    // not spell path
                    if (spellPaths.find(dest_i->second.ID) == spellPaths.end())
                    {
                        ok = true;
                        break;
                    }
                }

                if (!ok)
                    continue;
            }

            // valid taxi network node
            uint8  field   = (uint8)((i - 1) / 32);
            uint32 submask = 1<<((i-1)%32);
            sTaxiNodesMask[field] |= submask;

            if (node->MountCreatureID[0] && node->MountCreatureID[0] != 32981)
                sHordeTaxiNodesMask[field] |= submask;
            if (node->MountCreatureID[1] && node->MountCreatureID[1] != 32981)
                sAllianceTaxiNodesMask[field] |= submask;
            if (node->MountCreatureID[0] == 32981 || node->MountCreatureID[1] == 32981)
                sDeathKnightTaxiNodesMask[field] |= submask;

            // old continent node (+ nodes virtually at old continents, check explicitly to avoid loading map files for zone info)
            if (node->map_id < 2 || i == 82 || i == 83 || i == 93 || i == 94)
                sOldContinentsNodesMask[field] |= submask;

            // fix DK node at Ebon Hold
            if (i == 315) {
                ((TaxiNodesEntry*)node)->MountCreatureID[1] = 32981;
            }
        }
    }

    LoadData(availableDbcLocales, bad_dbc_files, sTotemCategoryStore,          storagesPath, "TotemCategory.dbc");

    LoadData(availableDbcLocales, bad_dbc_files, sVehicleStore,                storagesPath, "Vehicle.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sVehicleSeatStore,            storagesPath, "VehicleSeat.dbc");

    LoadData(availableDbcLocales, bad_dbc_files, sWMOAreaTableStore,           storagesPath, "WMOAreaTable.dbc");
    for(uint32 i = 0; i < sWMOAreaTableStore.GetNumRows(); ++i)
        if(WMOAreaTableEntry const* entry = sWMOAreaTableStore.LookupEntry(i))
            sWMOAreaInfoByTripple.insert(WMOAreaInfoByTripple::value_type(WMOAreaTableTripple(entry->rootId, entry->adtId, entry->groupId), entry));
    LoadData(availableDbcLocales, bad_dbc_files, sWorldMapAreaStore,           storagesPath, "WorldMapArea.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sWorldMapOverlayStore,        storagesPath, "WorldMapOverlay.dbc");
    LoadData(availableDbcLocales, bad_dbc_files, sWorldSafeLocsStore,          storagesPath, "WorldSafeLocs.dbc");

    // error checks
    if (bad_dbc_files.size() >= DataFileCount)
    {
        sLog->outError("Incorrect DataDir value in worldserver.conf or ALL required *.dbc and *.db2 files (%d) not found by path: %sstorages", DataFileCount, dataPath.c_str());
        exit(1);
    }
    else if (!bad_dbc_files.empty())
    {
        std::string str;
        for (StoreProblemList::iterator i = bad_dbc_files.begin(); i != bad_dbc_files.end(); ++i)
            str += *i + "\n";

        sLog->outError("Some required *.dbc and *.db2 files (%u from %d) not found or not compatible:\n%s", (uint32)bad_dbc_files.size(), DataFileCount, str.c_str());
        exit(1);
    }

    // Check loaded DB2 files proper version
    if (!sItemStore.LookupEntry(72068)             ||       // last item added in 4.2.0 (14333)
        !sItemExtendedCostStore.LookupEntry(3652)  )        // last item extended cost added in 4.2.0 (14333)
    {
        sLog->outError("You have _outdated_ DB2 files. Please extract correct versions from current using client.");
        exit(1);
    }
    // Check loaded DBC files proper version
    if (!sAreaStore.LookupEntry(4559)              ||       // last area (areaflag) added in 4.2.0 (14333)
        !sCharTitlesStore.LookupEntry(279)         ||       // last char title added in 4.2.0 (14333)
        !sGemPropertiesStore.LookupEntry(1860)     ||       // last added spell in 4.2.0 (14333)
        !sMapStore.LookupEntry(968)                ||       // last map added in 4.2.0 (14333)
        !sSpellStore.LookupEntry(102129)           )        // last client known item added in 4.2.0 (14333)
    {
        sLog->outError("You have _outdated_ DBC files. Please extract correct versions from current using client.");
        exit(1);
    }

    sLog->outString(">> Initialized %d data stores in %u ms", DataFileCount, GetMSTimeDiffToNow(oldMSTime));
    sLog->outString();
}

SimpleFactionsList const* GetFactionTeamList(uint32 faction)
{
    FactionTeamMap::const_iterator itr = sFactionTeamMap.find(faction);
    if (itr != sFactionTeamMap.end())
        return &itr->second;

    return NULL;
}

char const* GetPetName(uint32 petfamily, uint32 dbclang)
{
    if (!petfamily)
        return NULL;
    CreatureFamilyEntry const* pet_family = sCreatureFamilyStore.LookupEntry(petfamily);
    if (!pet_family)
        return NULL;
    return pet_family->Name[dbclang] ? pet_family->Name[dbclang] : NULL;
}

TalentSpellPos const* GetTalentSpellPos(uint32 spellId)
{
    TalentSpellPosMap::const_iterator itr = sTalentSpellPosMap.find(spellId);
    if (itr == sTalentSpellPosMap.end())
        return NULL;

    return &itr->second;
}

SpellEffectEntry const* GetSpellEffectEntry(uint32 spellId, uint32 effect)
{
    SpellEffectMap::const_iterator itr = sSpellEffectMap.find(spellId);
    if (itr == sSpellEffectMap.end())
        return NULL;

    return itr->second.effects[effect];
}

uint32 GetTalentSpellCost(uint32 spellId)
{
    if (TalentSpellPos const* pos = GetTalentSpellPos(spellId))
        return pos->rank+1;

    return 0;
}

int32 GetAreaFlagByAreaID(uint32 area_id)
{
    AreaFlagByAreaID::iterator i = sAreaFlagByAreaID.find(area_id);
    if (i == sAreaFlagByAreaID.end())
        return -1;

    return i->second;
}

WMOAreaTableEntry const* GetWMOAreaTableEntryByTripple(int32 rootid, int32 adtid, int32 groupid)
{
    WMOAreaInfoByTripple::iterator i = sWMOAreaInfoByTripple.find(WMOAreaTableTripple(rootid, adtid, groupid));
        if(i == sWMOAreaInfoByTripple.end())
            return NULL;
        return i->second;
}

MapEntry const* GetMapEntry(uint32 map_id)
{
    if (MapEntry const* mapEntry = sMapStore.LookupEntry(map_id))
        return mapEntry;
    
    return NULL;
}

AreaTableEntry const* GetAreaEntryByAreaID(uint32 area_id)
{
    int32 areaflag = GetAreaFlagByAreaID(area_id);
    if (areaflag < 0)
        return NULL;

    return sAreaStore.LookupEntry(areaflag);
}

AreaTableEntry const* GetAreaEntryByAreaFlagAndMap(uint32 area_flag, uint32 map_id)
{
    if (area_flag)
        return sAreaStore.LookupEntry(area_flag);

    if (MapEntry const* mapEntry = sMapStore.LookupEntry(map_id))
        return GetAreaEntryByAreaID(mapEntry->linked_zone);

    return NULL;
}

uint32 GetAreaFlagByMapId(uint32 mapid)
{
    AreaFlagByMapID::iterator i = sAreaFlagByMapID.find(mapid);
    if (i == sAreaFlagByMapID.end())
        return 0;
    else
        return i->second;
}

uint32 GetVirtualMapForMapAndZone(uint32 mapid, uint32 zoneId)
{
    if (mapid != 530 && mapid != 571)                        // speed for most cases
        return mapid;

    if (WorldMapAreaEntry const* wma = sWorldMapAreaStore.LookupEntry(zoneId))
        return wma->virtual_map_id >= 0 ? wma->virtual_map_id : wma->map_id;

    return mapid;
}

ContentLevels GetContentLevelsForMapAndZone(uint32 mapid, uint32 zoneId)
{
    mapid = GetVirtualMapForMapAndZone(mapid, zoneId);
    if (mapid < 2 || mapid == 648 || mapid == 654 || mapid == 638 || mapid == 655 || mapid == 656 || mapid == 661 || mapid == 659)
        return CONTENT_1_60;

    MapEntry const* mapEntry = sMapStore.LookupEntry(mapid);
    if (!mapEntry)
        return CONTENT_1_60;

    if (zoneId == 5034 || zoneId == 4922 || zoneId == 616 || zoneId == 5146 || zoneId == 5042)
        return CONTENT_81_85;

    switch(mapEntry->Expansion())
    {
        default: return CONTENT_1_60;
        case 1:  return CONTENT_61_70;
        case 2:  return CONTENT_71_80;
        case 3:  return CONTENT_81_85;
    }
}

bool IsTotemCategoryCompatiableWith(uint32 itemTotemCategoryId, uint32 requiredTotemCategoryId)
{
    if (requiredTotemCategoryId == 0)
        return true;
    if (itemTotemCategoryId == 0)
        return false;

    TotemCategoryEntry const* itemEntry = sTotemCategoryStore.LookupEntry(itemTotemCategoryId);
    if (!itemEntry)
        return false;
    TotemCategoryEntry const* reqEntry = sTotemCategoryStore.LookupEntry(requiredTotemCategoryId);
    if (!reqEntry)
        return false;

    if (itemEntry->categoryType != reqEntry->categoryType)
        return false;

    return (itemEntry->categoryMask & reqEntry->categoryMask) == reqEntry->categoryMask;
}

void Zone2MapCoordinates(float& x, float& y, uint32 zone)
{
    WorldMapAreaEntry const* maEntry = sWorldMapAreaStore.LookupEntry(zone);

    // if not listed then map coordinates (instance)
    if (!maEntry)
        return;

    std::swap(x, y);                                         // at client map coords swapped
    x = x*((maEntry->x2-maEntry->x1)/100)+maEntry->x1;
    y = y*((maEntry->y2-maEntry->y1)/100)+maEntry->y1;      // client y coord from top to down
}

void Map2ZoneCoordinates(float& x, float& y, uint32 zone)
{
    WorldMapAreaEntry const* maEntry = sWorldMapAreaStore.LookupEntry(zone);

    // if not listed then map coordinates (instance)
    if (!maEntry)
        return;

    x = (x-maEntry->x1)/((maEntry->x2-maEntry->x1)/100);
    y = (y-maEntry->y1)/((maEntry->y2-maEntry->y1)/100);    // client y coord from top to down
    std::swap(x, y);                                         // client have map coords swapped
}

MapDifficulty const* GetMapDifficultyData(uint32 mapId, Difficulty difficulty)
{
    MapDifficultyMap::const_iterator itr = sMapDifficultyMap.find(MAKE_PAIR32(mapId, difficulty));
    return itr != sMapDifficultyMap.end() ? &itr->second : NULL;
}

MapDifficulty const* GetDownscaledMapDifficultyData(uint32 mapId, Difficulty &difficulty)
{
    uint32 tmpDiff = difficulty;
    MapDifficulty const* mapDiff = GetMapDifficultyData(mapId, Difficulty(tmpDiff));
    if (!mapDiff)
    {
        if (tmpDiff > RAID_DIFFICULTY_25MAN_NORMAL) // heroic, downscale to normal
            tmpDiff -= 2;
        else
            tmpDiff -= 1;   // any non-normal mode for raids like tbc (only one mode)

        // pull new data
        mapDiff = GetMapDifficultyData(mapId, Difficulty(tmpDiff)); // we are 10 normal or 25 normal
        if (!mapDiff)
        {
            tmpDiff -= 1;
            mapDiff = GetMapDifficultyData(mapId, Difficulty(tmpDiff)); // 10 normal
        }
    }

    difficulty = Difficulty(tmpDiff);
    return mapDiff;
}

PvPDifficultyEntry const* GetBattlegroundBracketByLevel(uint32 mapid, uint32 level)
{
    PvPDifficultyEntry const* maxEntry = NULL;              // used for level > max listed level case
    for (uint32 i = 0; i < sPvPDifficultyStore.GetNumRows(); ++i)
    {
        if (PvPDifficultyEntry const* entry = sPvPDifficultyStore.LookupEntry(i))
        {
            // skip unrelated and too-high brackets
            if (entry->mapId != mapid || entry->minLevel > level)
                continue;

            // exactly fit
            if (entry->maxLevel >= level)
                return entry;

            // remember for possible out-of-range case (search higher from existed)
            if (!maxEntry || maxEntry->maxLevel < entry->maxLevel)
                maxEntry = entry;
        }
    }

    return maxEntry;
}

PvPDifficultyEntry const* GetBattlegroundBracketById(uint32 mapid, BattlegroundBracketId id)
{
    for (uint32 i = 0; i < sPvPDifficultyStore.GetNumRows(); ++i)
        if (PvPDifficultyEntry const* entry = sPvPDifficultyStore.LookupEntry(i))
            if (entry->mapId == mapid && entry->GetBracketId() == id)
                return entry;

    return NULL;
}

uint32 const* GetTalentTabPages(uint8 cls)
{
    return sTalentTabPages[cls];
}

// script support functions
 DataStorage <SoundEntriesEntry>  const* GetSoundEntriesStore()   { return &sSoundEntriesStore;   }
 DataStorage <SpellRangeEntry>    const* GetSpellRangeStore()     { return &sSpellRangeStore;     }
 DataStorage <FactionEntry>       const* GetFactionStore()        { return &sFactionStore;        }
 DataStorage <CreatureDisplayInfoEntry> const* GetCreatureDisplayStore() { return &sCreatureDisplayInfoStore; }
 DataStorage <EmotesEntry>        const* GetEmotesStore()         { return &sEmotesStore;         }
 DataStorage <EmotesTextEntry>    const* GetEmotesTextStore()     { return &sEmotesTextStore;     }
 DataStorage <AchievementEntry>   const* GetAchievementStore()    { return &sAchievementStore;    }
