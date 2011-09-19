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

#include "Creature.h"
#include "CreatureFormations.h"
#include "ObjectMgr.h"

#include "CreatureAI.h"

#define MAX_DESYNC 5.0f

CreatureFormationInfoType   CreatureFormationMap;
CreatureFormationDataType   CreatureFormationDataMap;

void CreatureFormationManager::AddCreatureToFormation(uint32 formationId, Creature* member)
{
    Map* map = member->FindMap();
    if (!map)
        return;

    CreatureFormationHolderType::iterator itr = map->CreatureFormationHolder.find(formationId);

    //Add member to an existing formation
    if (itr != map->CreatureFormationHolder.end())
    {
        sLog->outDebug(LOG_FILTER_UNITS, "Formation found: %u, inserting creature GUID: %u, Formation InstanceID %u", formationId, member->GetGUIDLow(), member->GetInstanceId());
        itr->second->AddMember(member);
    }
    //Create new formation
    else
    {
        sLog->outDebug(LOG_FILTER_UNITS, "Formation not found: %u. Creating new formation.", formationId);
        CreatureFormation* formation = new CreatureFormation(formationId);
        map->CreatureFormationHolder[formationId] = formation;
        formation->AddMember(member);
    }
}

void CreatureFormationManager::RemoveCreatureFromFormation(CreatureFormation *formation, Creature *member)
{
    sLog->outDebug(LOG_FILTER_UNITS, "Deleting member pointer to GUID: %u from formation %u", formation->GetId(), member->GetDBTableGUIDLow());
    formation->RemoveMember(member);

    if (formation->isEmpty())
    {
        Map *map = member->FindMap();
        if (!map)
            return;

        sLog->outDebug(LOG_FILTER_UNITS, "Deleting formation with InstanceID %u", member->GetInstanceId());
        map->CreatureFormationHolder.erase(formation->GetId());
        delete formation;
    }
}

void CreatureFormationManager::LoadCreatureFormations()
{
    uint32 oldMSTime = getMSTime();

    //Clear existing map
    for (CreatureFormationInfoType::iterator itr = CreatureFormationMap.begin(); itr != CreatureFormationMap.end(); ++itr) // for reload case 
        delete itr->second;
    CreatureFormationMap.clear();

    CreatureFormationDataMap.clear();

    //Check if member without formation exist
    PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_LOAD_CREFORMATION_DATA_WITHOUT_FORMATION);
    PreparedQueryResult result = WorldDatabase.Query(stmt);

    if (result)
        sLog->outDetail(">> %u Member without formation found, member skipped.",result->Fetch()->GetInt32());

    //Get formations
    stmt = WorldDatabase.GetPreparedStatement(WORLD_LOAD_CREFORMATIONS);
    PreparedQueryResult result_data = WorldDatabase.Query(stmt);

    if (!result_data)
    {
        sLog->outString(">> Loaded 0 creature formations. DB table `creature_formations` is empty.");
        sLog->outString();
        return;
    }

    //Get member
    stmt = WorldDatabase.GetPreparedStatement(WORLD_LOAD_CREFORMATION_DATA);
    PreparedQueryResult result_member = WorldDatabase.Query(stmt);

    if (!result_member)
    {
        sLog->outString(">> Loaded 0 formation formations. DB table `creature_formation_data` is empty.");
        sLog->outString();
        return;
    }

    std::set<uint32> guidSet;

    stmt = WorldDatabase.GetPreparedStatement(WORLD_LOAD_CREATURE_GUIDS);
    PreparedQueryResult guidResult = WorldDatabase.Query(stmt);

    if (guidResult)
    {
        do 
        {
            Field *fields = guidResult->Fetch();
            uint32 guid = fields[0].GetUInt32();

            guidSet.insert(guid);

        }
        while (guidResult->NextRow());
    }

    //Loading formations...
    uint32 formation_count = 0;

    FormationInfo *formation_info;

    do
    {
        Field* fields = result_data->Fetch();
        
        //Load formation member data
        uint32 formationId = fields[0].GetUInt32();
        uint32 leaderGUID = fields[1].GetUInt32();
        uint8  formationAI = fields[2].GetUInt8();

        formation_info                        = new FormationInfo;
        formation_info->leaderGUID            = leaderGUID;
        formation_info->formationAI           = formationAI; 

        // check data correctness
        if (guidSet.find(formation_info->leaderGUID) == guidSet.end())
        {
            sLog->outErrorDb("creature_formations table leader guid %u incorrect (not exist)", formation_info->leaderGUID);
            delete formation_info;
            return;
        }

        CreatureFormationMap[formationId] = formation_info;

        sLog->outDebug(LOG_FILTER_UNITS, "CreatureFormation::LoadCreatureFormations: Load Formation %u with Leader %u and formationAI %u.", formationId, leaderGUID, formationAI);
        ++formation_count;
    }
    while (result_data->NextRow()) ;

    //Loading member...
    uint32 member_count = 0;
    FormationData *formation_data;

    do
    {
        Field* fields = result_member->Fetch();
        
        //Load formation member data
        uint32 formationId = fields[0].GetUInt32();
        uint32 memberGUID = fields[1].GetUInt32();
        float follow_dist = fields[2].GetFloat();
        float follow_angle = fields[3].GetFloat() * M_PI / 180;

        formation_data                        = new FormationData;
        formation_data->formationId           = formationId;
        formation_data->follow_dist           = follow_dist; 
        formation_data->follow_angle          = follow_angle; 

        // check data correctness
        if (guidSet.find(memberGUID) == guidSet.end())
        {
            sLog->outErrorDb("creature_formation_data table member guid %u incorrect (not exist)", memberGUID);
            continue;
        }

        CreatureFormationDataMap[memberGUID] = formation_data;

        sLog->outDebug(LOG_FILTER_UNITS, "CreatureFormation::LoadCreatureFormations: Load Member %u for Formation with formationId %u.", memberGUID, formation_data->formationId);
        ++member_count;
    }
    while (result_member->NextRow()) ;

    sLog->outString(">> Loaded %u formations and %u creatures in formations in %u ms", formation_count, member_count, GetMSTimeDiffToNow(oldMSTime));
    sLog->outString();
}

