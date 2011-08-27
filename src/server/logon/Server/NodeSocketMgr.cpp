#include <ace/ACE.h>
#include <ace/Log_Msg.h>
#include <ace/Reactor.h>
#include <ace/Reactor_Impl.h>
#include <ace/TP_Reactor.h>
#include <ace/Dev_Poll_Reactor.h>
#include <ace/Guard_T.h>
#include <ace/Atomic_Op.h>
#include <ace/os_include/arpa/os_inet.h>
#include <ace/os_include/netinet/os_tcp.h>
#include <ace/os_include/sys/os_types.h>
#include <ace/os_include/sys/os_socket.h>

#include <set>

#include "Log.h"
#include "Common.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "NodeSocket.h"
#include "SharedDefines.h"

#include "NodeSocketMgr.h"
/**
* This is a helper class to WorldSocketMgr ,that manages
* network threads, and assigning connections from acceptor thread
* to other network threads
*/

/****************************************************************\
|******** PLEASE REMEMBER, MOST THINGS HERE OUT OF LOGIC ********|
|*** THIS FILE AND ROUTINGHELPER CONTAINING THE ROUTING LOGIC ***|
\****************************************************************/

class NodeThreadRunnable : protected ACE_Task_Base
{
    public:

        NodeThreadRunnable() :
            m_ThreadId(-1),
            m_Connections(0),
            m_Reactor(0)
        {
            ACE_Reactor_Impl* imp = 0;

            #if defined (ACE_HAS_EVENT_POLL) || defined (ACE_HAS_DEV_POLL)

            imp = new ACE_Dev_Poll_Reactor();

            imp->max_notify_iterations (128);
            imp->restart (1);

            #else

            imp = new ACE_TP_Reactor();
            imp->max_notify_iterations (128);

            #endif
            m_Reactor = new ACE_Reactor (imp, 1);
        }

        virtual ~NodeThreadRunnable()
        {
            Stop();
            Wait();

            if (m_Reactor)
                delete m_Reactor;
        }

        void Stop()
        {
            m_Reactor->end_reactor_event_loop();
        }

        int Start()
        {
            if (m_ThreadId != -1)
                return -1;
            return (m_ThreadId = activate());
        }

        long Connections()
        {
            return static_cast<long> (m_Connections.value());
        }

        int AddSocket (NodeSocket* sock)
        {
            ACE_GUARD_RETURN (ACE_Thread_Mutex, Guard, m_NewSockets_Lock, -1);

            ++m_Connections;
            sock->AddReference();
            sock->reactor (m_Reactor);
            sock->notifier_.reactor (m_Reactor);
            sock->msg_queue ()->notification_strategy (&sock->notifier_);
            //sock->reactor ()->schedule_timer (sock, 0, ACE_Time_Value::zero, iter_delay);
            m_NewSockets.insert (sock);

            return 0;
        }

        void Wait() { /*ACE_Task_Base::wait();*/ }

        ACE_Reactor* GetReactor()
        {
            return m_Reactor;
        }

    protected:

        void AddNewSockets()
        {
            ACE_GUARD (ACE_Thread_Mutex, Guard, m_NewSockets_Lock);

            if (m_NewSockets.empty())
                return;

            for (SocketSet::const_iterator i = m_NewSockets.begin(); i != m_NewSockets.end(); ++i)
            {
                NodeSocket* sock = (*i);
                if (sock->IsClosed())
                {
                    sock->RemoveReference();
                    --m_Connections;
                }
                else
                    m_Sockets.insert (sock);
            }

            m_NewSockets.clear();
        }

