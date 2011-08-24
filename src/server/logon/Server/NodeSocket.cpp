#include <ace/Message_Block.h>
#include <ace/OS_NS_string.h>
#include <ace/OS_NS_unistd.h>
#include <ace/os_include/arpa/os_inet.h>
#include <ace/os_include/netinet/os_tcp.h>
#include <ace/os_include/sys/os_types.h>
#include <ace/os_include/sys/os_socket.h>
#include <ace/OS_NS_string.h>
#include <ace/Reactor.h>
#include <ace/Auto_Ptr.h>

#include "NodeSocket.h"
#include "Common.h"

#include "Util.h"
#include "World.h"
#include "WorldPacket.h"
#include "SharedDefines.h"
#include "ByteBuffer.h"
#include "Opcodes.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "BigNumber.h"
#include "SHA1.h"
#include "WorldSession.h"
#include "NodeSocketMgr.h"
#include "Log.h"

#if defined(__GNUC__)
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

struct ServerPktHeader
{
    /**
     * size is the length of the payload _plus_ the length of the opcode
     */
    ServerPktHeader(uint32 size, uint16 cmd) : size(size)
    {
        uint8 headerIndex=0;
        if (isLargePacket())
        {
            sLog->outDebug(LOG_FILTER_NETWORKIO, "initializing large server to client packet. Size: %u, cmd: %u", size, cmd);
            header[headerIndex++] = 0x80|(0xFF &(size>>16));
        }
        header[headerIndex++] = 0xFF &(size>>8);
        header[headerIndex++] = 0xFF &size;

        header[headerIndex++] = 0xFF & cmd;
        header[headerIndex++] = 0xFF & (cmd>>8);
    }

    uint8 getHeaderLength()
    {
        // cmd = 2 bytes, size= 2||3bytes
        return 2+(isLargePacket()?3:2);
    }

    bool isLargePacket()
    {
        return size > 0x7FFF;
    }

    const uint32 size;
    uint8 header[5];
};

struct ClientPktHeader
{
    uint16 size;
    uint32 cmd;
};

#if defined(__GNUC__)
#pragma pack()
#else
#pragma pack(pop)
#endif


NodeSocket::NodeSocket (void) :
notifier_ (0, this, ACE_Event_Handler::WRITE_MASK),
NodeHandler(),
m_WorldSession (0),
m_RecvWPct (0),
m_RecvPct (),
m_Header (sizeof (ClientPktHeader)),
m_OutBuffer (),
m_OutBufferSize (65536),
m_OutActive (false),
m_Seed (static_cast<uint32> (rand32 ())),
m_OverSpeedPings (0),
m_LastPingTime (ACE_Time_Value::zero)
{
	reference_counting_policy().value (ACE_Event_Handler::Reference_Counting_Policy::ENABLED);

    msg_queue()->high_water_mark(8*1024*1024);
    msg_queue()->low_water_mark(8*1024*1024);
}

NodeSocket::~NodeSocket (void)
{
    delete m_RecvWPct;

    //if (m_OutBuffer)
    //    m_OutBuffer->release();

    closing_ = true;

    peer().close();
}

int NodeSocket::open (void *p)
{
    //if (m_OutBuffer)
    //    return -1;

    m_OutActive = true;

    ACE_Time_Value iter_delay (1);   // Two seconds

    if (NodeHandler::open (p) == -1)
        return -1;

    if (sNodeSocketMgr->OnSocketOpen(this) == -1)
        return -1;

    this->iterations_ = 0;

  return 0;
}

long NodeSocket::AddReference (void)
{
    return static_cast<long> (add_reference());
}

long NodeSocket::RemoveReference (void)
{
    return static_cast<long> (remove_reference());
}

int NodeSocket::close (int)
{
    shutdown ();

    closing_ = true;

    return 0;
}

bool NodeSocket::IsClosed (void) const
{
    return closing_;
}

void NodeSocket::CloseSocket (void)
{
    {
        ACE_GUARD (LockType, Guard, m_OutBufferLock);

        if (closing_)
            return;

        closing_ = true;
        peer().close_writer();
    }

    {
        ACE_GUARD (LockType, Guard, m_SessionLock);
        m_WorldSession = NULL;
    }
}

int NodeSocket::Update (void)
{
    if (closing_)
        return -1;

	if (m_OutActive || (m_OutBuffer.length() == 0 && msg_queue()->is_empty()))
        return 0;

    int ret;
    do
        ret = handle_output (get_handle());
    while (ret > 0);

    return ret;
}

