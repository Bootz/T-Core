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

#include "Common.h"
#include "Language.h"
#include "DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Player.h"
#include "GossipDef.h"
#include "UpdateMask.h"
#include "ObjectAccessor.h"
#include "Creature.h"
#include "Pet.h"
#include "BattlegroundMgr.h"
#include "Battleground.h"
#include "ScriptMgr.h"
#include "CreatureAI.h"
#include "SpellInfo.h"

enum StableResultCode
{
    STABLE_ERR_MONEY        = 0x01,                         // "you don't have enough money"
    STABLE_ERR_STABLE       = 0x06,                         // currently used in most fail cases
    STABLE_SUCCESS_STABLE   = 0x08,                         // stable success
    STABLE_SUCCESS_UNSTABLE = 0x09,                         // unstable/swap success
    STABLE_SUCCESS_BUY_SLOT = 0x0A,                         // buy slot success
    STABLE_ERR_EXOTIC       = 0x0C,                         // "you are unable to control exotic creatures"
};

void WorldSession::HandleTabardVendorActivateOpcode(WorldPacket & recv_data)
{
    uint64 guid;
    recv_data >> guid;

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_TABARDDESIGNER);
    if (!unit)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: HandleTabardVendorActivateOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STAT_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    SendTabardVendorActivate(guid);
}

void WorldSession::SendTabardVendorActivate(uint64 guid)
{
    WorldPacket data(MSG_TABARDVENDOR_ACTIVATE, 8);
    data << guid;
    SendPacket(&data);
}

void WorldSession::HandleBankerActivateOpcode(WorldPacket & recv_data)
{
    uint64 guid;

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_BANKER_ACTIVATE");

    recv_data >> guid;

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_BANKER);
    if (!unit)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: HandleBankerActivateOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STAT_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    SendShowBank(guid);
}

void WorldSession::SendShowBank(uint64 guid)
{
    WorldPacket data(SMSG_SHOW_BANK, 8);
    data << guid;
    SendPacket(&data);
}

void WorldSession::HandleTrainerListOpcode(WorldPacket & recv_data)
{
    uint64 guid;
    uint32 spellId, unk;

    recv_data >> guid;
    recv_data >> spellId >> unk;
    SendTrainerList(guid);
}

void WorldSession::SendTrainerList(uint64 guid)
{
    std::string str = GetTrilliumString(LANG_NPC_TAINER_HELLO);
    SendTrainerList(guid, str);
}

void WorldSession::SendTrainerList(uint64 guid, const std::string &strTitle)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: SendTrainerList");

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_TRAINER);
    if (!unit)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: SendTrainerList - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STAT_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    // trainer list loaded at check;
    if (!unit->isCanTrainingOf(_player, true))
        return;

    CreatureTemplate const *creatureInfo = unit->GetCreatureInfo();

    if (!creatureInfo)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: SendTrainerList - (GUID: %u) NO CREATUREINFO!", GUID_LOPART(guid));
        return;
    }

    TrainerSpellData const* trainer_spells = unit->GetTrainerSpells();
    if (!trainer_spells)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: SendTrainerList - Training spells not found for creature (GUID: %u Entry: %u)",
            GUID_LOPART(guid), unit->GetEntry());
        return;
    }

    WorldPacket data(SMSG_TRAINER_LIST, 8 + 4 + 4 + trainer_spells->spellList.size() * 38 + strTitle.size() + 1);
    data << guid;
    data << uint32(trainer_spells->trainerType);
    data << uint32(creatureInfo->trainer_id);             // Get TrainerID from creature_template.

    size_t count_pos = data.wpos();
    data << uint32(trainer_spells->spellList.size());

    // reputation discount
    uint32 count = 0;
    float fDiscountMod = _player->GetReputationPriceDiscount(unit);

    for (TrainerSpellMap::const_iterator itr = trainer_spells->spellList.begin(); itr != trainer_spells->spellList.end(); ++itr)
    {
        TrainerSpell const* tSpell = &itr->second;

        TrainerSpellState state = _player->GetTrainerSpellState(tSpell);

        data << uint32(tSpell->spell);                      // learned spell (or cast-spell in profession case)
        data << uint8(state);
        data << uint32(floor(tSpell->spellCost * fDiscountMod));
        data << uint8(tSpell->reqLevel);
        data << uint32(tSpell->reqSkill);
        data << uint32(tSpell->reqSkillValue);
        data << uint32(0);                                  // 4.0.3
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);

        ++count;
    }

    data << strTitle;

    data.put<uint32>(count_pos, count);
    SendPacket(&data);
}