        virtual int svc()
        {
            sLog->outStaticDebug ("Connector Network Thread Starting");

            ACE_ASSERT (m_Reactor);

            SocketSet::iterator i, t;

            while (!m_Reactor->reactor_event_loop_done())
            {
                // dont be too smart to move this outside the loop
                // the run_reactor_event_loop will modify interval
                ACE_Time_Value interval (0, 10000);

                if (m_Reactor->run_reactor_event_loop (interval) == -1)
                    break;

                AddNewSockets();

                for (i = m_Sockets.begin(); i != m_Sockets.end();)
                {
                    if ((*i)->Update() == -1)
                    {
                        t = i;
                        ++i;

                        (*t)->CloseSocket();
                        (*t)->RemoveReference();
                        --m_Connections;
                        m_Sockets.erase (t);
                    }
                    else
                        ++i;
                }
            }

            sLog->outStaticDebug ("Connector Network Thread Exitting");

            return 0;
        }

    private:
        typedef ACE_Atomic_Op<ACE_SYNCH_MUTEX, long> AtomicInt;
        typedef std::set<NodeSocket*> SocketSet;

        NodeSocket sock;

        ACE_Reactor* m_Reactor;
        AtomicInt m_Connections;
        int m_ThreadId;

        SocketSet m_Sockets;
        SocketSet m_NewSockets;
        
        ACE_Thread_Mutex m_NewSockets_Lock;
};


NodeSocketMgr::NodeSocketMgr() :
    m_NetThreadsCount(0),
    m_NetThreads(0),
    m_SockOutKBuff(-1),
    m_SockOutUBuff(65536),
    m_UseNoDelay(true),
    m_Connector (0)
{
}

NodeSocketMgr::~NodeSocketMgr()
{
    if (m_NetThreads)
        delete [] m_NetThreads;

    if (m_Connector)
        delete m_Connector;
}

int
NodeSocketMgr::StartReactiveIO ()
{
    m_UseNoDelay = sConfig->GetBoolDefault ("Network.TcpNodelay", true);
    int num_threads = sConfig->GetIntDefault ("Network.Threads", 1);
    if (num_threads <= 0)
    {
        sLog->outError ("Network.Threads is wrong in your config file");
        return -1;
    }

    m_NetThreadsCount = static_cast<size_t> (num_threads + 1);
    m_NetThreads = new NodeThreadRunnable[m_NetThreadsCount];
    sLog->outBasic ("Max allowed connections %d", ACE::max_handles());

    // -1 means use default
    m_SockOutKBuff = sConfig->GetIntDefault ("Network.OutKBuff", -1);
    m_SockOutUBuff = sConfig->GetIntDefault ("Network.OutUBuff", 65536);
    if (m_SockOutUBuff <= 0)
    {
        sLog->outError ("Network.OutUBuff is wrong in your config file");
        return -1;
    }

    for (size_t i = 0; i < m_NetThreadsCount; ++i)
       m_NetThreads[i].Start();

    return 0;
}

int
NodeSocketMgr::StartNetwork ()
{
    if (!sLog->IsOutDebug())
        ACE_Log_Msg::instance()->priority_mask (LM_ERROR, ACE_Log_Msg::PROCESS);

    if (StartReactiveIO() == -1)
        return -1;

    return 0;
}

int
NodeSocketMgr::OnSocketOpen (NodeSocket* sock)
{
    // set some options here
    if (m_SockOutKBuff >= 0)
    {
        if (sock->peer().set_option (SOL_SOCKET,
            SO_SNDBUF,
            (void*) & m_SockOutKBuff,
            sizeof (int)) == -1 && errno != ENOTSUP)
        {
            sLog->outError ("NodeSocketMgr::OnSocketOpen set_option SO_SNDBUF");
            return -1;
        }
    }

    static const int ndoption = 1;

    // Set TCP_NODELAY.
    if (m_UseNoDelay)
    {
        if (sock->peer().set_option (ACE_IPPROTO_TCP,
            TCP_NODELAY,
            (void*)&ndoption,
            sizeof (int)) == -1)
        {
            sLog->outError ("NodeSocketMgr::OnSocketOpen: peer().set_option TCP_NODELAY errno = %s", ACE_OS::strerror (errno));
            return -1;
        }
    }

    sock->m_OutBufferSize = static_cast<size_t> (m_SockOutUBuff);

    // we skip the Connector Thread
    size_t min = 1;

    ACE_ASSERT (m_NetThreadsCount >= 1);

    for (size_t i = 1; i < m_NetThreadsCount; ++i)
        if (m_NetThreads[i].Connections() < m_NetThreads[min].Connections())
            min = i;

    return m_NetThreads[min].AddSocket (sock);
}

