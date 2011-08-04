/*
 * Copyright (C) 2005-2011 MaNGOS <http://www.getmangos.com/>
 *
 * Copyright (C) 2008-2011 Trinity <http://www.trinitycore.org/>
 *
 * Copyright (C) 2006-2011 ScriptDev2 <http://www.scriptdev2.com/>
 *
 * Copyright (C) 2010-2011 Project SkyFire <http://www.projectskyfire.org/>
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

/* ScriptData
SDName: Gilneas City
SD%Complete: 
SDComment: 
SDCategory: Gilneas
EndScriptData */

/* ContentData
TODO
EndContentData */

#include "ScriptPCH.h"

enum GilneasCityPhase2
{
    SWORD_FLESH_SOUND = 143,
    SWORD_PLATE_SOUND = 147,
    SOUND_DELAY       = 500,
    ANIMATE_DELAY     = 2000,

    // NPC's
    PRINCE_LIAN_GREYMANE = 34913,
    GUARD                = 34916,
    LIEUTENANT_WALDEN    = 34836,
    WORGEN               = 34884,
    WORGEN2              = 35660,
    CITIZEN1             = 34981,
    CITIZEN2             = 35836
};

/*######
## npc_gilneas_city_guard
######*/

class npc_gilneas_city_guard : public CreatureScript
{
public:
    npc_gilneas_city_guard() : CreatureScript("npc_gilneas_city_guard") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_gilneas_city_guardAI (pCreature);
    }

    struct npc_gilneas_city_guardAI : public ScriptedAI
    {
        npc_gilneas_city_guardAI(Creature *c) : ScriptedAI(c) {}

		uint32 tAnimate;
		uint32 tSound;
		uint32 dmgCount;
		bool playSnd;

        void Reset()
        {
			tAnimate = ANIMATE_DELAY;
			dmgCount = 0;
			tSound = SOUND_DELAY;
			playSnd = false;
        }

		void DamageTaken(Unit * pWho, uint32 &uiDamage)
        {
            if (pWho->GetTypeId() == TYPEID_PLAYER)
            {
                me->getThreatManager().resetAllAggro();
                pWho->AddThreat(me, 100000.0f);
                me->AddThreat(pWho, 100000.0f);
                me->AI()->AttackStart(pWho);
                dmgCount = 0;
            }
            else if (pWho->isPet())
			{
				me->getThreatManager().resetAllAggro();
                me->AddThreat(pWho, 100000.0f);
				me->AI()->AttackStart(pWho);
				dmgCount = 0;
			}
        }

		void DamageDealt(Unit* target, uint32& damage, DamageEffectType damageType)
		{
			if (target->GetEntry() == WORGEN)
				dmgCount ++;
		}

        void UpdateAI(const uint32 diff)
        {
			if (me->isAlive() && !me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f))
				if (Creature* enemy = me->FindNearestCreature(WORGEN, 16.0f, true))
					me->AI()->AttackStart(enemy);

            if (!UpdateVictim())
                return;

			if (tSound <= diff)
			{
				me->PlayDistanceSound(SWORD_FLESH_SOUND);
				tSound = SOUND_DELAY;
				playSnd = false;
			}

			if (playSnd == true) tSound -= diff;

			if (dmgCount < 2)
				DoMeleeAttackIfReady();
			else if (me->getVictim()->GetTypeId() == TYPEID_PLAYER) dmgCount = 0;
			else if (me->getVictim()->isPet()) dmgCount = 0;
			else
			{
				if (tAnimate <= diff)
				{
					me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK1H);
					playSnd = true;
					tAnimate = ANIMATE_DELAY;
				}
				else
					tAnimate -= diff;
				
			}
        }
    };

};

/*######
## npc_prince_liam_greymane
######*/

