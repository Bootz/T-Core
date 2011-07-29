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

#include "ScriptPCH.h"
#include "ruins_of_ahnqiraj.h"

enum Yells
{
    EMOTE_TARGET               = -1509002
};

class boss_buru : public CreatureScript
{
public:
    boss_buru() : CreatureScript("boss_buru") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_buruAI (creature);
    }

    struct boss_buruAI : public ScriptedAI
    {
        boss_buruAI(Creature *c) : ScriptedAI(c)
        {
            pInstance = c->GetInstanceScript();
        }

        InstanceScript *pInstance;

        void Reset()
        {
            if (pInstance)
                pInstance->SetData(DATA_BURU_EVENT, NOT_STARTED);
        }

        void EnterCombat(Unit* /*who*/)
        {
            if (pInstance)
                pInstance->SetData(DATA_BURU_EVENT, IN_PROGRESS);
        }

        void JustDied(Unit* /*killer*/)
        {
            if (pInstance)
                pInstance->SetData(DATA_BURU_EVENT, DONE);
        }
    };

};

void AddSC_boss_buru()
{
    new boss_buru();
}
