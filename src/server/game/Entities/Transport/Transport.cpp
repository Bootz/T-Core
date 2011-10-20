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
#include "Transport.h"
#include "MapManager.h"
#include "ObjectMgr.h"
#include "Path.h"
#include "ScriptMgr.h"
#include "WorldPacket.h"
#include "DataStorage.h"
#include "World.h"
#include "GameObjectAI.h"
#include "Vehicle.h"

Transport::Transport() : GameObject(), _isStopped(false)
{
    m_updateFlag = (UPDATEFLAG_TRANSPORT | UPDATEFLAG_HAS_POSITION | UPDATEFLAG_ROTATION);
}

Transport::~Transport()
{
    for (UnitSet::iterator itr = _passengers.begin(); itr != _passengers.end(); ++itr)
    {
        (*itr)->SetTransport(NULL);
        if (Creature* passenger = (*itr)->ToCreature())
        {
            Map* map = passenger->GetMap();
            passenger->CleanupsBeforeDelete();
            map->Remove(passenger, true);
        }
    }

    _passengers.clear();
}

bool Transport::Create(uint32 guidlow, uint32 entry, uint32 mapid, float x, float y, float z, float ang, uint32 animprogress, uint32 dynflags)
{
    Relocate(x, y, z, ang);

    if (!IsPositionValid())
    {
        sLog->outError("Transport (GUID: %u) not created. Suggested coordinates isn't valid (X: %f Y: %f)",
            guidlow, x, y);
        return false;
    }

    Object::_Create(guidlow, 0, HIGHGUID_MO_TRANSPORT);

    GameObjectTemplate const* goinfo = sObjectMgr->GetGameObjectTemplate(entry);

    if (!goinfo)
    {
        sLog->outErrorDb("Transport not created: entry in `gameobject_template` not found, guidlow: %u map: %u  (X: %f Y: %f Z: %f) ang: %f", guidlow, mapid, x, y, z, ang);
        return false;
    }

    m_goInfo = goinfo;

    TransportTemplate const* tInfo = sTransportMgr->GetTransportTemplate(entry);
    if (!tInfo)
    {
        sLog->outError("Transport %u (name: %s) will not be created, missing `transport_template` entry.", entry, goinfo->name.c_str());
        return false;
    }

    _transportInfo = tInfo;

    // initialize waypoints
    _nextFrame = tInfo->keyFrames.begin();
    _currentFrame = _nextFrame++;

    SetFloatValue(OBJECT_FIELD_SCALE_X, goinfo->size);
    SetUInt32Value(GAMEOBJECT_FACTION, goinfo->faction);
    SetUInt32Value(GAMEOBJECT_FLAGS, goinfo->flags);
    SetUInt32Value(GAMEOBJECT_LEVEL, tInfo->pathTime);
    SetEntry(goinfo->entry);

    SetUInt32Value(GAMEOBJECT_DISPLAYID, goinfo->displayId);

    SetGoState(GO_STATE_READY);
    SetGoType(GameobjectTypes(goinfo->type));

    SetGoAnimProgress(animprogress);
    if (dynflags)
        SetUInt32Value(GAMEOBJECT_DYNAMIC, MAKE_PAIR32(0, dynflags));

    SetName(goinfo->name);
    return true;
}