void CreatureFormation::AddMember(Creature *member)
{
    if (!member)
        return;

    uint32 memberGUID = member->GetDBTableGUIDLow();

    sLog->outDebug(LOG_FILTER_UNITS, "CreatureFormation::AddMember: Adding unit GUID: %u to formation.", memberGUID);
    
    Formation* formation;
    formation = new Formation;

    //Check if it is a leader
    if (member->GetDBTableGUIDLow() == CreatureFormationMap.find(m_formationID)->second->leaderGUID)
    {
        sLog->outDebug(LOG_FILTER_UNITS, "Unit GUID: %u is formation leader.", memberGUID);
        m_leader = member;
        formation->follow_dist = 0;
        formation->follow_angle = 0;
    }
    else
    {
        formation->follow_dist = CreatureFormationDataMap.find(memberGUID)->second->follow_dist;
        formation->follow_angle = CreatureFormationDataMap.find(memberGUID)->second->follow_angle;
    }
    
    formation->formationAI = CreatureFormationMap.find(m_formationID)->second->formationAI; 
    formation->leaderGUID = CreatureFormationMap.find(m_formationID)->second->leaderGUID; 

    m_members[member] = formation;
    member->SetFormation(this);
}

void CreatureFormation::RemoveMember(Creature *member)
{
    if (!member)
        return;

    if (m_leader == member)
        m_leader = NULL;
    
    m_members.erase(member);
    member->SetFormation(NULL);
}

void CreatureFormation::MemberAttackStart(Creature *member, Unit *target)
{
    uint8 formationAI = CreatureFormationMap[m_formationID]->formationAI;

    if (!formationAI)
        return;

    if (formationAI == 1 && member != m_leader)
        return;

    for (CreatureFormationMemberType::iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        Creature* creature = itr->first;

        if (m_leader) // avoid crash if leader was killed and reset.
            sLog->outDebug(LOG_FILTER_UNITS, "CreatureFormation::MemberAttackStart: formation instance id %u calls member instid %u", m_leader->GetInstanceId(), member->GetInstanceId());

        //Skip one check
        if (creature == member)
            continue;

        if (!creature->isAlive())
            continue;

        if (creature->getVictim())
            continue;

        if (creature->IsAIEnabled)
            if (creature->IsValidAttackTarget(target))
                creature->AI()->AttackStart(target);
    }
}

void CreatureFormation::Reset(bool dismiss)
{
    for (CreatureFormationMemberType::iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        Creature* creature = itr->first;

        if (creature != m_leader && creature->isAlive())
        {
            if (dismiss)
                creature->GetMotionMaster()->Initialize();
            else
                creature->GetMotionMaster()->MoveIdle(MOTION_SLOT_IDLE);

            sLog->outDebug(LOG_FILTER_UNITS, "Set %s movement for member GUID: %u", dismiss ? "default" : "idle", creature->GetGUIDLow());
        }
    }
    m_Formed = !dismiss;
}

void CreatureFormation::LeaderMoveTo(float x, float y, float z)
{
    if (!m_leader)
        return;

    float pathangle = atan2(m_leader->GetPositionY() - y, m_leader->GetPositionX() - x);

    for (CreatureFormationMemberType::iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        Creature* creature = itr->first;
        if (creature == m_leader || !creature->isAlive() || creature->getVictim())
            continue;

        float angle = itr->second->follow_angle;
        float dist = itr->second->follow_dist;

        float dx = x + cos(angle + pathangle) * dist;
        float dy = y + sin(angle + pathangle) * dist;
        float dz = z;

        Trillium::NormalizeMapCoord(dx);
        Trillium::NormalizeMapCoord(dy);

        creature->UpdateGroundPositionZ(dx, dy, dz);

        if (creature->IsWithinDist(m_leader, dist + MAX_DESYNC))
            creature->SetUnitMovementFlags(m_leader->GetUnitMovementFlags());
        else
            creature->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);

        creature->GetMotionMaster()->MovePoint(0, dx, dy, dz);
        creature->SetHomePosition(dx, dy, dz, pathangle);
    }
}