void WorldSession::HandleTrainerBuySpellOpcode(WorldPacket & recv_data)
{
    uint64 guid;
    uint32 trainerId;
    uint32 spellId, result = ERR_TRAINER_OK;

    recv_data >> guid >> trainerId >> spellId;
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_TRAINER_BUY_SPELL NpcGUID=%u, learn spell id is: %u", uint32(GUID_LOPART(guid)), spellId);

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_TRAINER);
    if (!unit)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: HandleTrainerBuySpellOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STAT_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    if (!unit->isCanTrainingOf(_player, true))
        return;

    // check present spell in trainer spell list
    TrainerSpellData const* trainer_spells = unit->GetTrainerSpells();
    if (!trainer_spells)
        return;

    // not found, cheat?
    TrainerSpell const* trainer_spell = trainer_spells->Find(spellId);
    if (!trainer_spell)
        return;

    // can't be learn, cheat? Or double learn with lags...
    if (_player->GetTrainerSpellState(trainer_spell) != TRAINER_SPELL_GREEN)
        return;

    // apply reputation discount
    uint64 nSpellCost = uint32(floor(trainer_spell->spellCost * _player->GetReputationPriceDiscount(unit)));

    // check money requirement
    if (!_player->HasEnoughMoney(nSpellCost))
        result = ERR_TRAINER_NOT_ENOUGH_MONEY;

    if (result == ERR_TRAINER_OK)
    {
        _player->ModifyMoney(-int32(nSpellCost));

    unit->SendPlaySpellVisual(179); // 53 SpellCastDirected
    unit->SendPlaySpellImpact(_player->GetGUID(), 362); // 113 EmoteSalute

        // learn explicitly or cast explicitly
        if (trainer_spell->IsCastable())
            _player->CastSpell(_player, trainer_spell->spell, true);
        else
            _player->learnSpell(spellId, false);
    }

    WorldPacket data(SMSG_TRAINER_BUY_RESULT, 16);
    data << uint64(guid);
    data << uint32(spellId);                                // should be same as in packet from client
    data << uint32(result);
    SendPacket(&data);
}

void WorldSession::HandleGossipHelloOpcode(WorldPacket & recv_data)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_GOSSIP_HELLO");

    uint64 guid;
    recv_data >> guid;

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_NONE);
    if (!unit)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: HandleGossipHelloOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // set faction visible if needed
    if (FactionTemplateEntry const* factionTemplateEntry = sFactionTemplateStore.LookupEntry(unit->getFaction()))
        _player->GetReputationMgr().SetVisible(factionTemplateEntry);

    GetPlayer()->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TALK);
    // remove fake death
    //if (GetPlayer()->HasUnitState(UNIT_STAT_DIED))
    //    GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    if (unit->isArmorer() || unit->isCivilian() || unit->isQuestGiver() || unit->isServiceProvider() || unit->isGuard())
    {
        unit->StopMoving();
    }

    // If spiritguide, no need for gossip menu, just put player into resurrect queue
    if (unit->isSpiritGuide())
    {
        Battleground *bg = _player->GetBattleground();
        if (bg)
        {
            bg->AddPlayerToResurrectQueue(unit->GetGUID(), _player->GetGUID());
            sBattlegroundMgr->SendAreaSpiritHealerQueryOpcode(_player, bg, unit->GetGUID());
            return;
        }
    }

    if (!sScriptMgr->OnGossipHello(_player, unit))
    {
//        _player->TalkedToCreature(unit->GetEntry(), unit->GetGUID());
        _player->PrepareGossipMenu(unit, unit->GetCreatureInfo()->GossipMenuId, true);
        _player->SendPreparedGossip(unit);
    }
    unit->AI()->sGossipHello(_player);
}

void WorldSession::HandleSpiritHealerActivateOpcode(WorldPacket & recv_data)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_SPIRIT_HEALER_ACTIVATE");

    uint64 guid;

    recv_data >> guid;

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_SPIRITHEALER);
    if (!unit)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: HandleSpiritHealerActivateOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STAT_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    SendSpiritResurrect();
}