float Transport::CalculateSegmentPos(float now)
{
    const float speed = float(m_goInfo->moTransport.moveSpeed);
    const float accel = float(m_goInfo->moTransport.accelRate);
    KeyFrame const& frame = *_currentFrame;
    float timeSinceStop = frame.timeFrom + (now - (1.0f/IN_MILLISECONDS) * frame.departureTime);
    float timeUntilStop = frame.timeTo - (now - (1.0f/IN_MILLISECONDS) * frame.departureTime);
    // timeSinceStop includes the waiting time, so check if we move already
    /* if (frame.IsStopFrame())
    {
        if (timeSinceStop < frame.node->delay)
            return 0.0f;
        timeSinceStop -= frame.node->delay;
    } */
    float segmentPos, dist;
    float accelTime = _transportInfo->accelTime;
    float accelDist = _transportInfo->accelDist;
    // calculate from nearest stop, less confusing calculation...
    if (timeSinceStop < timeUntilStop)
    {
        if (timeSinceStop < accelTime)
            dist = 0.5f * accel * timeSinceStop * timeSinceStop;
        else
            dist = accelDist + (timeSinceStop - accelTime) * speed;
        segmentPos = dist - frame.distSinceStop;
    }
    else
    {
        if (timeUntilStop < _transportInfo->accelTime)
            dist = 0.5f * accel * timeUntilStop * timeUntilStop;
        else
            dist = accelDist + (timeUntilStop - accelTime) * speed;
        segmentPos = frame.distUntilStop - dist;
    }

    float t = segmentPos / _nextFrame->distFromPrev;
    if (t < -0.01f || t > 1.01f)
    {
        sLog->outError("strange t=%f (%s, %u, %u)", t, GetName(), m_goInfo->moTransport.mapID, frame.node->index);
        sLog->outDetail("tSince: %f tUntil: %f dist: %f segmentPos: %f, speed %f, accel: %f, accelTime: %f",
            timeSinceStop, timeUntilStop, dist, segmentPos, speed, accel, accelTime);
    }
    return t;
}

void Transport::MoveToNextWayPoint()
{
    _currentFrame = _nextFrame++;
    if (_nextFrame == GetKeyFrames().end())
        _nextFrame = GetKeyFrames().begin();
}

