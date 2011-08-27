#ifndef ROUTING_HELPER_H
#define ROUTING_HELPER_H

#include "Define.h"
#include <ace/Singleton.h>
#include <ace/Thread_Mutex.h>
#include "Common.h"

class WorldSession;

//MapHolder
struct NodeMap
{
    uint32 NodeID;
    bool NodeOnline;
    uint32 BackupNodeID;
    bool BackupNodeOnline;
    uint32 Master;

    //Used for expand Maps
    uint32 SubNodeID;
    bool SubNodeOnline;
};
typedef UNORDERED_MAP<uint32, NodeMap> NodeMapList;

class RoutingHelper
{
    public:
        
        void LoadNodeMap();

        uint32 GetNodeForMap(uint32 Map_ID);
        uint32 GetNodeForID(uint32 Node_ID);
        uint32 GetBackupNodeForMap(uint32 Map_ID);
        uint32 GetBackupNodeForID(uint32 Node_ID);
        uint32 GetSubNodeForMap(uint32 Map_ID);
        
        bool GoToMapNode(WorldSession *m_Session, uint32 MapID);
        bool ConnectToMaster(WorldSession *m_Session);

    private:
        NodeMapList m_maplist;

        bool m_OnMaster;
        uint32 MasterNode;

};

#define sRoutingHelper ACE_Singleton<RoutingHelper, ACE_Thread_Mutex>::instance()
#endif
