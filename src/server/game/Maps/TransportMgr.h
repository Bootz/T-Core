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

#ifndef TRANSPORTMGR_H
#define TRANSPORTMGR_H

#include <ace/Singleton.h>
#include "GameObject.h"

struct KeyFrame
{
    explicit KeyFrame(TaxiPathNodeEntry const& _node) : node(&_node),
        distSinceStop(-1.0f), distUntilStop(-1.0f), distFromPrev(-1.0f), timeFrom(0.0f), timeTo(0.0f),
        teleport(false), pathTime(0), departureTime(0)
    {
    }

    TaxiPathNodeEntry const* node;
    float distSinceStop;
    float distUntilStop;
    float distFromPrev;
    float timeFrom;
    float timeTo;
    bool teleport;
    uint32 pathTime;
    uint32 departureTime;

    bool IsTeleportFrame() const { return teleport; }
    bool IsStopFrame() const { return node->actionFlag == 2; }
};

typedef std::vector<KeyFrame> KeyFrameVec;
struct TransportTemplate
{
    TransportTemplate() : name(""), scriptId(0), pathTime(0), accelTime(0.0f), accelDist(0.0f) { }
    std::string name;
    uint32 scriptId;
    std::set<uint32> mapsUsed;
    uint32 pathTime;
    KeyFrameVec keyFrames;
    float accelTime;
    float accelDist;
};

typedef UNORDERED_MAP<uint32, TransportTemplate> TransportTemplates;
typedef std::set<Transport*>                     TransportSet;
typedef UNORDERED_MAP<uint32, TransportSet>      TransportMap;
typedef UNORDERED_MAP<uint32, std::set<uint32> > TransportInstanceMap;

struct GameObjectInfo;
class Transport;

class TransportMgr
{
        friend class ACE_Singleton<TransportMgr, ACE_Thread_Mutex>;

    public:
        void Unload();

        void LoadTransportTemplates();

        // Creates a transport using given GameObject template entry
        Transport* CreateTransport(uint32 entry, Map* map = NULL);

        // Spawns all continent transports, used at core startup
        void SpawnContinentTransports();

        // creates all transports for instance
        void CreateInstanceTransports(Map* map);

        void Update(uint32 diff);

        void RemoveTransport(Transport* transport);

        // used to remove AND delete no longer needed transports (instance only)
        void RemoveTransportsFromMap(Map* map);

        // Gets a list of transport for map/instance to send them to players
        // the reason we don't store this in grids is because we need to send transports to the entire map
        bool GetTransportsForMap(std::vector<Transport*>& transports, uint32 mapId, uint32 instanceId = 0) const;

        Transport* GetTransport(uint64 guid) const;

        TransportTemplate const* GetTransportTemplate(uint32 entry) const
        {
            TransportTemplates::const_iterator itr = _transportTemplates.find(entry);
            if (itr != _transportTemplates.end())
                return &itr->second;
            return NULL;
        }

    private:
        TransportMgr();
        ~TransportMgr();
        TransportMgr(TransportMgr const&);
        TransportMgr& operator=(TransportMgr const&);

        // Generates and precaches a path for transport to avoid generation each time transport instance is created
        void GeneratePath(GameObjectTemplate const* goInfo, TransportTemplate* transport);

        // Container storing transport templates
        TransportTemplates _transportTemplates;

        // Container for storing Transports to use them during map update
        TransportSet _transportUpdates;

        // Container storing Transports per map, used for sending transports
        TransportMap _transportMap;

        // Container storing transport entries to create for instanced maps
        TransportInstanceMap _instanceTransports;
};

#define sTransportMgr ACE_Singleton<TransportMgr, ACE_Thread_Mutex>::instance()

#endif // TRANSPORTMGR_H
