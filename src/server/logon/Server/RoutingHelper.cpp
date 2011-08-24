#include "DatabaseEnv.h"
#include "Log.h"
#include "Config.h"
#include "RoutingHelper.h"
#include "WorldSession.h"
#include "NodeSocketMgr.h"
#include "World.h"

void RoutingHelper::LoadNodeMap()
{
    QueryResult result = LogonDatabase.Query( "SELECT * FROM map_table" );

    int count = 0;
    if( !result )
    {
        sLog->outString();
        sLog->outString( ">> Loaded %u Node assigns", count );
        return;
    }

    do
    {
        NodeMap m_nodemap;

        Field *fields = result->Fetch();
        uint32 Mapid     = fields[0].GetUInt32();
        m_nodemap.NodeID = fields[1].GetUInt32();
        m_nodemap.BackupNodeID = fields[2].GetUInt32();
        m_nodemap.SubNodeID = fields[3].GetUInt32();

        m_maplist[Mapid] = m_nodemap;

        ++count;

    }while (result->NextRow());

    sLog->outString();
    sLog->outString( ">> Loaded %u Node assigns", count );

    MasterNode = sNodeSocketMgr->GetFirstNode();
    m_OnMaster = true;
}

uint32 RoutingHelper::GetNodeForMap(uint32 Map_ID)
{
    NodeMapList::const_iterator iter = m_maplist.find(Map_ID);
    if (iter == m_maplist.end())
        return 0;
    else
        return iter->second.NodeID;
}

uint32 RoutingHelper::GetBackupNodeForMap(uint32 Map_ID)
{
    NodeMapList::const_iterator iter = m_maplist.find(Map_ID);
    if (iter == m_maplist.end())
        return 0;
    else
        return iter->second.SubNodeID;
}

uint32 RoutingHelper::GetNodeForID(uint32 Node_ID)
{
    NodeMapList::iterator iter = m_maplist.begin();
    for (; iter != m_maplist.end(); ++iter)
        if (iter->second.BackupNodeID == Node_ID)
            return iter->second.NodeID;

    return 0;
}

uint32 RoutingHelper::GetBackupNodeForID(uint32 Node_ID)
{
    NodeMapList::iterator iter = m_maplist.begin();
    for (; iter != m_maplist.end(); ++iter)
        if (iter->second.NodeID == Node_ID)
            return iter->second.BackupNodeID;

    return 0;
}

uint32 RoutingHelper::GetSubNodeForMap(uint32 Map_ID)
{
    NodeMapList::const_iterator iter = m_maplist.find(Map_ID);
    if (iter == m_maplist.end())
        return 0;
    else
        return iter->second.SubNodeID;
}

bool RoutingHelper::GoToMapNode(WorldSession *m_Session, uint32 MapID)
{
    NodeMapList::const_iterator iter = m_maplist.find(MapID);
    if (iter == m_maplist.end())
        return false;
    else
    {
        if(sNodeSocketMgr->CheckNodeID(iter->second.NodeID))
            return sNodeSocketMgr->OpenConnection(iter->second.NodeID, m_Session);
        else if(sNodeSocketMgr->CheckNodeID(iter->second.BackupNodeOnline))
            return sNodeSocketMgr->OpenConnection(iter->second.BackupNodeID, m_Session);
    }

    return false;
}

//Master ist immer Node1 bis der Logon alles selbst kann
//Wenn Node1 tot ist gehts zum Backup
bool RoutingHelper::ConnectToMaster(WorldSession *m_Session)
{
    if (sNodeSocketMgr->OpenConnection(MasterNode, m_Session))
        return true;
    else 
    {
        if (m_OnMaster)
        {
            uint32 NewMasterNode = GetBackupNodeForID(MasterNode);
            if ((NewMasterNode > 0) && sNodeSocketMgr->OpenConnection(NewMasterNode, m_Session))
            {
                MasterNode = NewMasterNode;
                m_OnMaster = false;
                return true;
            }
        }
        else
        {
            uint32 NewMasterNode = GetNodeForID(MasterNode);
            if ((NewMasterNode > 0) && sNodeSocketMgr->OpenConnection(NewMasterNode, m_Session))
            {
                MasterNode = NewMasterNode;
                m_OnMaster = true;
                return true;
            }
        }

    }
    return false;
}