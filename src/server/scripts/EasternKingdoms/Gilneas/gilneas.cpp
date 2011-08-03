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
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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

enum eGilneas_city_phase2
{
	#define sword_flesh_sound 143
	#define sword_plate_sound 147
	#define sound_delay 500
	#define animate_delay 2000
	//npcs
	#define prince_liam_greymane 34913
	#define guard 34916
	#define lieutenant_walden 34863
	#define worgen 34884
	#define worgen2 35660
	#define citizen1 34981
	#define citizen2 35836
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
			tAnimate = animate_delay;
			dmgCount = 0;
			tSound = sound_delay;
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
			if (target->GetEntry() == worgen)
				dmgCount ++;
		}

        void UpdateAI(const uint32 diff)
        {
			if (me->isAlive() && !me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f))
				if (Creature* enemy = me->FindNearestCreature(worgen, 16.0f, true))
					me->AI()->AttackStart(enemy);

            if (!UpdateVictim())
                return;

			if (tSound <= diff)
			{
				me->PlayDistanceSound(sword_flesh_sound);
				tSound = sound_delay;
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
					tAnimate = animate_delay;
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
			tAnimate = animate_delay;
			dmgCount = 0;
			tSound = sound_delay;
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
			if (target->GetEntry() == worgen)
				dmgCount ++;
		}

        void UpdateAI(const uint32 diff)
        {
			if (!UpdateVictim())
                return;

			if (tSound <= diff)
			{
				me->PlayDistanceSound(sword_flesh_sound);
				tSound = sound_delay;
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
					tAnimate = animate_delay;
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
	#define spell_enrage 8599
	#define enrage_cd 30000
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
			tAnimate = animate_delay;
			tSound = sound_delay;
			playSound = false;
        }

		void DamageDealt(Unit* target, uint32& damage, DamageEffectType damageType)
		{
			if (target->GetEntry() == guard || target->GetEntry() == prince_liam_greymane)
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
				if (Creature* enemy = me->FindNearestCreature(prince_liam_greymane, 10.0f, true))
					me->AI()->AttackStart(enemy);
					

			if (!UpdateVictim())
                return;

			if (tEnrage <= diff)
			{
				if (me->GetHealthPct() <= 30)
				{
					DoCast(me, spell_enrage);
					tEnrage = enrage_cd;
				}
			}
			else tEnrage -= diff;

			//play attack sound
			if (playSound == true) tSound -= diff;

			if (tSound <= diff)
			{
				me->PlayDistanceSound(sword_plate_sound);
				tSound = sound_delay;
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
					tAnimate = animate_delay;
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
	#define summon1_ttl 30000
	#define q_evac_merch_sq 14098
};

class go_merchant_square_door : public GameObjectScript
{
public:
    go_merchant_square_door() : GameObjectScript("go_merchant_square_door") { }

	uint32 creatureID;
	uint8 spawnID;
	float x, y, z, angle;

	bool OnGossipHello(Player *pPlayer, GameObject *pGO)
    {
		pGO->Use(pPlayer);
		spawnID=urand(1, 3);

		if (spawnID == 1) creatureID = worgen2;
		if (spawnID == 2) creatureID = citizen1;
		if (spawnID == 3) creatureID = citizen2;

		angle=pGO->GetOrientation();
		x=pGO->GetPositionX()-cos(angle)*2;
		y=pGO->GetPositionY()-sin(angle)*2;
		z=pGO->GetPositionZ();
		
		
		if (Creature *spawnedCreature = pGO->SummonCreature(creatureID,x,y,z,angle,TEMPSUMMON_TIMED_DESPAWN,summon1_ttl))
		{
			spawnedCreature->SetPhaseMask(2, 1);
			if (creatureID == worgen2)
			{
				spawnedCreature->getThreatManager().resetAllAggro();
				pPlayer->AddThreat(spawnedCreature, 100000.0f);
				spawnedCreature->AddThreat(pPlayer, 100000.0f);
				spawnedCreature->AI()->AttackStart(pPlayer);
			}
			else if (pPlayer->GetQuestStatus(q_evac_merch_sq) == QUEST_STATUS_INCOMPLETE)
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
	#define q_lockdown 14078
	#define spell_phase2 59073
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
			if (pQuest->GetQuestId() == q_lockdown && pPlayer->GetPhaseMask() == 1)
				pPlayer->SetAuraStack(spell_phase2, pPlayer, 1); //phaseshift
		}
	};

};

/*######
## npc_frightened_citizen
######*/

enum eFrightened_citizen
{
	#define run_delay 3000
	#define citizen_despawn 5000
	#define citizen_say_1 "Worgen! Worgen everywhere!"
	#define citizen_say_2 "No time to Waste!"
	#define citizen_say_3 "This place isn't safe. Let's Leave!"
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
			delay = run_delay;
			run = false;
			despawn = citizen_despawn;
			onceTimer = true;
			x = me->m_positionX+cos(me->m_orientation)*16;
			y = me->m_positionY+sin(me->m_orientation)*16;
			z = me->m_positionZ;
			
			sayID = urand(1,3);
			if (sayID == 1) text = citizen_say_1;
			if (sayID == 2) text = citizen_say_2;
			if (sayID == 3) text = citizen_say_3;
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