void Transport::TeleportTransport(uint32 newMapid, float x, float y, float z)
{
    Map const* oldMap = GetMap();
    Relocate(x, y, z);

    // we need to create and save new Map object with 'newMapid' because if not done -> lead to invalid Map object reference...
    // player far teleport would try to create same instance, but we need it NOW for transport...
    RemoveFromWorld();
    ResetMap();
    Map * newMap = sMapMgr->CreateMap(newMapid, this, 0);
    SetMap(newMap);
    ASSERT(GetMap());
    AddToWorld();

    for (UnitSet::iterator itr = _passengers.begin(); itr != _passengers.end();)
    {
        Unit* passenger = *itr;
        ++itr;

        switch (passenger->GetTypeId())
        {
            case TYPEID_UNIT:
                passenger->ToCreature()->FarTeleportTo(newMap, x, y, z, passenger->GetOrientation());
                break;
            case TYPEID_PLAYER:
                if (passenger->isDead() && !passenger->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
                    passenger->ToPlayer()->ResurrectPlayer(1.0f);
                passenger->ToPlayer()->TeleportTo(newMapid, x, y, z, GetOrientation(), TELE_TO_NOT_LEAVE_TRANSPORT);
                break;
        }
    }

    if (oldMap != newMap)
    {
        UpdateForMap(oldMap);
        UpdateForMap(newMap);
    }

    MoveToNextWayPoint();
}

void Transport::AddPassenger(Unit* passenger)
{
    if (_passengers.insert(passenger).second)
        sLog->outDetail("Unit %s boarded transport %s.", passenger->GetName(), GetName());

    if (Player* plr = passenger->ToPlayer())
        sScriptMgr->OnAddPassenger(this, plr);
}

void Transport::RemovePassenger(Unit* passenger)
{
    if (_passengers.erase(passenger))
        sLog->outDetail("Unit %s removed from transport %s.", passenger->GetName(), GetName());

    if (Player* plr = passenger->ToPlayer())
        sScriptMgr->OnRemovePassenger(this, plr);
}

void Transport::Update(uint32 diff)
{
    if (!AI())
    {
        if (!AIM_Initialize())
            sLog->outError("Could not initialize GameObjectAI for Transport");
    }
    else
        AI()->UpdateAI(diff);

    if (_isStopped)
        return;

    if (GetKeyFrames().size() <= 1)
        return;

    //_moveTimer = getMSTime() % _transportInfo->pathTime;
    _moveTimer += diff;
    _moveTimer %= _transportInfo->pathTime;
    // need restart path from beginning
    /* if (m_timer < m_curr->pathTime)
    {
        m_curr = keyFrames.begin();
        m_next = m_curr + 1;
    } */
    while (_moveTimer > _nextFrame->pathTime || _moveTimer < _currentFrame->departureTime)
    {
        // arrived at next stop point
        if (_transportInfo->pathTime > _nextFrame->pathTime && _moveTimer < _nextFrame->departureTime)
        {
            if (IsMoving())
            {
                SetMoving(false);
                DoEventIfAny(*_currentFrame, false);
            }
            break;
        }


        MoveToNextWayPoint();

        SetMoving(true);

        DoEventIfAny(*_currentFrame, true);

        // first check help in case client-server transport coordinates de-synchronization
        if (_currentFrame->IsTeleportFrame())
            TeleportTransport(_nextFrame->node->mapid, _nextFrame->node->x, _nextFrame->node->y, _nextFrame->node->z);

        ASSERT(_nextFrame != GetKeyFrames().begin());

        sScriptMgr->OnRelocate(this, _currentFrame->node->index, _currentFrame->node->mapid, _currentFrame->node->x, _currentFrame->node->y, _currentFrame->node->z);

        sLog->outDebug(LOG_FILTER_TRANSPORTS, "%s moved to %f %f %f %d", GetName(), GetPositionX(), GetPositionY(), GetPositionZ(), _currentFrame->node->mapid);
    }

    if (IsMoving())
    {
        if (_moveTimer < _currentFrame->departureTime || _moveTimer > _nextFrame->pathTime)
            sLog->outError("strange times, c.dep:%u, n.pt:%u (%s, %u)", _currentFrame->departureTime, _nextFrame->pathTime, GetName(), m_goInfo->moTransport.mapID);
        float t = CalculateSegmentPos((float)_moveTimer/(float)IN_MILLISECONDS);
        //if (t < -0.01f || t > 1.01f)
        //    sLog.outError("strange t=%f (%s, %u)", t, GetName(), m_goInfo->moTransport.mapID);
        //G3D::Vector3 pos;
        //m_spline->Evaluate(m_curr->node->index - 1, t, pos);
        //G3D::Vector3 dir;
        //m_spline->EvaluateDerivative(m_curr->node->index - 1, t, dir);
        //dir.z = 0.0f;
        //dir = -dir.direction();
        //Relocate(pos.x, pos.y, pos.z);
        float x = _currentFrame->node->x * (1.0f - t) + _nextFrame->node->x * t;
        float y = _currentFrame->node->y * (1.0f - t) + _nextFrame->node->y * t;
        float z = _currentFrame->node->z * (1.0f - t) + _nextFrame->node->z * t;
        float o = GetAngle(_nextFrame->node->x, _nextFrame->node->y) + float(M_PI);
        Relocate(x, y, z, o);
        UpdatePassengerPositions();
    }

    sScriptMgr->OnTransportUpdate(this, diff);
}

void Transport::UpdateForMap(Map const* targetMap)
{
    Map::PlayerList const& players = targetMap->GetPlayers();
    if (players.isEmpty())
        return;

    if (GetMapId() == targetMap->GetId())
    {
        for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
        {
            if (this != itr->getSource()->GetTransport())
            {
                UpdateData transData(GetMapId());
                BuildCreateUpdateBlockForPlayer(&transData, itr->getSource());
                WorldPacket packet;
                transData.BuildPacket(&packet);
                itr->getSource()->SendDirectMessage(&packet);
            }
        }
    }
    else
    {
        UpdateData transData(targetMap->GetId());
        BuildOutOfRangeUpdateBlock(&transData);
        WorldPacket out_packet;
        transData.BuildPacket(&out_packet);

        for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
            if (this != itr->getSource()->GetTransport())
                itr->getSource()->SendDirectMessage(&out_packet);
    }
}

void Transport::DoEventIfAny(KeyFrame const& node, bool departure)
{
    if (uint32 eventid = departure ? node.node->departureEventID : node.node->arrivalEventID)
    {
        sLog->outDebug(LOG_FILTER_MAPSCRIPTS, "Taxi %s event %u of node %u of %s path", departure ? "departure" : "arrival", eventid, node.node->index, GetName());
        GetMap()->ScriptsStart(sEventScripts, eventid, this, this);
        EventInform(eventid);
    }
}

void Transport::BuildStartMovePacket(Map const* targetMap)
{
    SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
    SetGoState(GO_STATE_ACTIVE);
    SetStopped(false);
    UpdateForMap(targetMap);
}

void Transport::BuildStopMovePacket(Map const* targetMap)
{
    RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
    SetGoState(GO_STATE_READY);
    SetStopped(true);
    UpdateForMap(targetMap);
}

Creature* Transport::CreateNPCPassenger(uint32 entry, float x, float y, float z, float o, CreatureData const* data /*= NULL*/)
{
    Map* map = GetMap();
    Creature* creature = new Creature();

    if (!creature->Create(sObjectMgr->GenerateLowGuid(HIGHGUID_UNIT), map, GetPhaseMask(), entry, 0, GetGOInfo()->faction, 0.0f, 0.0f, 0.0f, 0.0f, data))
    {
        delete creature;
        return NULL;
    }

    creature->SetTransport(this);
    creature->AddUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT);
    creature->m_movementInfo.guid = GetGUID();
    creature->m_movementInfo.t_pos.Relocate(x, y, z, o);
    o += GetOrientation();
    MapManager::NormalizeOrientation(o);

    creature->Relocate(GetPositionX() + (x * cos(GetOrientation()) + y * sin(GetOrientation() + float(M_PI))),
                       GetPositionY() + (y * cos(GetOrientation()) + x * sin(GetOrientation())),
                       z + GetPositionZ(), o);

    creature->SetHomePosition(creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ(), creature->GetOrientation());

    if (!creature->IsPositionValid())
    {
        sLog->outError("Creature (guidlow %d, entry %d) not created. Suggested coordinates isn't valid (X: %f Y: %f)",creature->GetGUIDLow(),creature->GetEntry(),creature->GetPositionX(),creature->GetPositionY());
        delete creature;
        return NULL;
    }

    creature->setActive(true);
    map->Add(creature);
    AddPassenger(creature);

    sScriptMgr->OnAddCreaturePassenger(this, creature);
    return creature;
}

