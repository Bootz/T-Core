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

#ifndef _QUADMAP_H
#define _QUADMAP_H

#include <ace/Array_Map.h>
#include <map>

class Player;
class WorldObject;

#define GRID_SIZE_LIMIT 32
#define MAX_AMMOUNT_OBJECT_PER_NODE 32
#define PHASE_COUNT  17

enum eQuadtreeCorner
{
    QUADTREE_UPPER_LEFT_CORNER = 0,
    QUADTREE_BOTTOM_LEFT_CORNER,
    QUADTREE_UPPER_RIGHT_CORNER,
    QUADTREE_BOTTOM_RIGHT_CORNER,
    QUADTREE_MAX
};

typedef ACE_Array_Map<uint64, WorldObject*> cont;

enum Type
{
    TYPE_OBJECT        = 0,
    TYPE_ITEM          = 1,
    TYPE_CONTAINER     = 2,
    TYPE_UNIT          = 3,
    TYPE_PLAYER        = 4,
    TYPE_GAMEOBJECT    = 5,
    TYPE_DYNAMICOBJECT = 6,
    TYPE_CORPSE        = 7,
    TYPE_MAX
};

class Quadmap
{
    public:
        const static size_t DEFAULT_SIZE = 0x40;

        Quadmap(float xMin,float xMax,float yMin,float yMax,Map* m, bool s);
        ~Quadmap();

        /// Add
        void AddObject (WorldObject* object);
        void AddInsideQuad(WorldObject* object);

        void ResetMark();
        void CheckObjectInRange(float range,WorldObject* source);

        void Remove (uint64 guid);

        /// Zone limit managed by this quad
        float m_XMin;
        float m_XMax;
        float m_YMin;
        float m_YMax;

        uint32 GetSize() { return Objects.size(); }
        uint32 FullSize();
        bool isMarked() const { return mark; }
        bool FindQuad(Quadmap *quad) const;
        bool DoWeHaveObject(WorldObject* object);

        int GetType(const WorldObject *object) const;
        void GetList(float x, float y, float radius, uint32 phase, std::list<WorldObject*> &l);

        template<class NOTIFIER> void Visit(float x, float y, float radius, uint32 phase, NOTIFIER &notifier);
    protected:
        ACE_Recursive_Thread_Mutex Lock;

        /// This contains all objects in the node
        cont Objects;
        /// sons
        Quadmap* sons[4];

        Map* map;

        /// Say if this quadmap contain static object
        bool staticquadmap;
        bool mark;
};

inline bool IsPointOnQuad(Quadmap* q, float x, float y)
{
    return (q->m_XMin <= x && q->m_XMax >= x && q->m_YMin <= y && q->m_YMax >= y);
}
static bool IsCrossing(const Quadmap* q, float x_min, float x_max, float y_min, float y_max)
{
    if (q->m_XMin > x_max || x_min > q->m_XMax)
        return false;

    if (q->m_YMin > y_max || y_min > q->m_YMax)
        return false;

    return true;
}
inline float GetExactDist2dSq(float x, float y, float x2, float y2)
{
    float dx = x2 - x;
    float dy = y2 - y;
    return dx*dx + dy*dy;
}
template<class NOTIFIER>
inline void Quadmap::Visit(float x, float y, float radius, uint32 phase, NOTIFIER &notifier)
{
    std::list<WorldObject*> l;
    GetList(x, y, radius, phase, l);  //generate target list
    for (std::list<WorldObject*>::iterator itr = l.begin(); itr!=l.end();++itr)
    {
        switch(GetType(*itr))
        {
            case TYPE_UNIT:
                notifier.VisitUnit(*itr);
                break;
            case TYPE_PLAYER:
                notifier.VisitPlayer(*itr);
                break;
            case TYPE_GAMEOBJECT:
                notifier.VisitGobject(*itr);
                break;
            case TYPE_DYNAMICOBJECT:
                notifier.VisitDobject(*itr);
                break;
            case TYPE_CORPSE:
                notifier.VisitCorpse(*itr);
                break;
            default:
                continue;
        }
    }
}
#endif