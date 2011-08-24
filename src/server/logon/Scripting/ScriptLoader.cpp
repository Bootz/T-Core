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

#include "ScriptLoader.h"

//examples
void AddSC_example_commandscript();

//Commands
void AddSC_account_commandscript();
void AddSC_achievement_commandscript();
void AddSC_event_commandscript();
void AddSC_gm_commandscript();
void AddSC_go_commandscript();
void AddSC_honor_commandscript();
void AddSC_learn_commandscript();
void AddSC_quest_commandscript();
void AddSC_reload_commandscript();
void AddSC_tele_commandscript();
void AddSC_titles_commandscript();
void AddSC_wp_commandscript();
void AddSC_gps_commandscript();

#ifdef SCRIPTS

// player
void AddSC_chat_log();

#endif

void AddScripts()
{
    AddExampleScripts();
    AddCommandScripts();
#ifdef SCRIPTS

#endif
}

void AddExampleScripts()
{
    AddSC_example_commandscript();
}

void AddCommandScripts()
{
    AddSC_account_commandscript();
    AddSC_achievement_commandscript();
    AddSC_event_commandscript();
    AddSC_gm_commandscript();
    AddSC_go_commandscript();
    AddSC_honor_commandscript();
    AddSC_learn_commandscript();
    AddSC_quest_commandscript();
    AddSC_reload_commandscript();
    AddSC_tele_commandscript();
    AddSC_titles_commandscript();
    AddSC_wp_commandscript();
    AddSC_gps_commandscript();
}

#ifdef SCRIPTS
/* This is where custom scripts' loading functions should be declared. */

#endif

void AddCustomScripts()
{
#ifdef SCRIPTS
    /* This is where custom scripts should be added. */

#endif
}