void WorldSession::SendSpiritResurrect()
{
    _player->ResurrectPlayer(0.5f, true);

    _player->DurabilityLossAll(0.25f, true);

    // get corpse nearest graveyard
    WorldSafeLocsEntry const *corpseGrave = NULL;
    Corpse *corpse = _player->GetCorpse();
    if (corpse)
        corpseGrave = sObjectMgr->GetClosestGraveYard(
            corpse->GetPositionX(), corpse->GetPositionY(), corpse->GetPositionZ(), corpse->GetMapId(), _player->GetTeam());

    // now can spawn bones
    _player->SpawnCorpseBones();

    // teleport to nearest from corpse graveyard, if different from nearest to player ghost
    if (corpseGrave)
    {
        WorldSafeLocsEntry const *ghostGrave = sObjectMgr->GetClosestGraveYard(
            _player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ(), _player->GetMapId(), _player->GetTeam());

        if (corpseGrave != ghostGrave)
            _player->TeleportTo(corpseGrave->map_id, corpseGrave->x, corpseGrave->y, corpseGrave->z, _player->GetOrientation());
        // or update at original position
        else
            _player->UpdateObjectVisibility();
    }
    // or update at original position
    else
        _player->UpdateObjectVisibility();
}

void WorldSession::HandleBinderActivateOpcode(WorldPacket & recv_data)
{
    uint64 npcGUID;
    recv_data >> npcGUID;

    if (!GetPlayer()->IsInWorld() || !GetPlayer()->isAlive())
        return;

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(npcGUID, UNIT_NPC_FLAG_INNKEEPER);
    if (!unit)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: HandleBinderActivateOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(npcGUID)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STAT_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    SendBindPoint(unit);
}

void WorldSession::SendBindPoint(Creature *npc)
{
    // prevent set homebind to instances in any case
    if (GetPlayer()->GetMap()->Instanceable())
        return;

    uint32 bindspell = 3286;

    // update sql homebind
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SET_PLAYER_HOMEBIND);
    stmt->setUInt16(0, _player->GetMapId());
    stmt->setUInt16(1, _player->GetAreaId());
    stmt->setFloat (2, _player->GetPositionX());
    stmt->setFloat (3, _player->GetPositionY());
    stmt->setFloat (4, _player->GetPositionZ());
    stmt->setUInt32(5, _player->GetGUIDLow());
    CharacterDatabase.Execute(stmt);

    _player->m_homebindMapId = _player->GetMapId();
    _player->m_homebindAreaId = _player->GetAreaId();
    _player->m_homebindX = _player->GetPositionX();
    _player->m_homebindY = _player->GetPositionY();
    _player->m_homebindZ = _player->GetPositionZ();

    // send spell for homebinding (3286)
    npc->CastSpell(_player, bindspell, true);

    WorldPacket data(SMSG_TRAINER_BUY_RESULT, (8+4));
    data << uint64(npc->GetGUID());
    data << uint32(bindspell);
    SendPacket(&data);

    _player->PlayerTalkClass->SendCloseGossip();
}

void WorldSession::HandleListStabledPetsOpcode(WorldPacket & recv_data)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Recv MSG_LIST_STABLED_PETS");
    uint64 npcGUID;

    recv_data >> npcGUID;

    if (!CheckStableMaster(npcGUID))
        return;

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STAT_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    // remove mounts this fix bug where getting pet from stable while mounted deletes pet.
    if (GetPlayer()->IsMounted())
        GetPlayer()->RemoveAurasByType(SPELL_AURA_MOUNTED);

    SendStablePet(npcGUID);
}

void WorldSession::SendStablePet(uint64 guid)
{
    _sendStabledPetCallback.SetParam(guid);
    _sendStabledPetCallback.SetFutureResult(
        CharacterDatabase.AsyncPQuery("SELECT owner, slot, id, entry, level, name FROM character_pet WHERE owner = '%u' AND slot >= '%u' AND slot <= '%u' ORDER BY slot",
            _player->GetGUIDLow(), PET_SLOT_HUNTER_FIRST, PET_SLOT_STABLE_LAST)
        );
}

void WorldSession::SendStablePetCallback(QueryResult result, uint64 guid)
{
    if (!GetPlayer())
        return;

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Recv MSG_LIST_STABLED_PETS Send.");

    WorldPacket data(MSG_LIST_STABLED_PETS, 200);           // guess size

    data << uint64 (guid);

    Pet *pet = _player->GetPet();

    size_t wpos = data.wpos();
    data << uint8(0);                                       // place holder for slot show number

    data << uint8(20);

    uint8 num = 0;                                          // counter for place holder

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            data << uint32(fields[1].GetUInt32());                  // slot
            data << uint32(fields[2].GetUInt32());                  // petnumber
            data << uint32(fields[3].GetUInt32());                  // creature entry
            data << uint32(fields[4].GetUInt32());                  // level
            data << fields[5].GetString();                          // name
            data << uint8(fields[1].GetUInt32() <= PET_SLOT_STABLE_FIRST ? 1 : 2);   // 1 = current, 2/3 = in stable (any from 4,5,... create problems with proper show)



            ++num;
        }
        while (result->NextRow());
    }

    data.put<uint8>(wpos, num);                             // set real data to placeholder
    SendPacket(&data);
}

