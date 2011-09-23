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

#include "TransportMgr.h"
#include "Transport.h"
#include "InstanceScript.h"

TransportMgr::TransportMgr()
{
}

TransportMgr::~TransportMgr()
{
}

void TransportMgr::Unload()
{
    for (TransportSet::iterator itr = _transportUpdates.begin(); itr != _transportUpdates.end(); ++itr)
    {
        (*itr)->RemoveFromWorld();
        (*itr)->ResetMap();
        delete *itr;
    }
    _transportUpdates.clear();
    _transportMap.clear();
    _transportTemplates.clear();
}

void TransportMgr::LoadTransportTemplates()
{
    uint32 oldMSTime = getMSTime();

    QueryResult result = WorldDatabase.Query("SELECT entry, name, ScriptName FROM transport_template");

    if (!result)
    {
        sLog->outString(">> Loaded 0 transport templates. DB table `transport_template` is empty!");
        sLog->outString();
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();
        uint32 entry = fields[0].GetUInt32();
        std::string name = fields[1].GetString();
        GameObjectTemplate const* goInfo = sObjectMgr->GetGameObjectTemplate(entry);
        if (!goInfo)
        {
            sLog->outErrorDb("Transport %u (name: %s) is missing `gameobject_template` entry, skipped.", entry, name.c_str());
            continue;
        }

        if (goInfo->type != GAMEOBJECT_TYPE_MO_TRANSPORT)
        {
            sLog->outErrorDb("Transport %u (name: %s) specifies a `gameobject_template` entry with type other than GAMEOBJECT_TYPE_MO_TRANSPORT (15), skipped.", entry, name.c_str());
            continue;
        }

        if (goInfo->moTransport.taxiPathId >= sTaxiPathNodesByPath.size())
        {
            sLog->outErrorDb("Transport %u (name: %s) has an invalid path specified in `gameobject_template`.`data0` (%u) field, skipped.", entry, name.c_str(), goInfo->moTransport.taxiPathId);
            continue;
        }

        TransportTemplate& transport = _transportTemplates[entry];
        transport.name = name;
        transport.scriptId = sObjectMgr->GetScriptId(fields[2].GetCString());

        // paths are generated per template, saves us from generating it again in case of instanced transports
        GeneratePath(goInfo, &transport);

        // transports in instance are only on one map
        if (goInfo->moTransport.inInstance)
            _instanceTransports[*transport.mapsUsed.begin()].insert(entry);

        ++count;
    } while (result->NextRow());

    sLog->outString(">> Loaded %u transport templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    sLog->outString();
}

void TransportMgr::GeneratePath(GameObjectTemplate const* goInfo, TransportTemplate* transport)
{
    uint32 pathId = goInfo->moTransport.taxiPathId;
    TaxiPathNodeList const& path = sTaxiPathNodesByPath[pathId];
    std::vector<KeyFrame>& keyFrames = transport->keyFrames;
    bool mapChange = false;
    for (size_t i = 1; i < path.size() - 1; ++i)
    {
        if (!mapChange)
        {
            TaxiPathNodeEntry const& node_i = path[i];
            if (node_i.actionFlag == 1 || node_i.mapid != path[i+1].mapid)
            {
                keyFrames.back().teleport = true;
                mapChange = true;
            }
            else
            {
                KeyFrame k(node_i);
                keyFrames.push_back(k);
                transport->mapsUsed.insert(k.node->mapid);
            }
        }
        else
            mapChange = false;
    }

    // last to first is always "teleport", even for closed paths
    keyFrames.back().teleport = true;

    const float speed = float(goInfo->moTransport.moveSpeed);
    const float accel = float(goInfo->moTransport.accelRate);
    const float accel_dist = 0.5f * speed * speed / accel;

    transport->accelTime = speed / accel;
    transport->accelDist = accel_dist;

    int32 firstStop = -1;
    int32 lastStop = -1;

    // first cell is arrived at by teleportation :S
    keyFrames[0].distFromPrev = 0;
    if (keyFrames[0].IsStopFrame())
    {
        firstStop = 0;
        lastStop = 0;
    }

    bool closed = (keyFrames[0].node->mapid == keyFrames.back().node->mapid) && (keyFrames[0].node->actionFlag != 1);
    // find the rest of the distances between key points
    for (size_t i = 1; i < keyFrames.size(); ++i)
    {
        if (keyFrames[i-1].teleport)
            keyFrames[i].distFromPrev = 0;
        else
            keyFrames[i].distFromPrev = sqrt(pow(keyFrames[i].node->x - keyFrames[i - 1].node->x, 2) +
                pow(keyFrames[i].node->y - keyFrames[i - 1].node->y, 2) +
                pow(keyFrames[i].node->z - keyFrames[i - 1].node->z, 2));

        if (keyFrames[i].IsStopFrame())
        {
            // remember first stop frame
            if (firstStop == -1)
                firstStop = i;
            lastStop = i;
        }
        else if (keyFrames[i].node->actionFlag == 1)
            closed = false;
    }

    if (closed)
        keyFrames[0].distFromPrev =
            sqrt(pow(keyFrames[0].node->x - keyFrames.back().node->x, 2) +
                 pow(keyFrames[0].node->y - keyFrames.back().node->y, 2) +
                 pow(keyFrames[0].node->z - keyFrames.back().node->z, 2));

    // at stopping keyframes, we define distSinceStop == 0,
    // and distUntilStop is to the next stopping keyframe.
    // this is required to properly handle cases of two stopping frames in a row (yes they do exist)
    float tmpDist = 0.0f;
    for (size_t i = 0; i < keyFrames.size(); ++i)
    {
        int32 j = (i + lastStop) % keyFrames.size();
        if (keyFrames[j].IsStopFrame())
            tmpDist = 0.0f;
        else
            tmpDist += keyFrames[j].distFromPrev;
        keyFrames[j].distSinceStop = tmpDist;
    }

    tmpDist = 0.0f;
    for (int32 i = int32(keyFrames.size()) - 1; i >= 0; i--)
    {
        int32 j = (i + firstStop) % keyFrames.size();
        tmpDist += keyFrames[(j + 1) % keyFrames.size()].distFromPrev;
        keyFrames[j].distUntilStop = tmpDist;
        if (keyFrames[j].IsStopFrame())
            tmpDist = 0.0f;
    }

    for (size_t i = 0; i < keyFrames.size(); ++i)
    {
        /*if (keyFrames[i].distSinceStop < accel_dist)
            keyFrames[i].timeFrom = sqrt(2 * keyFrames[i].distSinceStop / accel);
        else
            //keyFrames[i].timeFrom = ((keyFrames[i].distSinceStop - (30 * 30 * 0.5f)) / 30) + 30;
            // t = (constant speed time) + (0.5 * time until full speed)
            keyFrames[i].timeFrom = (keyFrames[i].distSinceStop / speed) + (0.5f * speed / accel); */

        float total_dist = keyFrames[i].distSinceStop + keyFrames[i].distUntilStop;
        if (total_dist < 2 * accel_dist) // won't reach full speed
        {
            if (keyFrames[i].distSinceStop < keyFrames[i].distUntilStop) // is still accelerating
            {
                // calculate accel+brake time for this short segment
                float segment_time = 2.0f * sqrt((keyFrames[i].distUntilStop + keyFrames[i].distSinceStop) / accel);
                // substract acceleration time
                keyFrames[i].timeTo = segment_time - sqrt(2 * keyFrames[i].distSinceStop / accel);
            }
            else // slowing down
                keyFrames[i].timeTo = sqrt(2 * keyFrames[i].distUntilStop / accel);
        }
        else if (keyFrames[i].distSinceStop < accel_dist) // still accelerating (but will reach full speed)
        {
            // calculate accel + cruise + brake time for this long segment
            float segment_time = (keyFrames[i].distUntilStop + keyFrames[i].distSinceStop) / speed + (speed / accel);
            // substract acceleration time
            keyFrames[i].timeTo = segment_time - sqrt(2 * keyFrames[i].distSinceStop / accel);
        }
        else if (keyFrames[i].distUntilStop < accel_dist) // already slowing down (but reached full speed)
            keyFrames[i].timeTo = sqrt(2 * keyFrames[i].distUntilStop / accel);
        else // at full speed
            keyFrames[i].timeTo = (keyFrames[i].distUntilStop / speed) + (0.5f * speed / accel);
    }

    // calculate tFrom times from tTo times
    float segmentTime = 0.0f;
    for (size_t i = 0; i < keyFrames.size(); ++i)
    {
        int32 j = (i + lastStop) % keyFrames.size();
        if (keyFrames[j].IsStopFrame())
            segmentTime = keyFrames[j].timeTo;
        keyFrames[j].timeFrom = segmentTime - keyFrames[j].timeTo;
    }

    // calculate path times
    keyFrames[0].pathTime = 0;
    float curPathTime = 0.0f;
    if (keyFrames[0].IsStopFrame())
    {
        curPathTime = float(keyFrames[0].node->delay);
        keyFrames[0].departureTime = uint32(curPathTime * IN_MILLISECONDS);
    }

    for (size_t i = 1; i < keyFrames.size(); ++i)
    {
        curPathTime += keyFrames[i-1].timeTo;
        if (keyFrames[i].IsStopFrame())
        {
            keyFrames[i].pathTime = uint32(curPathTime * IN_MILLISECONDS);
            curPathTime += (float)keyFrames[i].node->delay;
            keyFrames[i].departureTime = uint32(curPathTime * IN_MILLISECONDS);
        }
        else
        {
            curPathTime -= keyFrames[i].timeTo;
            keyFrames[i].pathTime = uint32(curPathTime * IN_MILLISECONDS);
            keyFrames[i].departureTime = keyFrames[i].pathTime;
        }
    }

    //sLog->outString("sinceStop | untilStop | fromPrev  |  tFrom  |  tTo    | stop | pathTime");
    //for (int i = 0; i < keyFrames.size(); ++i)
    //{
    //    sLog->outString("%9.3f | %9.3f | %9.3f | %7.3f | %7.3f | %u    | %u",
    //        keyFrames[i].distSinceStop, keyFrames[i].distUntilStop, keyFrames[i].distFromPrev, keyFrames[i].timeFrom, keyFrames[i].timeTo,
    //        keyFrames[i].IsStopFrame(), keyFrames[i].pathTime);
    //}

    transport->pathTime = keyFrames.back().departureTime;
    //if (keyFrames.back().IsStopFrame())
    //    transport->pathTime += keyFrames.back().node->delay * IN_MILLISECONDS;
}

Transport* TransportMgr::CreateTransport(uint32 entry, Map* map /*= NULL*/)
{
    // instance case, execute GetGameObjectEntry hook
    if (map)
    {
        // SetZoneScript() is called after adding to map, so fetch the script using map
        if (map->IsDungeon())
            if (InstanceScript* instance = static_cast<InstanceMap*>(map)->GetInstanceScript())
                entry = instance->GetGameObjectEntry(0, entry);

        if (!entry)
            return NULL;
    }

    TransportTemplate const* tInfo = GetTransportTemplate(entry);
    if (!tInfo)
    {
        sLog->outErrorDb("Transport %u will not be loaded, `transport_template` missing", entry);
        return NULL;
    }

    // create transport...
    Transport* trans = new Transport();

    // ...at first waypoint
    TaxiPathNodeEntry const* startNode = tInfo->keyFrames.begin()->node;
    uint32 mapId = startNode->mapid;
    float x = startNode->x;
    float y = startNode->y;
    float z = startNode->z;
    float o = 1.0f;

    // initialize the gameobject base
    if (!trans->Create(sObjectMgr->GenerateLowGuid(HIGHGUID_MO_TRANSPORT), entry, mapId, x, y, z, o, 100, 0))
    {
        delete trans;
        return NULL;
    }

    if (MapEntry const* mapEntry = sMapStore.LookupEntry(mapId))
    {
        if (uint32(mapEntry->Instanceable()) != trans->GetGOInfo()->moTransport.inInstance)
        {
            sLog->outError("Transport %u (name: %s) attempted creation in instance map (id: %u) but it is not an instanced transport!", entry, trans->GetName(), mapId);
            delete trans;
            return NULL;
        }
    }

    // use preset map for instances (need to know which instance)
    trans->SetMap(map ? map : sMapMgr->CreateMap(mapId, trans, 0));
    trans->SetZoneScript();

    // Get all spawns on Transport map
    if (uint32 mapId = trans->GetGOInfo()->moTransport.mapID)
    {
        CellObjectGuidsMap const& cells = sObjectMgr->GetMapObjectGuids(mapId, REGULAR_DIFFICULTY);
        CellGuidSet::const_iterator guidEnd;
        for (CellObjectGuidsMap::const_iterator cellItr = cells.begin(); cellItr != cells.end(); ++cellItr)
        {
            // Creatures on transport
            guidEnd = cellItr->second.creatures.end();
            for (CellGuidSet::const_iterator guidItr = cellItr->second.creatures.begin(); guidItr != guidEnd; ++guidItr)
            {
                CreatureData const* data = sObjectMgr->GetCreatureData(*guidItr);
                trans->CreateNPCPassenger(data->id, data->posX, data->posY, data->posZ, data->orientation, data);
            }

            // GameObjects on transport
        }
    }

    // register in container for updates
    _transportUpdates.insert(trans);

    // and in container designed for quicker access by mapId
    for (std::set<uint32>::const_iterator i = tInfo->mapsUsed.begin(); i != tInfo->mapsUsed.end(); ++i)
        _transportMap[*i].insert(trans);

    trans->AddToWorld();
    if (map)
        trans->UpdateForMap(map);

    return trans;
}

void TransportMgr::SpawnContinentTransports()
{
    if (_transportTemplates.empty())
        return;

    uint32 oldMSTime = getMSTime();

    uint32 count = 0;
    for (TransportTemplates::const_iterator itr = _transportTemplates.begin(); itr != _transportTemplates.end(); ++itr)
        // we can safely do this, verified on startup
        if (!sObjectMgr->GetGameObjectTemplate(itr->first)->moTransport.inInstance)
            if (CreateTransport(itr->first))
                ++count;

    sLog->outString(">> Spawned %u continent transports in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    sLog->outString();
}

void TransportMgr::CreateInstanceTransports(Map* map)
{
    TransportInstanceMap::const_iterator mapTransports = _instanceTransports.find(map->GetId());

    // no transports here
    if (mapTransports == _instanceTransports.end() || mapTransports->second.empty())
        return;

    // create transports
    for (std::set<uint32>::const_iterator itr = mapTransports->second.begin(); itr != mapTransports->second.end(); ++itr)
        CreateTransport(*itr, map);
}

void TransportMgr::Update(uint32 diff)
{
    for (TransportSet::iterator itr = _transportUpdates.begin(); itr != _transportUpdates.end(); ++itr)
        (*itr)->Update(diff);
}

void TransportMgr::RemoveTransport(Transport* transport)
{
    for (TransportSet::iterator itr = _transportUpdates.begin(); itr != _transportUpdates.end();)
    {
        if ((*itr) == transport)
        {
            _transportUpdates.erase(itr++);
            break;
        }
        else
            ++itr;
    }

    // instanced transports can only exist on one map, no teleporting across maps
    uint32 mapId = transport->GetMapId();
    for (TransportSet::iterator itr = _transportMap[mapId].begin(); itr != _transportMap[mapId].end();)
    {
        if ((*itr) == transport)
        {
            _transportMap[mapId].erase(itr++);

            // delete the transport, this function is called only on instance destruction
            (*itr)->RemoveFromWorld();
            (*itr)->ResetMap();
            delete *itr;

            if (_transportMap[mapId].empty())
                _transportMap.erase(mapId);
            break;
        }
        else
            ++itr;
    }
}

void TransportMgr::RemoveTransportsFromMap(Map* map)
{
    for (TransportSet::iterator itr = _transportUpdates.begin(); itr != _transportUpdates.end();)
    {
        if ((*itr)->GetMap() == map)
            _transportUpdates.erase(itr++);
        else
            ++itr;
    }

    // instanced transports can only exist on one map, no teleporting across maps
    for (TransportSet::iterator itr = _transportMap[map->GetId()].begin(); itr != _transportMap[map->GetId()].end();)
    {
        if ((*itr)->GetMap() == map)
        {
            _transportMap[map->GetId()].erase(itr++);

            // delete the transport, this function is called only on instance destruction
            (*itr)->RemoveFromWorld();
            (*itr)->ResetMap();
            delete *itr;

            if (_transportMap[map->GetId()].empty())
            {
                _transportMap.erase(map->GetId());
                break;
            }
        }
        else
            ++itr;
    }
}

bool TransportMgr::GetTransportsForMap(std::vector<Transport*>& transports, uint32 mapId, uint32 instanceId /*= 0*/) const
{
    TransportMap::const_iterator itr = _transportMap.find(mapId);

    // check if there are any trasports for our map
    if (itr == _transportMap.end())
         return false;

    // fill vector with transports for our instance
    for (TransportSet::const_iterator iter = itr->second.begin(); iter != itr->second.end(); ++iter)
    {
        if ((*iter)->GetMapId() != mapId)
            continue;

        if (instanceId && (*iter)->GetInstanceId() != instanceId)
            continue;

        transports.push_back(*iter);
    }

    return !transports.empty();
}

Transport* TransportMgr::GetTransport(uint64 guid) const
{
    for (TransportSet::const_iterator itr = _transportUpdates.begin(); itr != _transportUpdates.end(); ++itr)
        if ((*itr)->GetGUID() == guid)
            return *itr;

    return NULL;
}
