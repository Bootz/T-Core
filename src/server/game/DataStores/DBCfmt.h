/*
 * Copyright (C) 2011      TrilliumEMU <http://www.trilliumemu.com/>
 * Copyright (C) 2008-2011 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2011 MaNGOS      <http://getmangos.com/>
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

#ifndef DBCSFRM_H
#define DBCSFRM_H

const char Achievementfmt[]="niixsxiixixxii";
//const std::string CustomAchievementfmt="pppaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaapapaaaaaaaaaaaaaaaaaapp";
//const std::string CustomAchievementIndex = "ID";
const char AchievementCriteriafmt[]="niiiiiiiisiiiiixxxxxxxx";
const char AreaTableEntryfmt[]="iiinixxxxxisixxxxxxxxxxxx";
const char AreaGroupEntryfmt[]="niiiiiii";
const char AreaPOIEntryfmt[]="niiiiiiiiiiiffixixxixx";
const char AreaTriggerEntryfmt[]="nifffxxxfffff";
const char ArmorLocationfmt[]="nfffff";
const char AuctionHouseEntryfmt[]="niiix";
const char BankBagSlotPricesEntryfmt[]="ni";
const char BarberShopStyleEntryfmt[]="nixxxiii";
const char BattlemasterListEntryfmt[]="niiiiiiiiixsiiiiiiix";
const char CharStartOutfitEntryfmt[]="diiiiiiiiiiiiiiiiiiiiiiiiixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
const char CharTitlesEntryfmt[]="nxsxix";
const char ChatChannelsEntryfmt[]="nixsx";
                                                            // ChatChannelsEntryfmt, index not used (more compact store)
const char ChrClassesEntryfmt[]="nixsxxxixiixxx";
const char ChrRacesEntryfmt[]="nxixiixixxxxixsxxxxxixxx";
const char CinematicSequencesEntryfmt[]="nxxxxxxxxx";
const char CreatureDisplayInfofmt[]="nxxxfxxxxxxxxxxxx";
const char CreatureFamilyfmt[]="nfifiiiiixsx";
const char CreatureSpellDatafmt[]="niiiixxxx";
const char CreatureTypefmt[]="nxx";
const char CurrencyTypesfmt[]="nxxxxxxiixx";
const char DestructibleModelDatafmt[]="nxxixxxixxxixxxixxx";
const char DungeonEncounterfmt[]="niixisxx";
const char DurabilityCostsfmt[]="niiiiiiiiiiiiiiiiiiiiiiiiiiiii";
const char DurabilityQualityfmt[]="nf";
const char EmotesEntryfmt[]="nxxiiix";
const char EmotesTextEntryfmt[]="nxixxxxxxxxxxxxxxxx";
const char FactionEntryfmt[]="niiiiiiiiiiiiiiiiiiffixsxx";
const char FactionTemplateEntryfmt[]="niiiiiiiiiiiii";
const char GameObjectDisplayInfofmt[]="nxxxxxxxxxxxffffffxxx";
const char GemPropertiesEntryfmt[]="nixxix";
const char GlyphPropertiesfmt[]="niii";
const char GlyphSlotfmt[]="nii";
const char GtBarberShopCostBasefmt[]="xf";
const char GtCombatRatingsfmt[]="xf";
const char GtChanceToMeleeCritBasefmt[]="xf";
const char GtChanceToMeleeCritfmt[]="xf";
const char GtChanceToSpellCritBasefmt[]="xf";
const char GtChanceToSpellCritfmt[]="xf";
const char GtOCTClassCombatRatingScalarfmt[]="df";
const char GtOCTRegenHPfmt[]="xf";
//const char GtOCTRegenMPfmt[]="f";
const char GtRegenHPPerSptfmt[]="xf";
const char GtRegenMPPerSptfmt[]="xf";
const char Holidaysfmt[]="nxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
const char Itemfmt[]="niiiiiii";
const char ItemBagFamilyfmt[]="nxxxxxxxxxxxxxxxxx";
//const char ItemDisplayTemplateEntryfmt[]="nxxxxxxxxxxixxxxxxxxxxx";
//const char ItemCondExtCostsEntryfmt[]="xiii";
const char ItemExtendedCostEntryfmt[]="niiiiiiiiiiiiiix";
const char ItemLimitCategoryEntryfmt[]="nxxxxxxxxxxxxxxxxxii";
const char ItemRandomPropertiesfmt[]="nxiiixxssssssssssssssssx";
const char ItemRandomSuffixfmt[]="nssssssssssssssssxxiiixxiiixx";
const char ItemSetEntryfmt[]="dssssssssssssssssxiiiiiiiiiixxxxxxxiiiiiiiiiiiiiiiiii";
const char LFGDungeonEntryfmt[]="nxxxxxxxxxxxxxxxxxiiiiiiixixxixixxxxxxxxxxxxxxxxx";
const char LockEntryfmt[]="niiiiiiiiiiiiiiiiiiiiiiiixxxxxxxx";
const char MailTemplateEntryfmt[]="nxxxxxxxxxxxxxxxxxssssssssssssssssx";
const char MapEntryfmt[]="nxixxssssssssssssssssxixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxixiffxiix";
const char MapDifficultyEntryfmt[]="diisxxxxxxxxxxxxxxxxiix";
const char MovieEntryfmt[]="nxx";
const char OverrideSpellDatafmt[]="niiiiiiiiiix";
const char QuestSortEntryfmt[]="nxxxxxxxxxxxxxxxxx";
const char QuestXPfmt[]="niiiiiiiiii";
const char QuestFactionRewardfmt[]="niiiiiiiiii";
const char PvPDifficultyfmt[]="diiiii";
const char RandomPropertiesPointsfmt[]="niiiiiiiiiiiiiii";
const char ScalingStatDistributionfmt[]="niiiiiiiiiiiiiiiiiiiii";
const char ScalingStatValuesfmt[]="iniiiiiiiiiiiiiiiiixiiii";
const char SkillLinefmt[]="nixssssssssssssssssxxxxxxxxxxxxxxxxxxixxxxxxxxxxxxxxxxxi";
const char SkillLineAbilityfmt[]="niiiixxiiiiixx";
const char SoundEntriesfmt[]="nxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
const char SpellCastTimefmt[]="nixx";
const char SpellDifficultyfmt[]="niiii";
const std::string CustomSpellDifficultyfmt="ppppp";
const std::string CustomSpellDifficultyIndex="id";
const char SpellDurationfmt[]="niii";
const char SpellEntryfmt[]="niiiiiiiixxiiiifiiiissxxiixxixiiiiiiixiiiiiiiix";
//const std::string CustomSpellEntryfmt="pappppppppaapapapaaaaaaaaaaapaaapapppppppaaaaapaapaaaaaaaaaaaaaaaaaappppppppppppppppppppppppppppppppppppaaaaaapppppppppaaapppppppppaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaappppppppapppaaaaappaaaaaaa";
//const std::string CustomSpellEntryIndex = "Id";
const char SpellAuraOptionsEntryfmt[]="diiii";
const char SpellAuraRestrictionsEntryfmt[]="diiiiiiii";
const char SpellCastingRequirementsEntryfmt[]="dixxixi";
const char SpellCategoriesEntryfmt[]="diiiiii";
const char SpellClassOptionsEntryfmt[]="dxiiii";
const char SpellCooldownsEntryfmt[]="diii";
const char SpellEffectEntryfmt[]="difiiixfiiiiiifixfiiiiiiii";
const char SpellEquippedItemsEntryfmt[]="diii";
const char SpellInterruptsEntryfmt[]="dixixi";
const char SpellLevelsEntryfmt[]="diii";
const char SpellPowerEntryfmt[]="diiiix";
const char SpellReagentsEntryfmt[]="diiiiiiiiiiiiiiii";
const char SpellScalingEntryfmt[]="diiiiffffffffffi";
const char SpellShapeshiftEntryfmt[]="dixixx";
const char SpellTargetRestrictionsEntryfmt[]="diiii";
const char SpellTotemsEntryfmt[]="diiii";
const char SpellFocusObjectfmt[]="nxxxxxxxxxxxxxxxxx";
const char SpellItemEnchantmentfmt[]="nxiiiiiixxxiiissssssssssssssssxiiiiiii";
const char SpellItemEnchantmentConditionfmt[]="nbbbbbxxxxxbbbbbbbbbbiiiiiXXXXX";
const char SpellRadiusfmt[]="nfxf";
const char SpellRangefmt[]="nffffixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
const char SpellRuneCostfmt[]="niiii";
const char SpellShapeshiftfmt[]="nxxxxxxxxxxxxxxxxxxiixiiixxiiiiiiii";
const char StableSlotPricesfmt[] = "ni";
const char SummonPropertiesfmt[] = "niiiii";
const char TalentEntryfmt[]="niiiiiiiixxxxixxixxxxxx";
const char TalentTabEntryfmt[]="nxxxxxxxxxxxxxxxxxxxiiix";
const char TaxiNodesEntryfmt[]="nifffssssssssssssssssxii";
const char TaxiPathEntryfmt[]="niii";
const char TaxiPathNodeEntryfmt[]="diiifffiiii";
const char TeamContributionPointsfmt[]="df";
const char TotemCategoryEntryfmt[]="nxxxxxxxxxxxxxxxxxii";
const char VehicleEntryfmt[]="niffffiiiiiiiifffffffffffffffssssfifiixx";
const char VehicleSeatEntryfmt[]="niiffffffffffiiiiiifffffffiiifffiiiiiiiffiiiiixxxxxxxxxxxx";
const char WMOAreaTableEntryfmt[]="niiixxxxxiixxxxxxxxxxxxxxxxx";
const char WorldMapAreaEntryfmt[]="xinxffffixx";
const char WorldMapOverlayEntryfmt[]="nxiiiixxxxxxxxxxx";
const char WorldSafeLocsEntryfmt[]="nifffxxxxxxxxxxxxxxxxx";

#endif