void Transport::UpdatePassengerPositions()
{
    for (UnitSet::iterator itr = _passengers.begin(); itr != _passengers.end(); ++itr)
    {
        Unit* passenger = *itr;
        // transport teleported but passenger not yet (can happen for players)
        if (passenger->GetMap() != GetMap())
            continue;
        float x = GetPositionX() + (passenger->m_movementInfo.t_pos.m_positionX * cos(GetOrientation()) + passenger->m_movementInfo.t_pos.m_positionY * sin(GetOrientation() + M_PI));
        float y = GetPositionY() + (passenger->m_movementInfo.t_pos.m_positionY * cos(GetOrientation()) + passenger->m_movementInfo.t_pos.m_positionX * sin(GetOrientation()));
        float z = GetPositionZ() + passenger->m_movementInfo.t_pos.m_positionZ;
        float o = GetOrientation() + passenger->m_movementInfo.t_pos.m_orientation;
        MapManager::NormalizeOrientation(o);
        switch (passenger->GetTypeId())
        {
            case TYPEID_UNIT:
                passenger->ToCreature()->SetHomePosition(x, y, z, o);
                GetMap()->CreatureRelocation(passenger->ToCreature(), x, y, z, o, false);
                break;
            case TYPEID_PLAYER:
                GetMap()->PlayerRelocation(passenger->ToPlayer(), x, y, z, o);
                break;
        }

        if (passenger->IsVehicle())
            passenger->GetVehicleKit()->RelocatePassengers(x, y, z, o);
    }
}