int NodeSocket::ProcessIncoming (WorldPacket* new_pct)
{
	ACE_ASSERT (new_pct);

	// manage memory ;)
    ACE_Auto_Ptr<WorldPacket> aptr (new_pct);

	const ACE_UINT16 opcode = new_pct->GetOpcode ();

	sLog->outDebug (LOG_FILTER_NETWORKIO, "NodeSocket_IN: SOCKET: %u LENGTH: %u OPCODE: %s (0x%.4X)",
                   (uint32) get_handle (),
                   new_pct->size (),
                   LookupOpcodeName (new_pct->GetOpcode ()),
                   new_pct->GetOpcode ());

    //We've got no Session for the node, so we have to handle all NodeOpcodes here :)
    try {
        switch(opcode)
        {
		case SMSG_AUTH_CHALLENGE:
			return HandleAuth(*new_pct);
        default:
            {
                ACE_GUARD_RETURN (LockType, Guard, m_SessionLock, -1);

                if (m_WorldSession != NULL)
                {
                    // Our Idle timer will reset on any non PING opcodes.
                    // Catches people idling on the login screen and any lingering ingame connections.
                    m_WorldSession->ResetTimeOutTime();

                    // OK ,give the packet to WorldSession
                    aptr.release();
                    // WARNINIG here we call it with locks held.
                    // Its possible to cause deadlock if QueuePacket calls back
                    m_WorldSession->QueueNodePacket (new_pct);
                    return 0;
                }
                else
                {
                    sLog->outError ("NodeSocket::ProcessIncoming: Client not authed opcode = %u", uint32(opcode));
                    return -1;
                }
            }
        }
    }
    catch(ByteBufferException &)
    {
        if(sLog->IsOutDebug())
        {
            sLog->outDebug(LOG_FILTER_NETWORKIO, "Dumping error causing packet:");
            new_pct->hexlike();
        }

        return -1;
    }

    ACE_NOTREACHED (return 0);
    return 0;
}

const std::string& NodeSocket::GetRemoteAddress (void) const
{
    return m_Address;
}

int NodeSocket::handle_input (ACE_HANDLE)
{
    if (closing_)
        return -1;

    switch (handle_input_missing_data ())
    {
        case -1 :
        {
            if ((errno == EWOULDBLOCK) || (errno == EAGAIN))
            {
                return Update ();                           // interesting line ,isn't it ?
            }

            //m_WorldSession->SetReconnect(true);
            sLog->outDebug(LOG_FILTER_NETWORKIO, "ClusterSocket::handle_input: Peer error closing connection errno = %s", ACE_OS::strerror (errno));
            errno = ECONNRESET;
            CloseSocket();
            return -1;
        }
        case 0:
        {
            //m_WorldSession->SetReconnect(true);
            sLog->outDebug(LOG_FILTER_NETWORKIO, "ClusterSocket::handle_input: Peer has closed connection");
            errno = ECONNRESET;
            CloseSocket();
            return -1;
        }
        case 1:
            return 1;
        default:
            return Update ();                               // another interesting line ;)
    }

    ACE_NOTREACHED(return -1);
}

//Der Output Müll...
//Fragt mich nicht was das hier macht, ich weiss es selbst nicht...
int NodeSocket::handle_output (ACE_HANDLE)
{
	ACE_GUARD_RETURN (LockType, Guard, m_OutBufferLock, -1);

    if (closing_)
        return -1;

    size_t send_len = m_OutBuffer.length();

    if (send_len == 0)
        return handle_output_queue(Guard);

#ifdef MSG_NOSIGNAL
    ssize_t n = peer().send (m_OutBuffer.rd_ptr(), send_len, MSG_NOSIGNAL);
#else
    ssize_t n = peer().send (m_OutBuffer.rd_ptr(), send_len);
#endif // MSG_NOSIGNAL

    if (n == 0)
        return -1;
    else if (n == -1)
    {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
            return 0;

        return -1;
    }
    else if (n < (ssize_t)send_len) //now n > 0
    {
        m_OutBuffer.rd_ptr (static_cast<size_t> (n));

        // move the data to the base of the buffer
        m_OutBuffer.crunch();

        return 0;
    }
    else //now n == send_len
    {
        m_OutBuffer.reset ();

        return handle_output_queue (Guard);
    }

    ACE_NOTREACHED (return 0);
}

