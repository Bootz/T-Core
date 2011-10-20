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
#include "bastion_of_twilight.h"

class boss_feludius : public CreatureScript
{
    public:
        boss_feludius() : CreatureScript("boss_feludius") { }

        struct boss_feludiusAI : BossAI
        {
            boss_feludiusAI(Creature * pCreature) : BossAI(pCreature,DATA_FELUDIUS), summons(me)
            {
                pInstance = (InstanceScript*)pCreature->GetInstanceScript();
            }

            void Reset()
            {
                uiHydrolanceTimer = 11000;
                uiWaterBombTimer = 16000;
                uiHeartofIceTimer = 39000;
                uiGlaciateTimer = 32000;
            }

            void UpdateAI(const uint32 uiDiff)
            {
                if(uiHydrolanceTimer <= uiDiff)
                {
                    uiHydrolanceTimer = 11000;
                    Unit * Victim = SelectTarget(SELECT_TARGET_RANDOM);
                    DoCast(Victim,SPELL_HYDROLANCE);
                } else uiHydrolanceTimer -= uiDiff;
                if(uiWaterBombTimer <= uiDiff)
                {
                    uiHydrolanceTimer = 11000;
                    DoCast(SPELL_WATERBOMB);
                } else uiWaterBombTimer -= uiDiff;
                if(uiHeartofIceTimer <= uiDiff)
                {
                    uiHeartofIceTimer = 39000;
                    Unit * Victim = SelectTarget(SELECT_TARGET_RANDOM);
                    DoCast(Victim,SPELL_HEART_OF_ICE);
                } else uiHeartofIceTimer -= uiDiff;
                if(uiGlaciateTimer <= uiDiff)
                {
                    uiGlaciateTimer = 32000;
                    DoCast(SPELL_GLACIATE);
                } else uiGlaciateTimer -= uiDiff;
            }

        private:
            InstanceScript* pInstance;
            SummonList summons;

            uint32 uiHydrolanceTimer;
            uint32 uiWaterBombTimer;
            uint32 uiHeartofIceTimer;
            uint32 uiGlaciateTimer;
        };

        CreatureAI * GetAI(Creature* pCreature) const
        {
            return new boss_feludiusAI(pCreature);
        }
};


void AddSC_boss_ascendant_council()
{
    new boss_feludius();
}