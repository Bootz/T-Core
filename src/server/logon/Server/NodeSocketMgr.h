/*
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * Copyright (C) 2008-2010 Trinity <http://www.trinitycore.org/>
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

/** \addtogroup u2w User to World Communication
 *  @{
 *  \file WorldSocketMgr.h
 *  \author Derex <derex101@gmail.com>
 */

#ifndef __NODESOCKETMGR_H
#define __NODESOCKETMGR_H

#include <ace/Basic_Types.h>
#include <ace/Singleton.h>
#include <ace/Thread_Mutex.h>
#include "SharedDefines.h"

class NodeSocket;
class NodeThreadRunnable;
class ACE_Event_Handler;


struct NodeList
{
    std::string name;
    std::string IP;
    uint16 port;
    bool online;
};

typedef UNORDERED_MAP<uint32, NodeList> NodeListList;


/// Manages all sockets connected to peers and network threads
class NodeSocketMgr
{
public:
  friend class NodeSocket;
  friend class ACE_Singleton<NodeSocketMgr,ACE_Thread_Mutex>;

  /// Start network
  int StartNetwork ();

  /// Load NodeList
  void LoadNodeList();
  /// FindNodeID
  bool CheckNodeID(uint32 NodeID);
  uint32 GetFirstNode();

  /// Stops all network threads, It will wait for all running threads .
  void StopNetwork();

  /// Wait untill all network threads have "joined" .
  void Wait();

  /// Make this class singleton .
  static NodeSocketMgr* Instance();

  bool OpenConnection(uint32 nodeid, WorldSession *m_Session);
  void RefreshOnlineStat();

private:

  int OnSocketOpen(NodeSocket* sock);
  int StartReactiveIO();

private:
  NodeSocketMgr();
  virtual ~NodeSocketMgr();

  NodeThreadRunnable* m_NetThreads;
  size_t m_NetThreadsCount;

  int m_SockOutKBuff;
  int m_SockOutUBuff;
  bool m_UseNoDelay;

  ACE_Event_Handler* m_Connector;

  NodeListList m_nodelist;
};

#define sNodeSocketMgr NodeSocketMgr::Instance()

#endif