bool NodeSocketMgr::OpenConnection (uint32 nodeid, WorldSession *m_Session)
{
    //Check if ID exists or online
    NodeListList::const_iterator iter = m_nodelist.find(nodeid);
    if (iter == m_nodelist.end() || !CheckNodeID(nodeid))
        return false;

    ACE_INET_Addr port_to_connect (uint16(iter->second.port), iter->second.IP.c_str() );
    //ACE_Connector<NodeSocket, ACE_SOCK_CONNECTOR> con;

    NodeSocket::Connector *con = new NodeSocket::Connector;
    NodeSocket *sock = new NodeSocket;
    //m_Connector = con;
    if(con->open(m_NetThreads[0].GetReactor(),ACE_NONBLOCK) != -1)
        if(con->connect(sock, port_to_connect) == -1)
        {
            LogonDatabase.DirectPExecute("UPDATE nodelist SET Online = 0 WHERE NodeID = '%d'", nodeid);
            RefreshOnlineStat();
            return false;
        }
        else
        {
            m_Session->SetNodeID(nodeid);
            sock->SetSession(m_Session);
        }

    return true;
}

void
NodeSocketMgr::StopNetwork()
{
    if (m_NetThreadsCount != 0)
    {
        for (size_t i = 0; i < m_NetThreadsCount; ++i)
            m_NetThreads[i].Stop();
    }

    Wait();
}

void
NodeSocketMgr::Wait()
{
    if (m_NetThreadsCount != 0)
    {
        for (size_t i = 0; i < m_NetThreadsCount; ++i)
            m_NetThreads[i].Wait();
    }
}

void NodeSocketMgr::LoadNodeList()
{
    QueryResult result = LogonDatabase.Query( "SELECT * FROM nodelist" );

    int count = 0;
    if( !result )
    {
        sLog->outString();
        sLog->outString( ">> Loaded %u Nodes ", count );
        return;
    }

    do
    {
        NodeList m_nodes;

        Field *fields = result->Fetch();
        uint32 NodeID = fields[0].GetUInt32();
        m_nodes.name = fields[1].GetString();
        m_nodes.IP   = fields[2].GetString();
        m_nodes.port = fields[3].GetInt32();
        m_nodes.online = fields[4].GetBool();


        m_nodelist[NodeID] = m_nodes;

        ++count;

    }while (result->NextRow());

    sLog->outString();
    sLog->outString( ">> Loaded %u Nodes", count );
}

bool NodeSocketMgr::CheckNodeID(uint32 NodeID)
{
    NodeListList::const_iterator iter = m_nodelist.find(NodeID);

    if (iter == m_nodelist.end())
        return false;
    else 
    {
        if (iter->second.online)
            return true;
    }

    return false;
}

uint32 NodeSocketMgr::GetFirstNode()
{
    NodeListList::const_iterator iter = m_nodelist.begin();
    return iter->first;
}

void NodeSocketMgr::RefreshOnlineStat()
{
    NodeListList::iterator iter = m_nodelist.begin();
    for (; iter != m_nodelist.end(); ++iter)
    {
        if(!iter->second.online)
        {
            QueryResult result = LogonDatabase.PQuery( "SELECT * FROM nodelist WHERE NodeID = %u;", iter->first );
            Field *fields = result->Fetch();

            m_nodelist[iter->first].online = fields[4].GetBool();
        }
    }
}


NodeSocketMgr*
NodeSocketMgr::Instance()
{
    return ACE_Singleton<NodeSocketMgr,ACE_Thread_Mutex>::instance();
}

