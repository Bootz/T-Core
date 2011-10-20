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

#include <ace/Dev_Poll_Reactor.h>
#include <ace/TP_Reactor.h>
#include <ace/ACE.h>
#include <ace/Sig_Handler.h>
#include <openssl/opensslv.h>
#include <openssl/crypto.h>

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "Configuration/Config.h"
#include "LogMgr.h"
#include "SystemConfig.h"
#include "Util.h"
#include "SignalHandler.h"
#include "RealmList.h"
#include "RealmAcceptor.h"

#ifndef _AUTHSERVER_CONFIG
# define _AUTHSERVER_CONFIG  "authserver.conf"
#endif

bool StartDB();
void StopDB();

bool stopEvent = false;                                     // Setting it to true stops the server

LoginDatabaseWorkerPool LoginDatabase;                      // Accessor to the auth server database

// Handle authserver's termination signals
class AuthServerSignalHandler : public Trillium::SignalHandler
{
public:
    virtual void HandleSignal(int SigNum)
    {
        switch (SigNum)
        {
        case SIGINT:
        case SIGTERM:
            stopEvent = true;
            break;
        }
    }
};

/// Print out the usage string for this program on the console.
void usage(const char *prog)
{
    sLogMgr->WriteLn(SERVER_LOG, LOGL_STRING, "Usage: \n %s [<options>]\n"
        "    -c config_file           use config_file as configuration file\n\r",
        prog);
}

// Launch the auth server
extern int main(int argc, char **argv)
{
    sLogMgr->ResetLogDb();
    // Command line parsing to get the configuration file name
    char const *cfg_file = _AUTHSERVER_CONFIG;
    int c = 1;
    while(c < argc)
    {
        if (strcmp(argv[c], "-c") == 0)
        {
            if (++c >= argc)
            {
                sLogMgr->WriteLn(SERVER_LOG, LOGL_ERROR, "Runtime-Error: -c option requires an input argument");
                usage(argv[0]);
                return 1;
            }
            else
                cfg_file = argv[c];
        }
        ++c;
    }

    if (!sConfig->SetSource(cfg_file))
    {
        sLogMgr->WriteLn(SERVER_LOG, LOGL_ERROR, "Invalid or missing configuration file : %s", cfg_file);
        sLogMgr->WriteLn(SERVER_LOG, LOGL_ERROR, "Verify that the file exists and has \'[authserver]\' written in the top of the file!");
        return 1;
    }
    sLogMgr->Initialize();
    sLogMgr->WriteLn(SERVER_LOG, LOGL_STRING, "%s (authserver)", _FULLVERSION);
    sLogMgr->WriteLn(SERVER_LOG, LOGL_STRING, "<Ctrl-C> to stop.\n");
    sLogMgr->WriteLn(SERVER_LOG, LOGL_STRING, "Using configuration file %s.", cfg_file);

    sLogMgr->WriteLn(SERVER_LOG, LOGL_INFO, "%s (Library: %s)", OPENSSL_VERSION_TEXT, SSLeay_version(SSLEAY_VERSION));

#if defined (ACE_HAS_EVENT_POLL) || defined (ACE_HAS_DEV_POLL)
    ACE_Reactor::instance(new ACE_Reactor(new ACE_Dev_Poll_Reactor(ACE::max_handles(), 1), 1), true);
#else
    ACE_Reactor::instance(new ACE_Reactor(new ACE_TP_Reactor(), true), true);
#endif

    sLogMgr->WriteLn(SERVER_LOG, LOGL_WARNING, "Max allowed open files is %d", ACE::max_handles());

    // authserver PID file creation
    std::string pidfile = sConfig->GetStringDefault("PidFile", "");
    if (!pidfile.empty())
    {
        uint32 pid = CreatePIDFile(pidfile);
        if (!pid)
        {
            sLogMgr->WriteLn(SERVER_LOG, LOGL_ERROR, "Cannot create PID file %s.\n", pidfile.c_str());
            return 1;
        }

        sLogMgr->WriteLn(SERVER_LOG, LOGL_STRING, "Daemon PID: %u\n", pid);
    }

    // Initialize the database connection
    if (!StartDB())
        return 1;

    // Initialize the log database
    sLogMgr->ResetLogDb();
    sLogMgr->SetRealmId(0);

    // Get the list of realms for the server
    sRealmList->Initialize(sConfig->GetIntDefault("RealmsStateUpdateDelay", 20));
    if (sRealmList->size() == 0)
    {
        sLogMgr->WriteLn(SERVER_LOG, LOGL_ERROR, "No valid realms specified.");
        return 1;
    }

    // Launch the listening network socket
    RealmAcceptor acceptor;

    uint16 rmport = sConfig->GetIntDefault("RealmServerPort", 3724);
    std::string bind_ip = sConfig->GetStringDefault("BindIP", "0.0.0.0");

    ACE_INET_Addr bind_addr(rmport, bind_ip.c_str());

    if (acceptor.open(bind_addr, ACE_Reactor::instance(), ACE_NONBLOCK) == -1)
    {
        sLogMgr->WriteLn(SERVER_LOG, LOGL_ERROR, "Auth server can not bind to %s:%d", bind_ip.c_str(), rmport);
        return 1;
    }

    // Initialise the signal handlers
    AuthServerSignalHandler SignalINT, SignalTERM;

    // Register authservers's signal handlers
    ACE_Sig_Handler Handler;
    Handler.register_handler(SIGINT, &SignalINT);
    Handler.register_handler(SIGTERM, &SignalTERM);

    ///- Handle affinity for multiple processors and process priority on Windows
#ifdef _WIN32
    {
        HANDLE hProcess = GetCurrentProcess();

        uint32 Aff = sConfig->GetIntDefault("UseProcessors", 0);
        if (Aff > 0)
        {
            ULONG_PTR appAff;
            ULONG_PTR sysAff;

            if (GetProcessAffinityMask(hProcess, &appAff, &sysAff))
            {
                ULONG_PTR curAff = Aff & appAff;            // remove non accessible processors

                if (!curAff)
                    sLogMgr->WriteLn(SERVER_LOG, LOGL_ERROR, "Processors marked in UseProcessors bitmask (hex) %x not accessible for authserver. Accessible processors bitmask (hex): %x", Aff, appAff);
                else if (SetProcessAffinityMask(hProcess, curAff))
                    sLogMgr->WriteLn(SERVER_LOG, LOGL_STRING, "Using processors (bitmask, hex): %x", curAff);
                else
                    sLogMgr->WriteLn(SERVER_LOG, LOGL_ERROR, "Can't set used processors (hex): %x", curAff);
            }
            sLogMgr->WriteLn(SERVER_LOG, LOGL_STRING, std::string());
        }

        bool Prio = sConfig->GetBoolDefault("ProcessPriority", false);

        if (Prio)
        {
            if (SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS))
                sLogMgr->WriteLn(SERVER_LOG, LOGL_STRING, "The auth server process priority class has been set to HIGH");
            else
                sLogMgr->WriteLn(SERVER_LOG, LOGL_ERROR, "Can't set auth server process priority class.");
            sLogMgr->WriteLn(SERVER_LOG, LOGL_STRING, std::string());
        }
    }
