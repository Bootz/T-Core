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

#ifndef _PATHMANAGER_H
#define _PATHMANAGER_H

#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"

//  move map related classes
namespace MMAP
{
    typedef UNORDERED_MAP<uint32, dtTileRef> MapTileSet;
    typedef UNORDERED_MAP<uint32, dtNavMeshQuery*> NavMeshQuerySet;

    // dummy struct to hold maps data
    struct MapData
    {
        MapData(dtNavMesh* mesh) : navMesh(mesh) {}
        ~MapData()
        {
            for (NavMeshQuerySet::iterator i = navMeshQueries.begin(); i != navMeshQueries.end(); ++i)
                dtFreeNavMeshQuery(i->second);

            if (navMesh)
                dtFreeNavMesh(navMesh);
        }

        dtNavMesh* navMesh;

        // we have to use single dtNavMeshQuery for every instance, since those are not thread safe
        NavMeshQuerySet navMeshQueries;     // instanceId to query
        MapTileSet mapLoadedTiles;        // maps [map grid coords] to [dtTile]
    };

    typedef UNORDERED_MAP<uint32, MapData*> MapDataSet;

    // singelton class
    // holds all all access to mmap loading unloading and meshes
    class PathManager
    {
        public:
            PathManager() : loadedTiles(0) {}
            ~PathManager();

            bool loadMap(uint32 mapId, int32 x, int32 y);
            bool unloadMap(uint32 mapId, int32 x, int32 y);
            bool unloadMap(uint32 mapId);
            bool unloadMapInstance(uint32 mapId, uint32 instanceId);

            // the returned [dtNavMeshQuery const*] is NOT threadsafe
            dtNavMeshQuery const* GetNavMeshQuery(uint32 mapId, uint32 instanceId);
            dtNavMesh const* GetNavMesh(uint32 mapId);

            uint32 getLoadedTilesCount() const { return loadedTiles; }
            uint32 getLoadedMapsCount() const { return loadedMaps.size(); }
        private:
            bool loadMapData(uint32 mapId);
            uint32 packTileID(int32 x, int32 y);

            MapDataSet loadedMaps;
            uint32 loadedTiles;
    };
}

#endif