int NodeSocket::handle_output_queue (GuardType& g)
{
    if (msg_queue()->is_empty())
        return 0;

    ACE_Message_Block *mblk;

    if (msg_queue()->dequeue_head(mblk, (ACE_Time_Value*)&ACE_Time_Value::zero) == -1)
    {
        sLog->outError("NodeSocket::handle_output_queue dequeue_head");
        return -1;
    }

    const size_t send_len = mblk->length();

#ifdef MSG_NOSIGNAL
    ssize_t n = peer().send (mblk->rd_ptr(), send_len, MSG_NOSIGNAL);
#else
    ssize_t n = peer().send (mblk->rd_ptr(), send_len);
#endif // MSG_NOSIGNAL

    if (n == 0)
    {
        mblk->release();

        return -1;
    }
    else if (n == -1)
    {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
        {
            msg_queue()->enqueue_head(mblk, (ACE_Time_Value*) &ACE_Time_Value::zero);
            return 0;
        }

        mblk->release();
        return -1;
    }
    else if (n < (ssize_t)send_len) //now n > 0
    {
        mblk->rd_ptr(static_cast<size_t> (n));

        if (msg_queue()->enqueue_head(mblk, (ACE_Time_Value*) &ACE_Time_Value::zero) == -1)
        {
            sLog->outError("NodeSocket::handle_output_queue enqueue_head");
            mblk->release();
            return -1;
        }

        return 0;
    }
    else //now n == send_len
    {
        mblk->release();

        return msg_queue()->is_empty() ? 0 : ACE_Event_Handler::WRITE_MASK;
    }

    ACE_NOTREACHED(return -1);
}

int NodeSocket::SendPacket (const WorldPacket& pct)
{
	ACE_GUARD_RETURN (LockType, Guard, m_OutBufferLock, -1);

    if (closing_)
        return -1;

    // Dump outgoing packet.
	sLog->outDebug (LOG_FILTER_NETWORKIO, "NODE_OUT:\nSOCKET: %u\nLENGTH: %u\nOPCODE: %s (0x%.4X)\nDATA:\n",
                     (uint32) get_handle (),
                     pct.size (),
					 LookupOpcodeName (pct.GetOpcode ()),
                     pct.GetOpcode ());

    WorldPacket pack(pct.GetOpcode());
    pack << uint16(00);

    //Kleine Vorsichtsmaßnahme um Crashes vorzubeugen
    if (pct.size() != 0)
        pack.append(pct.contents(), pct.size()) ;

    ServerPktHeader header(pack.size()+2, pack.GetOpcode());
    m_Crypt.EncryptSend ((uint8*)header.header, header.getHeaderLength());

    if (m_OutBuffer.space() >= pack.size()+ header.getHeaderLength() && msg_queue()->is_empty())
    {
        // Put the packet on the buffer.
        if (m_OutBuffer.copy((char*) header.header, header.getHeaderLength()) == -1)
            ACE_ASSERT (false);

        if (!pack.empty())
        {
            if (m_OutBuffer.copy((char*) pack.contents(), pack.size()) == -1)
                ACE_ASSERT (false);
        }
    }
    else
    {
        // Enqueue the packet.
        ACE_Message_Block* mb;

        ACE_NEW_RETURN(mb, ACE_Message_Block(pack.size() + header.getHeaderLength()), -1);

        mb->copy((char*) header.header, header.getHeaderLength());

        if (!pack.empty())
            mb->copy((const char*)pack.contents(), pack.size());

        if (msg_queue()->enqueue_tail(mb,(ACE_Time_Value*)&ACE_Time_Value::zero) == -1)
        {
            sLog->outError("NodeSocket::SendPacket enqueue_tail failed");
            mb->release();
            return -1;
        }
    }

    return 0;
}

//Handlings
int NodeSocket::handle_input_missing_data (void)
{
	char buf [4096];

    ACE_Data_Block db ( sizeof (buf),
                        ACE_Message_Block::MB_DATA,
                        buf,
                        0,
                        0,
                        ACE_Message_Block::DONT_DELETE,
                        0);

    ACE_Message_Block message_block(&db,
                                    ACE_Message_Block::DONT_DELETE,
                                    0);

    const size_t recv_size = message_block.space ();

    const ssize_t n = peer ().recv (message_block.wr_ptr (), recv_size);

    if (n <= 0)
        return n;

    message_block.wr_ptr (n);

    while (message_block.length () > 0)
    {
        if (m_Header.space () > 0)
        {
            //need to receive the header
            const size_t to_header = (message_block.length () > m_Header.space () ? m_Header.space () : message_block.length ());
            m_Header.copy (message_block.rd_ptr (), to_header);
            message_block.rd_ptr (to_header);

            if (m_Header.space () > 0)
            {
                // Couldn't receive the whole header this time.
                ACE_ASSERT (message_block.length () == 0);
                errno = EWOULDBLOCK;
                return -1;
            }
            // We just received nice new header
            if (handle_input_header () == -1)
            {
                ACE_ASSERT ((errno != EWOULDBLOCK) && (errno != EAGAIN));
                return -1;
            }
        }

        // Its possible on some error situations that this happens
        // for example on closing when epoll receives more chunked data and stuff
        // hope this is not hack ,as proper m_RecvWPct is asserted around
        if (!m_RecvWPct)
        {
            sLog->outError ("Forcing close on input m_RecvWPct = NULL");
            errno = EINVAL;
            return -1;
        }

        // We have full read header, now check the data payload
        if (m_RecvPct.space () > 0)
        {
            //need more data in the payload
            const size_t to_data = (message_block.length () > m_RecvPct.space () ? m_RecvPct.space () : message_block.length ());
            m_RecvPct.copy (message_block.rd_ptr (), to_data);
            message_block.rd_ptr (to_data);

            if (m_RecvPct.space () > 0)
            {
                // Couldn't receive the whole data this time.
                ACE_ASSERT (message_block.length () == 0);
                errno = EWOULDBLOCK;
                return -1;
            }
        }

        //just received fresh new payload
        if (handle_input_payload () == -1)
        {
            ACE_ASSERT ((errno != EWOULDBLOCK) && (errno != EAGAIN));
            return -1;
        }
    }

    return n == recv_size ? 1 : 2;
}

