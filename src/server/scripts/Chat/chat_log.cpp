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

#include "Common.h"
#include "ChatLexicsCutter.h"
#include "chat_log.h"
#include "Chat.h"
#include "Group.h"
#include "Guild.h"
#include "Channel.h"
#include "ObjectMgr.h"
#include "SpellAuras.h"
#include "Config.h"
#include "LogMgr.h"

ChatLogInfo::ChatLogInfo(ChatLogType type, bool chat, bool lexics) : _cutFlag(false), _type(type)
{
    _strType = ChatLog::GetChatNameByType(type);
    if (chat)
    {
        sLogMgr->RegisterLogFile(_strType.c_str());
        sLogMgr->WriteLn(_strType.c_str(), "[SYSTEM] %s Log Initialized", ChatLog::GetChatDescByType(_type));
    }
    if (lexics)
        _cutFlag = sConfig->GetBoolDefault(std::string("ChatLog.Lexics." + _strType + ".Cut").c_str(), true);
}

void ChatLogInfo::Write(const std::string& msg)
{
    sLogMgr->WriteLn(_strType.c_str(), msg);
}

void ChatLogInfo::Write(const char* fmt, ...)
{
    va_list lst;
    va_start(lst, fmt);
    sLogMgr->WriteLn(_strType.c_str(), fmt, lst);
    va_end(lst);
}

const char* ChatLog::GetChatNameByType(ChatLogType type)
{
    switch (type)
    {
        case CHAT_LOG_CHAT: return "Chat";
        case CHAT_LOG_PARTY: return "Party";
        case CHAT_LOG_GUILD: return "Guild";
        case CHAT_LOG_WHISPER: return "Whisper";
        case CHAT_LOG_CHANNEL: return "Channel";
        case CHAT_LOG_RAID: return "Raid";
        case CHAT_LOG_BATTLEGROUND: return "BattleGround";
        case CHAT_LOG_INNORMATIVE: return "Lexics.Innormative";
        default: return "Unknown";
    }
}

const char* ChatLog::GetChatDescByType(ChatLogType type)
{
    switch (type)
    {
        case CHAT_LOG_CHAT: return "Chat";
        case CHAT_LOG_PARTY: return "Party Chat";
        case CHAT_LOG_GUILD: return "Guild Chat";
        case CHAT_LOG_WHISPER: return "Whisper";
        case CHAT_LOG_CHANNEL: return "Channels";
        case CHAT_LOG_RAID: return "Raid Chat";
        case CHAT_LOG_BATTLEGROUND: return "Battleground Chat";
        case CHAT_LOG_INNORMATIVE: return "Lexics Innormative";
        default: return "Unknown";
    }
}

ChatLog::ChatLog() : PlayerScript("LexicsChatLog"), _lexics(NULL), _innormativeLog(NULL)
{
    _Initialize();
}

ChatLog::~ChatLog()
{
    if (_lexics)
        delete _lexics;
    for (uint32 i = CHAT_LOG_CHAT; i < CHAT_LOG_COUNT; ++i)
        delete _logs[i];
    if (_innormativeLog)
        delete _innormativeLog;
}

void ChatLog::_Initialize()
{
    // Load config settings
    _enable = sConfig->GetBoolDefault("ChatLog.Enable", true);
    _ignoreUnprintable = sConfig->GetBoolDefault("ChatLog.Ignore.Unprintable", true);

    _lexicsEnable = sConfig->GetBoolDefault("ChatLog.Lexics.Enable", true);
    if (_lexicsEnable)
    {
        std::string analogsFileName = sConfig->GetStringDefault("ChatLog.Lexics.AnalogsFile", "");
        std::string innormativeWordsFileName = sConfig->GetStringDefault("ChatLog.Lexics.WordsFile", "");

        _innormativeLog = new ChatLogInfo(CHAT_LOG_INNORMATIVE, true, false);
        if (analogsFileName.empty() || innormativeWordsFileName.empty())
            _lexicsEnable = false;
        else
        {
            // Initialize lexics cutter parameters
            _lexicsInnormativeCut = sConfig->GetBoolDefault("ChatLog.Lexics.Cut.Enable", true);
            _lexicsCutReplacement = sConfig->GetStringDefault("ChatLog.Lexics.Cut.Replacement", "&!@^%!^&*!!!");
            _lexicsAction = LexicsActions(sConfig->GetIntDefault("ChatLog.Lexics.Action", LEXICS_ACTION_LOG));
            _lexicsActionDuration = sConfig->GetIntDefault("ChatLog.Lexics.Action.Duration", 0);

            // Initialize lexics cutter object
            _lexics = new LexicsCutter(analogsFileName, innormativeWordsFileName,
                sConfig->GetBoolDefault("ChatLog.Lexics.Ignore.Spaces", true),
                sConfig->GetBoolDefault("ChatLog.Lexics.Ignore.Repeats", true));

            // Read additional parameters
            _lexicsIgnoreGm = sConfig->GetBoolDefault("ChatLog.Lexics.Ignore.GM", true);
        }
    }

    for (uint32 i = CHAT_LOG_CHAT; i < CHAT_LOG_COUNT; ++i)
        _logs[i] = new ChatLogInfo(ChatLogType(i), _enable, _lexicsEnable);
}

