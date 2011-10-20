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

#include "WorldDatabase.h"

void WorldDatabaseConnection::DoPrepareStatements()
{
    if (!m_reconnecting)
        m_stmts.resize(MAX_WORLDDATABASE_STATEMENTS);

    PREPARE_STATEMENT(WORLD_LOAD_QUEST_POOLS, "SELECT entry, pool_entry FROM pool_quest", CONNECTION_SYNCH)
    PREPARE_STATEMENT(WORLD_DEL_CRELINKED_RESPAWN, "DELETE FROM linked_respawn WHERE guid = ?", CONNECTION_ASYNC)
    PREPARE_STATEMENT(WORLD_REP_CRELINKED_RESPAWN, "REPLACE INTO linked_respawn (guid, linkedGuid) VALUES (?, ?)", CONNECTION_ASYNC)
    PREPARE_STATEMENT(WORLD_LOAD_CRETEXT, "SELECT entry, groupid, id, text, type, language, probability, emote, duration, sound FROM creature_text", CONNECTION_SYNCH)
    PREPARE_STATEMENT(WORLD_LOAD_SMART_SCRIPTS,  "SELECT entryorguid, source_type, id, link, event_type, event_phase_mask, event_chance, event_flags, event_param1, event_param2, event_param3, event_param4, action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6, target_type, target_param1, target_param2, target_param3, target_x, target_y, target_z, target_o FROM smart_scripts ORDER BY entryorguid, source_type, id, link", CONNECTION_SYNCH)
    PREPARE_STATEMENT(WORLD_LOAD_SMARTAI_WP,  "SELECT entry, pointid, position_x, position_y, position_z FROM waypoints ORDER BY entry, pointid", CONNECTION_SYNCH)

    PREPARE_STATEMENT(WORLD_LOAD_CREATURE_GUIDS, "SELECT guid FROM creature", CONNECTION_SYNCH);

    PREPARE_STATEMENT(WORLD_LOAD_CREFORMATIONS_MAXID, "SELECT MAX(formationId) FROM creature_formations", CONNECTION_SYNCH)
    PREPARE_STATEMENT(WORLD_LOAD_CREFORMATIONS_WITHOUT_DATA, "SELECT COUNT(formationId) FROM creature_formations WHERE formationId NOT IN (SELECT formationId FROM creature_formation_data)", CONNECTION_SYNCH)
    PREPARE_STATEMENT(WORLD_LOAD_CREFORMATION_DATA_WITHOUT_FORMATION, "SELECT COUNT(formationId) FROM creature_formation_data WHERE formationId NOT IN (SELECT formationId FROM creature_formations)", CONNECTION_SYNCH)
    PREPARE_STATEMENT(WORLD_LOAD_CREFORMATIONS, "SELECT formationId, leaderGUID, formationAI FROM creature_formations WHERE formationId IN (SELECT formationId FROM creature_formation_data) ORDER BY formationId", CONNECTION_SYNCH)
    PREPARE_STATEMENT(WORLD_LOAD_CREFORMATION_DATA, "SELECT formationId, memberGUID, dist, angle FROM creature_formation_data WHERE formationId IN (SELECT formationId FROM creature_formations) ORDER BY formationId", CONNECTION_SYNCH)
    PREPARE_STATEMENT(WORLD_LOAD_CREFORMATIONS_BY_LEADER, "SELECT formationId, leaderGUID, formationAI FROM creature_formations WHERE leaderGUID = ? ", CONNECTION_SYNCH)
    PREPARE_STATEMENT(WORLD_ADD_CREFORMATION_DATA, "INSERT INTO creature_formation_data (formationId, memberGUID, dist, angle) VALUES (?, ?, ?, ?)", CONNECTION_SYNCH);
    PREPARE_STATEMENT(WORLD_ADD_CREFORMATIONS, "INSERT INTO creature_formations (leaderGUID, formationAI, comment) VALUES (?, ?, ?)", CONNECTION_SYNCH)

    PREPARE_STATEMENT(WORLD_LOAD_CREGROUPS_MAXID, "SELECT MAX(groupId) FROM creature_groups", CONNECTION_SYNCH);
    PREPARE_STATEMENT(WORLD_LOAD_CREGROUPS_WITHOUT_DATA, "SELECT COUNT(groupId) FROM creature_groups WHERE groupId NOT IN (SELECT groupId FROM creature_group_data)", CONNECTION_SYNCH)
    PREPARE_STATEMENT(WORLD_LOAD_CREGROUP_DATA_WITHOUT_GROUP, "SELECT COUNT(groupId) FROM creature_group_data WHERE groupId NOT IN (SELECT groupId FROM creature_groups)", CONNECTION_SYNCH)
    PREPARE_STATEMENT(WORLD_LOAD_CREGROUPS, "SELECT groupId, leaderGUID, groupType FROM creature_groups WHERE groupId IN (SELECT groupId FROM creature_group_data) ORDER BY groupId", CONNECTION_SYNCH)
    PREPARE_STATEMENT(WORLD_LOAD_CREGROUP_DATA, "SELECT groupId, memberGUID FROM creature_group_data WHERE groupId IN (SELECT groupId FROM creature_groups) ORDER BY groupId", CONNECTION_SYNCH)
    PREPARE_STATEMENT(WORLD_LOAD_CREGROUPS_BY_LEADER, "SELECT groupId, leaderGUID, groupType FROM creature_groups WHERE leaderGUID = ? ", CONNECTION_SYNCH)
    PREPARE_STATEMENT(WORLD_ADD_CREGROUP_DATA, "INSERT INTO creature_group_data (groupId, memberGUID) VALUES (?, ?)", CONNECTION_SYNCH)
    PREPARE_STATEMENT(WORLD_ADD_CREGROUPS, "INSERT INTO creature_groups (leaderGUID, groupType, comment) VALUES (?, ?, ?)", CONNECTION_SYNCH)
}