class npc_prince_liam_greymane : public CreatureScript
{
public:
    npc_prince_liam_greymane() : CreatureScript("npc_prince_liam_greymane") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_prince_liam_greymaneAI (pCreature);
    }

    struct npc_prince_liam_greymaneAI : public ScriptedAI
    {
        npc_prince_liam_greymaneAI(Creature *c) : ScriptedAI(c) {}

		uint32 tAnimate;
		uint32 tSound;
		uint32 dmgCount;
		bool playSnd;

        void Reset()
        {
			tAnimate = ANIMATE_DELAY;
			dmgCount = 0;
			tSound = SOUND_DELAY;
			playSnd = false;
        }

		void sQuestReward(Player *pPlayer, const Quest *pQuest, uint32 data)
		{
			if (pQuest->GetQuestId() == 14098 && pPlayer->GetPhaseMask() == 2)
				pPlayer->SetAuraStack(59074, pPlayer, 1); //phaseshift
		}

		void DamageTaken(Unit * pWho, uint32 &uiDamage)
        {
            if (pWho->GetTypeId() == TYPEID_PLAYER)
            {
                me->getThreatManager().resetAllAggro();
                pWho->AddThreat(me, 100000.0f);
                me->AddThreat(pWho, 100000.0f);
                me->AI()->AttackStart(pWho);
                dmgCount = 0;
            }
            else if (pWho->isPet())
			{
				me->getThreatManager().resetAllAggro();
                me->AddThreat(pWho, 100000.0f);
				me->AI()->AttackStart(pWho);
				dmgCount = 0;
			}
        }

		void DamageDealt(Unit* target, uint32& damage, DamageEffectType damageType)
		{
			if (target->GetEntry() == WORGEN)
				dmgCount ++;
		}

        void UpdateAI(const uint32 diff)
        {
			if (!UpdateVictim())
                return;

			if (tSound <= diff)
			{
				me->PlayDistanceSound(SWORD_PLATE_SOUND);
				tSound = SOUND_DELAY;
				playSnd = false;
			}

			if (playSnd == true) tSound -= diff;

			if (dmgCount < 2)
				DoMeleeAttackIfReady();
			else if (me->getVictim()->GetTypeId() == TYPEID_PLAYER) dmgCount = 0;
			else if (me->getVictim()->isPet()) dmgCount = 0;
			else
			{
				if (tAnimate <= diff)
				{
					me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK1H);
					playSnd = true;
					tAnimate = ANIMATE_DELAY;
				}
				else
					tAnimate -= diff;
				
			}
        }
    };

};

/*######
## npc_rampaging_worgen
######*/

enum eRampaging_worgen
{
	SPELL_ENRAGE = 8599,
	ENRAGE_CD    = 30000,
};

class npc_rampaging_worgen : public CreatureScript
{
public:
    npc_rampaging_worgen() : CreatureScript("npc_rampaging_worgen") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_rampaging_worgenAI (pCreature);
    }

    struct npc_rampaging_worgenAI : public ScriptedAI
    {
        npc_rampaging_worgenAI(Creature *c) : ScriptedAI(c) {}

		uint32 tEnrage;
		uint32 dmgCount;
		uint32 tAnimate;
		uint32 tSound;
		bool playSound;

        void Reset()
        {
			tEnrage = 0;
			dmgCount = 0;
			tAnimate = ANIMATE_DELAY;
			tSound = SOUND_DELAY;
			playSound = false;
        }

		void DamageDealt(Unit* target, uint32& damage, DamageEffectType damageType)
		{
			if (target->GetEntry() == GUARD || target->GetEntry() == PRINCE_LIAN_GREYMANE)
				dmgCount ++;
		}

		void DamageTaken(Unit * pWho, uint32 &uiDamage)
        {
            if (pWho->GetTypeId() == TYPEID_PLAYER)
            {
                me->getThreatManager().resetAllAggro();
                pWho->AddThreat(me, 100000.0f);
                me->AddThreat(pWho, 100000.0f);
                me->AI()->AttackStart(pWho);
                dmgCount = 0;
            }
            else if (pWho->isPet())
			{
				me->getThreatManager().resetAllAggro();
                me->AddThreat(pWho, 100000.0f);
				me->AI()->AttackStart(pWho);
				dmgCount = 0;
			}
        }

        void UpdateAI(const uint32 diff)
        {
			if (me->isAlive() && !me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f))
				if (Creature* enemy = me->FindNearestCreature(PRINCE_LIAN_GREYMANE, 10.0f, true))
					me->AI()->AttackStart(enemy);
					

			if (!UpdateVictim())
                return;

			if (tEnrage <= diff)
			{
				if (me->GetHealthPct() <= 30)
				{
					DoCast(me, SPELL_ENRAGE);
					tEnrage = ENRAGE_CD;
				}
			}
			else tEnrage -= diff;

			//play attack sound
			if (playSound == true) tSound -= diff;

			if (tSound <= diff)
			{
				me->PlayDistanceSound(SWORD_PLATE_SOUND);
				tSound = SOUND_DELAY;
				playSound = false;
			}
			
			if (dmgCount < 2)
				DoMeleeAttackIfReady();
			else if (me->getVictim()->GetTypeId() == TYPEID_PLAYER) dmgCount = 0;
			else if (me->getVictim()->isPet()) dmgCount = 0;
			else
			{
				if (tAnimate <= diff)
				{
					me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACKUNARMED);
					tAnimate = ANIMATE_DELAY;
					playSound = true;
				}
				else
				tAnimate -= diff;
			}
			
        }
    };

};

/*######
## go_merchant_square_door
######*/

enum eMerchant_square_door
{
    SUMMON_TTL = 30000,
    QUEST_EVAC_MERCH = 14098,
};

class go_merchant_square_door : public GameObjectScript
{
public:
    go_merchant_square_door() : GameObjectScript("go_merchant_square_door") { }