int NodeSocket::handle_input_payload (void)
{
    //set errno properly here on error !!!
    // now have a header and payload

    ACE_ASSERT (m_RecvPct.space () == 0);
    ACE_ASSERT (m_Header.space () == 0);
    ACE_ASSERT (m_RecvWPct != NULL);

    const int ret = ProcessIncoming (m_RecvWPct);

    m_RecvPct.base (NULL, 0);
    m_RecvPct.reset ();
    m_RecvWPct = NULL;

    m_Header.reset ();

    if (ret == -1)
        errno = EINVAL;

    return ret;
}

int NodeSocket::handle_input_header (void)
{
    ACE_ASSERT (m_RecvWPct == NULL);

    ACE_ASSERT (m_Header.length() == sizeof(ClientPktHeader));

    m_Crypt.DecryptRecv ((uint8*) m_Header.rd_ptr(), sizeof(ClientPktHeader));

    ClientPktHeader& header = *((ClientPktHeader*) m_Header.rd_ptr());

    EndianConvertReverse(header.size);
    EndianConvert(header.cmd);

    if ((header.size < 4) )//|| (header.size > 10240) || (header.cmd  > 10240))
    {
        /*Player *_player = m_WorldSession ? m_WorldSession->GetPlayer() : NULL;
        sLog->outError ("WorldSocket::handle_input_header(): client (account: %u, char [GUID: %u, name: %s]) sent malformed packet (size: %d , cmd: %d)",
            m_WorldSession ? m_WorldSession->GetAccountId() : 0,
            _player ? _player->GetGUIDLow() : 0,
            _player ? _player->GetName() : "<none>",
            header.size, header.cmd);*/

        errno = EINVAL;
        return -1;
    }

    header.size -= 4;

    ACE_NEW_RETURN (m_RecvWPct, WorldPacket ((Opcodes) header.cmd, header.size), -1);

    if (header.size > 0)
    {
        m_RecvWPct->resize (header.size);
        m_RecvPct.base ((char*) m_RecvWPct->contents(), m_RecvWPct->size());
    }
    else
    {
        ACE_ASSERT(m_RecvPct.space() == 0);
    }

    return 0;
}

int NodeSocket::HandleAuth(WorldPacket &recvPacket)
{
    // Re-check account ban (same check as in realmd)
    QueryResult banresult =
          LoginDatabase.PQuery ("SELECT 1 FROM account_banned WHERE id = %u AND active = 1 ",
                                m_WorldSession->GetAccountId());

    if (banresult) // if account banned
    {
        sLog->outError ("NodeSocket::HandleAuth: recheck Account banned.");
        m_WorldSession->KickPlayer();
        return -1;
    }

    uint32 temp;
    recvPacket >> temp;
    recvPacket >> m_Seed;

    m_WorldSession->SetConnector(this);

    QueryResult result =
          LoginDatabase.PQuery ("SELECT "
                                "username, "                //0
                                "sessionkey, "              //1
                                "last_ip, "                 //2
                                "locked, "                  //3
                                "v, "                       //4
                                "s, "                       //5
                                "expansion, "               //6
                                "mutetime, "                //7
                                "locale, "                  //8
                                "recruiter "                //9
                                "FROM account "
                                "WHERE id = '%u'",
                                m_WorldSession->GetAccountId());

    // Stop if the account is not found
    if (!result)
    {
        sLog->outString ("FAIL:: Auth_by_Node");
        return -1;
    }

    Field* fields = result->Fetch();

    //Auth the Session
    WorldPacket packet (CMSG_AUTH_SESSION);
    packet << fields[0].GetCString();
    packet << m_WorldSession->GetAccountId();
    
    SendPacket(packet);

    if (m_WorldSession->GetPlayer())
    {
        packet.Initialize(CMSG_PLAYER_LOGIN);
        packet << m_WorldSession->GetPlayer()->GetGUID();
        SendPacket(packet);
    }

    return 0;
}