void WorldSession::SendStableResult(uint8 res)
{
    WorldPacket data(SMSG_STABLE_RESULT, 1);
    data << uint8(res);
    SendPacket(&data);
}

void WorldSession::HandleStableRevivePet(WorldPacket & /*recv_data*/)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "HandleStableRevivePet: Not implemented");
}

void WorldSession::HandleStableChangeSlot(WorldPacket & recv_data)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Recv CMSG_STABLE_CHANGE_SLOT.");
    uint32 petnumber;
    uint64 npcGUID;
    uint8 newslot = 0;

    recv_data >> npcGUID >> petnumber;

    if (!CheckStableMaster(npcGUID))
    {
        SendStableResult(STABLE_ERR_STABLE);
        return;
    }

    if (newslot > MAX_PET_STABLES)
    {
        SendStableResult(STABLE_ERR_STABLE);
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STAT_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    Pet* pet = _player->GetPet();

    // If we move the pet already summoned...
    if (pet && pet->GetCharmInfo() && pet->GetCharmInfo()->GetPetNumber() == petnumber)
        _player->RemovePet(pet, PET_SLOT_ACTUAL_PET_SLOT);

    //If we move to the pet already summoned...
    if (pet && GetPlayer()->m_currentPetSlot == newslot)
        _player->RemovePet(pet, PET_SLOT_ACTUAL_PET_SLOT);

    _stableChangeSlotCallback.SetParam(newslot);
    _stableChangeSlotCallback.SetFutureResult(
        CharacterDatabase.PQuery("SELECT slot,entry,id FROM character_pet WHERE owner = '%u' AND id = '%u'",
        _player->GetGUIDLow(), petnumber));
}

void WorldSession::HandleStableChangeSlotCallback(QueryResult result, uint8 newslot)
{
    if (!GetPlayer())
        return;

    if (!result)
    {
        SendStableResult(STABLE_ERR_STABLE);
        return;
    }

    Field* fields = result->Fetch();

    uint32 slot        = fields[0].GetUInt8();
    uint32 creature_id = fields[1].GetUInt32();
    uint32 pet_number  = fields[2].GetUInt32();

    if (!creature_id)
    {
        SendStableResult(STABLE_ERR_STABLE);
        return;
    }

    CreatureTemplate const* creatureInfo = sObjectMgr->GetCreatureTemplate(creature_id);
    if (!creatureInfo || !creatureInfo->isTameable(_player->CanTameExoticPets()))
    {
        // if problem in exotic pet
        if (creatureInfo && creatureInfo->isTameable(true))
            SendStableResult(STABLE_ERR_EXOTIC);
        else
            SendStableResult(STABLE_ERR_STABLE);
        return;
    }

    CharacterDatabase.PExecute("UPDATE character_pet SET slot = '%u' WHERE slot = '%u' AND owner='%u'", newslot, slot, GetPlayer()->GetGUIDLow());
    CharacterDatabase.PExecute("UPDATE character_pet SET slot = '%u' WHERE slot = '%u' AND owner='%u' AND id<>'%u'", slot, newslot, GetPlayer()->GetGUIDLow(), pet_number);

    SendStableResult(STABLE_SUCCESS_STABLE);
}

void WorldSession::HandleRepairItemOpcode(WorldPacket & recv_data)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_REPAIR_ITEM");

    uint64 npcGUID, itemGUID;
    uint8 guildBank;                                        // new in 2.3.2, bool that means from guild bank money

    recv_data >> npcGUID >> itemGUID >> guildBank;

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(npcGUID, UNIT_NPC_FLAG_REPAIR);
    if (!unit)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: HandleRepairItemOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(npcGUID)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STAT_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    // reputation discount
    float discountMod = _player->GetReputationPriceDiscount(unit);

    uint32 TotalCost = 0;
    if (itemGUID)
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "ITEM: Repair item, itemGUID = %u, npcGUID = %u", GUID_LOPART(itemGUID), GUID_LOPART(npcGUID));

        Item* item = _player->GetItemByGuid(itemGUID);
        if (item)
            TotalCost = _player->DurabilityRepair(item->GetPos(), true, discountMod, guildBank);
    }
    else
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "ITEM: Repair all items, npcGUID = %u", GUID_LOPART(npcGUID));
        TotalCost = _player->DurabilityRepairAll(true, discountMod, guildBank);
    }
}