#endif

    sLog->outString("%s (authserver-daemon) ready...", _FULLVERSION);

    // maximum counter for next ping
    uint32 numLoops = (sConfig->GetIntDefault("MaxPingTime", 30) * (MINUTE * 1000000 / 100000));
    uint32 loopCounter = 0;

    // possibly enable db logging; avoid massive startup spam by doing it here.
    sLogMgr->SetLogDb();

    // Wait for termination signal
    while (!stopEvent)
    {
        // dont move this outside the loop, the reactor will modify it
        ACE_Time_Value interval(0, 100000);

        if (ACE_Reactor::instance()->run_reactor_event_loop(interval) == -1)
            break;

        if ((++loopCounter) == numLoops)
        {
            loopCounter = 0;
            sLogMgr->WriteLn(SERVER_LOG, LOGL_INFO, "Ping MySQL to keep connection alive");
            LoginDatabase.KeepAlive();
        }
    }

    // Close the Database Pool and library
    StopDB();

    sLogMgr->WriteLn(SERVER_LOG, LOGL_STRING, "Halting process...");
    return 0;
}

// Initialize connection to the database
bool StartDB()
{
    MySQL::Library_Init();

    std::string dbstring = sConfig->GetStringDefault("LoginDatabaseInfo", "");
    if (dbstring.empty())
    {
        sLogMgr->WriteLn(SERVER_LOG, LOGL_ERROR, "Database not specified");
        return false;
    }

    uint8 worker_threads = sConfig->GetIntDefault("LoginDatabase.WorkerThreads", 1);
    if (worker_threads < 1 || worker_threads > 32)
    {
        sLogMgr->WriteLn(SERVER_LOG, LOGL_ERROR, "Improper value specified for LoginDatabase.WorkerThreads, defaulting to 1.");
        worker_threads = 1;
    }

    uint8 synch_threads = sConfig->GetIntDefault("LoginDatabase.SynchThreads", 1);
    if (synch_threads < 1 || synch_threads > 32)
    {
        sLogMgr->WriteLn(SERVER_LOG, LOGL_ERROR, "Improper value specified for LoginDatabase.SynchThreads, defaulting to 1.");
        synch_threads = 1;
    }

    // NOTE: While authserver is singlethreaded you should keep synch_threads == 1. Increasing it is just silly since only 1 will be used ever.
    if (!LoginDatabase.Open(dbstring.c_str(), worker_threads, synch_threads))
    {
        sLogMgr->WriteLn(SERVER_LOG, LOGL_ERROR, "Cannot connect to database");
        return false;
    }

    return true;
}

void StopDB()
{
    LoginDatabase.Close();
    MySQL::Library_End();
}
