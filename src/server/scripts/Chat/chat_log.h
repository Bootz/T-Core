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

#ifndef TRILLIUMCORE_CHATLOG_H
#define TRILLIUMCORE_CHATLOG_H

#include "ScriptPCH.h"
#include "ChatLexicsCutter.h"

enum ChatLogType
{
    CHAT_LOG_NONE,
    CHAT_LOG_CHAT,
    CHAT_LOG_PARTY,
    CHAT_LOG_GUILD,
    CHAT_LOG_WHISPER,
    CHAT_LOG_CHANNEL,
    CHAT_LOG_RAID,
    CHAT_LOG_BATTLEGROUND,

    CHAT_LOG_COUNT,
    CHAT_LOG_INNORMATIVE
};

enum LexicsActions
{
    LEXICS_ACTION_LOG,
    LEXICS_ACTION_SHEEP,
    LEXICS_ACTION_STUN,
    LEXICS_ACTION_DIE,
    LEXICS_ACTION_DRAIN,
    LEXICS_ACTION_SILENCE,
    LEXICS_ACTION_STUCK,
    LEXICS_ACTION_SICKNESS,
    LEXICS_ACTION_SHEAR,
};

class ChatLogInfo
{
private:
    bool _cutFlag;
    ChatLogType _type;
    std::string _strType;

public:
    ChatLogInfo(ChatLogType type, bool chat, bool lexics);

    bool IsCut() const { return _cutFlag; }

    void Write(const std::string& msg);
    void Write(const char* fmt, ...)        ATTR_PRINTF(2, 3);
};

class ChatLog : public PlayerScript
{
public:
    static const char* GetChatNameByType(ChatLogType type);
    static const char* GetChatDescByType(ChatLogType type);

    ChatLog();
    ~ChatLog();

    void OnChat(Player* player, uint32 type, uint32 lang, std::string& msg);
    void OnChat(Player* player, uint32 type, uint32 lang, std::string& msg, Player* receiver);
    void OnChat(Player* player, uint32 type, uint32 lang, std::string& msg, Group* group);
    void OnChat(Player* player, uint32 type, uint32 lang, std::string& msg, Guild* guild);
    void OnChat(Player* player, uint32 type, uint32 lang, std::string& msg, Channel* channel);

private:
    bool _ChatCommon(ChatLogType type, Player* player, std::string& msg);
    void _Punish(Player* player, std::string& msg);
    void _ApplySpell(Player* player, uint32 spellId);

    void _Initialize();
    void _AppendPlayerName(Player* player, std::ostringstream& ss);
    void _AppendGroupMembers(Group* group, std::ostringstream& ss);
    void _WriteLog(ChatLogInfo* log, const std::string& logStr, const std::string& origMsg);

    // Chats
    bool _enable;
    bool _ignoreUnprintable;

    ChatLogInfo* _logs[CHAT_LOG_COUNT];

    // Lexics
    LexicsCutter* _lexics;

    bool _lexicsEnable;
    bool _lexicsInnormativeCut;
    bool _lexicsIgnoreGm;

    std::string _lexicsCutReplacement;
    LexicsActions _lexicsAction;
    uint32 _lexicsActionDuration;

    ChatLogInfo* _innormativeLog;
};

#define sChatLog (*ACE_Singleton<ChatLog, ACE_Thread_Mutex>::instance())

#endif