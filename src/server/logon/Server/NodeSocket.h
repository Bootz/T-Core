#ifndef __NODESOCKET_H
#define __NODESOCKET_H

#include <ace/Basic_Types.h>
#include <ace/Synch_Traits.h>
#include <ace/Svc_Handler.h>
#include <ace/SOCK_Stream.h>
#include <ace/SOCK_Connector.h>
#include <ace/Connector.h>
#include <ace/Thread_Mutex.h>
#include <ace/Guard_T.h>
#include <ace/Unbounded_Queue.h>
#include <ace/Message_Block.h>
#include "ace/Reactor_Notification_Strategy.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
#pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#include "Common.h"
#include "AuthCrypt.h"
#include "BigNumber.h"

class ACE_Message_Block;
class WorldPacket;
class WorldSession;

typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> NodeHandler;

class NodeSocket : public NodeHandler
{

	public:
        friend class ACE_Connector< NodeSocket, ACE_SOCK_CONNECTOR >;
		friend class NodeSocketMgr;
        friend class NodeThreadRunnable;
		typedef ACE_Connector< NodeSocket, ACE_SOCK_CONNECTOR > Connector;

        void SetSession(WorldSession *m_Sess) { m_WorldSession = m_Sess; }
        // Called when a timer expires.
		//virtual int handle_timeout (const ACE_Time_Value &current_time, const void *act = 0);

		/// Mutex type used for various synchronizations.
		typedef ACE_Thread_Mutex LockType;
		typedef ACE_Guard<LockType> GuardType;

        /// Check if socket is closed.
        bool IsClosed (void) const;
		/// Close the socket.
        void CloseSocket (void);

		/// Send A packet on the socket, this function is reentrant.
        /// @param pct packet to send
        /// @return -1 of failure
        int SendPacket (const WorldPacket& pct);
		
		const std::string& GetRemoteAddress (void) const;

        /// Add reference to this object.
        long AddReference (void);

        /// Remove reference to this object.
        long RemoveReference (void);

        int RemoveSession(uint32 ID);

        /// Called on failures inside of the acceptor, don't call from your code.
        virtual int close (int);

	protected:
        
        NodeSocket(void);
        ~NodeSocket(void);

		
        virtual int open (void *);
        /// Called when we can read from the socket.
        virtual int handle_input (ACE_HANDLE = ACE_INVALID_HANDLE);

        /// Called when the socket can write.
        virtual int handle_output (ACE_HANDLE = ACE_INVALID_HANDLE);

        /// Called when connection is closed or error happens.
        //virtual int handle_close (ACE_HANDLE = ACE_INVALID_HANDLE,
        //    ACE_Reactor_Mask = ACE_Event_Handler::ALL_EVENTS_MASK);

        /// Called by NodeSocketMgr/NodeThreadRunnable.
        int Update (void);

	private:
        int HandleAuth (WorldPacket& recvPacket);
        /// Helper functions for processing incoming data.
        int handle_input_missing_data (void);
		int handle_input_payload (void);
		int handle_input_header (void);

        /// Drain the queue if its not empty.
        int handle_output_queue (GuardType& g);

		/// process one incoming packet.
        /// @param new_pct received packet ,note that you need to delete it.
        int ProcessIncoming (WorldPacket* new_pct);

        /// Called by ProcessIncoming() on CMSG_PING.
        int HandlePing (WorldPacket& recvPacket);

	private:
		int SetAuthed();
		bool m_authed;

        uint16 m_portnumber; //Its for multiconnections

		ACE_Time_Value m_LastPingTime;

        /// Keep track of over-speed pings ,to prevent ping flood.
        uint32 m_OverSpeedPings;

        /// Address of the remote peer
        std::string m_Address;

        /// Class used for managing encryption of the headers
        AuthCrypt m_Crypt;

        /// Mutex lock to protect m_Session
        LockType m_SessionLock;

        /// Session to which received packets are routed
        WorldSession* m_WorldSession;

        /// here are stored the fragments of the received data
        WorldPacket* m_RecvWPct;

        /// This block actually refers to m_RecvWPct contents,
        /// which allows easy and safe writing to it.
        /// It wont free memory when its deleted. m_RecvWPct takes care of freeing.
        ACE_Message_Block m_RecvPct;

        /// Fragment of the received header.
        ACE_Message_Block m_Header;

        /// Mutex for protecting output related data.
        LockType m_OutBufferLock;

		/// Buffer used for writing output.
        ACE_Message_Block m_OutBuffer;

        /// Size of the m_OutCBuffer.
        size_t m_OutBufferSize;

        /// True if the socket is registered with the reactor for output
        bool m_OutActive;

        uint32 m_Seed;

        typedef UNORDERED_MAP<uint32, WorldSession*> SessionMap;
        SessionMap m_sessions;
        ///The In_Buffer
        ACE_Based::LockedQueue<WorldPacket*, ACE_Thread_Mutex> _inPackBuff;
        int WriteBuffer();

        enum { ITERATIONS = 5 };
        int iterations_;
        ACE_Reactor_Notification_Strategy notifier_;
};

#endif