    uint32 creatureID;
    uint8 SpawnID;
    float x, y, z, angle;

    bool OnGossipHello(Player *pPlayer, GameObject *pGO)
    {
        pGO->Use(pPlayer);

        SpawnID = urand(1, 3);
        switch (SpawnID)
        {
            case 1:
                creatureID = WORGEN2;
                break;
            case 2:
                creatureID = CITIZEN1;
                break;
            case 3:
                creatureID = CITIZEN2;
                break;
        }

        angle = pGO->GetOrientation();
        x = pGO->GetPositionX() - cos(angle) * 2;
        y = pGO->GetPositionY() - sin(angle) * 2;
        z = pGO->GetPositionZ();	
		
        if (Creature *spawnedCreature = pGO->SummonCreature(creatureID, x, y, z, angle, TEMPSUMMON_TIMED_DESPAWN, SUMMON_TTL))
        {
            spawnedCreature->SetPhaseMask(2, 1);
            if (creatureID == WORGEN2)
            {
                spawnedCreature->getThreatManager().resetAllAggro();
                pPlayer->AddThreat(spawnedCreature, 100000.0f);
                spawnedCreature->AddThreat(pPlayer, 100000.0f);
                spawnedCreature->AI()->AttackStart(pPlayer);
            }
            else if (pPlayer->GetQuestStatus(QUEST_EVAC_MERCH) == QUEST_STATUS_INCOMPLETE)
            {
                pPlayer->KilledMonsterCredit(35830, 0);
                spawnedCreature->Respawn(1);
            }
        }
			
        return false;
    }
};

/*######
## npc_lieutenant_walden
######*/

enum eLieutenant_walden
{
    QUEST_LOCKDOWN = 14078,
    SPELL_PHASE2   = 58073,
};

class npc_lieutenant_walden : public CreatureScript
{
public:
    npc_lieutenant_walden() : CreatureScript("npc_lieutenant_walden") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_lieutenant_waldenAI (pCreature);
    }

    struct npc_lieutenant_waldenAI : public ScriptedAI
    {
        
		npc_lieutenant_waldenAI(Creature *c) : ScriptedAI(c) {}
		
		void sQuestReward(Player *pPlayer, const Quest *pQuest, uint32 data)
		{
			if (pQuest->GetQuestId() == QUEST_LOCKDOWN && pPlayer->GetPhaseMask() == 1)
				pPlayer->SetAuraStack(SPELL_PHASE2, pPlayer, 1); //phaseshift
		}
	};

};

/*######
## npc_frightened_citizen
######*/

#define CITIZEN_SAY_1  "Worgen! Worgen everywhere!"
#define CITIZEN_SAY_2  "No time to Waste!"
#define CITIZEN_SAY_3  "This place isn't safe. Let's Leave!"

enum eFrightened_citizen
{
    RUN_DELAY = 3000,
    CITIZEN_DESPAWN = 5000,
};

class npc_frightened_citizen : public CreatureScript
{
public:
    npc_frightened_citizen() : CreatureScript("npc_frightened_citizen") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_frightened_citizenAI (pCreature);
    }

    struct npc_frightened_citizenAI : public ScriptedAI
    {
        npc_frightened_citizenAI(Creature *c) : ScriptedAI(c) {}

		int16 delay, despawn;
		bool run, onceTimer;
		uint8 sayID;
		char* text;
		float x, y, z;

        void JustRespawned()
        {
            delay = RUN_DELAY;
            run = false;
            despawn = CITIZEN_DESPAWN;
            onceTimer = true;
            x = me->m_positionX+cos(me->m_orientation)*16;
            y = me->m_positionY+sin(me->m_orientation)*16;
            z = me->m_positionZ;

            sayID = urand(1,3);
            switch (sayID)
            {
                case 1:
                    text = CITIZEN_SAY_1;
                break;
                case 2:
                    text = CITIZEN_SAY_2;
                    break;
                case 3:
                    text = CITIZEN_SAY_3;
                break;
            }

            me->MonsterSay(text, 0, NULL);
        }

		void UpdateAI(const uint32 diff)
        {
			if (delay <= 0 && onceTimer == true)
			{
				run = true;
				onceTimer = false;
			}
			else delay -= diff;

			if (run == true)
			{
				run = false;
				me->GetMotionMaster()->MoveCharge(x, y, z, 8);
			}

			if (despawn <= 0)
			{
				me->DespawnOrUnsummon();
			}
			else despawn -= diff;

		}
    };

};

void AddSC_gilneas()
{
	new npc_gilneas_city_guard();
	new npc_prince_liam_greymane();
    new npc_rampaging_worgen();
	new go_merchant_square_door();
	new npc_lieutenant_walden();
	new npc_frightened_citizen();
}