bool ChatLog::_ChatCommon(ChatLogType type, Player* player, std::string& msg)
{
    // Check message for innormative lexics and punish if necessary.
    if (_lexicsEnable && _lexics && _logs[type]->IsCut() && _lexics->CheckLexics(msg)) 
        _Punish(player, msg);

    if (!_enable)
        return false;

    if (_ignoreUnprintable)
    {
        // If should ignore unprintables, verify string by UTF8 here
        unsigned int pos = 0;
        std::string ch;
        while (LexicsCutter::ReadUTF8(msg, ch, pos))
            if (ch.size() == 1)
                if (ch[0] < ' ')
                    return false; // Unprintable detected
    }
    return true;
}

void ChatLog::_Punish(Player* player, std::string& msg)
{
    std::ostringstream ss;

    _AppendPlayerName(player, ss);
    _WriteLog(_innormativeLog, ss.str(), msg);

    // Check if should ignore GM
    if (_lexicsIgnoreGm && (player->GetSession()->GetSecurity() > SEC_PLAYER))
        return;

    // Cut innormative lexics
    if (_lexicsInnormativeCut)
        msg = _lexicsCutReplacement;

    if (!player || !player->GetSession())
        return;

    // special action
    switch (_lexicsAction)
    {
        case LEXICS_ACTION_SHEEP:       _ApplySpell(player, 118);   break;
        case LEXICS_ACTION_STUN:        _ApplySpell(player, 13005); break;
        case LEXICS_ACTION_STUCK:       _ApplySpell(player, 23312); break;
        case LEXICS_ACTION_SICKNESS:    _ApplySpell(player, 15007); break;
        case LEXICS_ACTION_SHEAR:       _ApplySpell(player, 41032); break;
        case LEXICS_ACTION_DIE:
            player->DealDamage(player, player->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            break;
        case LEXICS_ACTION_DRAIN:
            player->DealDamage(player, player->GetHealth() - 5, NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            break;
        case LEXICS_ACTION_SILENCE:
            player->GetSession()->m_muteTime = time(NULL) + int64(_lexicsActionDuration / 1000);
            break;
        default:
            // No action except logging
            break;
    }
}

inline void ChatLog::_ApplySpell(Player *player, uint32 spellId)
{
    if (Aura *a = player->AddAura(spellId, player))
        a->SetDuration(_lexicsActionDuration);
}

inline void ChatLog::_WriteLog(ChatLogInfo* log, const std::string& logStr, const std::string& origMsg)
{
    if (log)
        log->Write("%s %s", logStr.c_str(), origMsg.c_str());
}

inline void ChatLog::_AppendPlayerName(Player* player, std::ostringstream& ss)
{
    ss << "[" << (player ? player->GetName() : "???") << "]";
}

inline void ChatLog::_AppendGroupMembers(Group* group, std::ostringstream& ss)
{
    if (!group)
        ss << " {unknown group}:";
    else
    {
        char sz[32];
        sprintf(sz, UI64FMTD, group->GetGUID());
        ss << " {" << sz << "} [";
        const uint64& leaderGuid = group->GetLeaderGUID();
        if (Player* leader = ObjectAccessor::FindPlayer(leaderGuid))
            ss << leader->GetName();

        Group::MemberSlotList members = group->GetMemberSlots();
        for (Group::member_citerator itr = members.begin(); itr != members.end(); ++itr)
        {
            if (itr->guid == leaderGuid)
                continue;

            if (Player* member = ObjectAccessor::FindPlayer(itr->guid))
                ss << "," << member->GetName();
        }
        ss << "]:";
    }
}

void ChatLog::OnChat(Player* player, uint32 type, uint32 /*lang*/, std::string& msg)
{
    std::string origMsg(msg);
    if (!_ChatCommon(CHAT_LOG_CHAT, player, msg))
        return;

    std::ostringstream ss;
    switch (type)
    {
        case CHAT_MSG_SAY:      ss << "{SAY}";      break;
        case CHAT_MSG_EMOTE:    ss << "{EMOTE}";    break;
        case CHAT_MSG_YELL:     ss << "{YELL}";     break;
    }
    _AppendPlayerName(player, ss);
    _WriteLog(_logs[CHAT_LOG_CHAT], ss.str(), origMsg);
}

void ChatLog::OnChat(Player* player, uint32 /*type*/, uint32 /*lang*/, std::string& msg, Player* receiver)
{
    std::string origMsg(msg);
    if (!_ChatCommon(CHAT_LOG_WHISPER, player, msg))
        return;

    std::ostringstream ss;
    _AppendPlayerName(player, ss);
    ss << "->";
    _AppendPlayerName(receiver, ss);

    _WriteLog(_logs[CHAT_LOG_WHISPER], ss.str(), origMsg);
}

void ChatLog::OnChat(Player* player, uint32 type, uint32 /*lang*/, std::string& msg, Group* group)
{
    std::string origMsg(msg);
    std::ostringstream ss;
    _AppendPlayerName(player, ss);

    switch (type)
    {
    case CHAT_MSG_PARTY:
    case CHAT_MSG_PARTY_LEADER:
        if (_ChatCommon(CHAT_LOG_PARTY, player, msg))
        {
            switch (type)
            {
                case CHAT_MSG_PARTY:        ss << "->PARTY";        break;
                case CHAT_MSG_PARTY_LEADER: ss << "->PARTY_LEADER"; break;
            }
            _AppendGroupMembers(group, ss);
            _WriteLog(_logs[CHAT_LOG_PARTY], ss.str(), origMsg);
        }
        break;
    case CHAT_MSG_RAID_LEADER:
    case CHAT_MSG_RAID_WARNING:
    case CHAT_MSG_RAID:
        if (_ChatCommon(CHAT_LOG_RAID, player, msg))
        {
            switch (type)
            {
                case CHAT_MSG_RAID_LEADER:  ss << "->RAID_LEADER";  break;
                case CHAT_MSG_RAID_WARNING: ss << "->RAID_WARN";    break;
                case CHAT_MSG_RAID:         ss << "->RAID";         break;
            }
            _AppendGroupMembers(group, ss);
            _WriteLog(_logs[CHAT_LOG_RAID], ss.str(), origMsg);
        }
        break;
    case CHAT_MSG_BATTLEGROUND:
    case CHAT_MSG_BATTLEGROUND_LEADER:
        if (_ChatCommon(CHAT_LOG_BATTLEGROUND, player, msg))
        {
            switch (type)
            {
                case CHAT_MSG_BATTLEGROUND:         ss << "->BG";           break;
                case CHAT_MSG_BATTLEGROUND_LEADER:  ss << "->BG_LEADER";    break;
            }
            _AppendGroupMembers(group, ss);
            _WriteLog(_logs[CHAT_LOG_BATTLEGROUND], ss.str(), origMsg);
        }
        break;
    }
}

void ChatLog::OnChat(Player* player, uint32 type, uint32 lang, std::string& msg, Guild* guild)
{
    std::string origMsg(msg);
    if (!_ChatCommon(CHAT_LOG_GUILD, player, msg))
        return;

    std::ostringstream ss;
    _AppendPlayerName(player, ss);
    switch (type)
    {
        case CHAT_MSG_GUILD:    ss << "->GUILD";      break;
        case CHAT_MSG_OFFICER:  ss << "->GUILD_OFF";  break;
    }
    ss << " {" << (guild ? guild->GetName() : "unknowng guild") << "}:";

    _WriteLog(_logs[CHAT_LOG_GUILD], ss.str(), origMsg);
}

void ChatLog::OnChat(Player* player, uint32 /*type*/, uint32 /*lang*/, std::string& msg, Channel* channel)
{
    std::string origMsg(msg);
    if (!_ChatCommon(CHAT_LOG_CHANNEL, player, msg))
        return;

    std::ostringstream ss;
    _AppendPlayerName(player, ss);
    ss << " {" << (channel ? channel->GetName() : "Unknown channel") << "}";

    _WriteLog(_logs[CHAT_LOG_CHANNEL], ss.str(), origMsg);
}

void AddSC_chat_log()
{
    new ChatLog();
}