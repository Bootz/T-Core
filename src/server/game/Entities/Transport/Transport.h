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

#ifndef TRANSPORTS_H
#define TRANSPORTS_H

#include "GameObject.h"
#include "TransportMgr.h"

class Transport : public GameObject
{
        friend Transport* TransportMgr::CreateTransport(uint32 entry, Map* map /*= NULL*/);
        Transport();
    public:
        ~Transport();

        typedef std::set<Unit*> UnitSet;

        bool Create(uint32 guidlow, uint32 entry, uint32 mapid, float x, float y, float z, float ang, uint32 animprogress, uint32 dynflags);
        Creature* CreateNPCPassenger(uint32 entry, float x, float y, float z, float o, CreatureData const* data = NULL);

        void Update(uint32 diff);

        void AddPassenger(Unit* passenger);
        void RemovePassenger(Unit* passenger);
        UnitSet const& GetPassengers() const { return _passengers; }

        TaxiPathNodeEntry const* GetCurrentPathNode() { return _currentFrame->node; }
        uint32 GetScriptId() const { return _transportInfo->scriptId; }

        void BuildStartMovePacket(Map const* targetMap);
        void BuildStopMovePacket(Map const* targetMap);
        bool IsStopped() const { return _isStopped; }
        void SetStopped(bool val) { _isStopped = val; }

    private:
        KeyFrameVec const& GetKeyFrames() const { return _transportInfo->keyFrames; }
        void MoveToNextWayPoint();
        void TeleportTransport(uint32 newMapid, float x, float y, float z);
        float CalculateSegmentPos(float now);
        void UpdatePassengerPositions();
        void UpdateForMap(Map const* map);
        bool IsMoving() const { return _isMoving; }
        void SetMoving(bool val) { _isMoving = val; }
        void DoEventIfAny(KeyFrame const& node, bool departure);

        TransportTemplate const* _transportInfo;
        UnitSet _passengers;
        KeyFrameVec::const_iterator _currentFrame;
        KeyFrameVec::const_iterator _nextFrame;
        uint32 _moveTimer;
        bool _isMoving;
        bool _isStopped;
};

#endif

