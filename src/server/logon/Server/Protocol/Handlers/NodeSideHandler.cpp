#include "Player.h"
#include "WorldSession.h"
#include "NodeSocketMgr.h"
#include "RoutingHelper.h"
#include "NodeSocket.h"
#include "WorldPacket.h"

void WorldSession::Handle_SMSG_TransferPending(WorldPacket& recvPacket)
{
    //4 + 4 + 4 + 4 + 4
    uint32 map_id;
    float x, y, z, o;
    recvPacket >> x;
    recvPacket >> y;
    recvPacket >> z;
    recvPacket >> map_id;
    recvPacket >> o;

    _player->TeleportTo(map_id, x, y, z, o);

    //Check if map is on this node
    if (sRoutingHelper->GetNodeForMap(map_id) == m_NodeID)
        return;

    WorldPacket data(CMSG_PLAYER_LOGOUT);
    SendPacketToNode(&data);
    
    m_new_node = true;
    m_new_map = map_id;
}

void WorldSession::Handle_SMSG_LOGOUT_COMPLETE(WorldPacket& recvPacket)
{
    if (!m_new_node)
        return;

    m_new_node = false;

    if (sRoutingHelper->GetNodeForMap(m_new_map) == m_NodeID)
        return;

    //_player->SaveToDB();
    
    if (!sRoutingHelper->GoToMapNode(this, m_new_map))
    {
        SendNotification("Node offline");
    }
}
