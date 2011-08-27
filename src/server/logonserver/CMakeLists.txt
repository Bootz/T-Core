# Copyright (C) 2011 TrilliumEMU <http://www.trilliumemu.org/>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

file(GLOB_RECURSE sources_CommandLine CommandLine/*.cpp CommandLine/*.h)
file(GLOB_RECURSE sources_RemoteAccess RemoteAccess/*.cpp RemoteAccess/*.h)
file(GLOB_RECURSE sources_TCSoap TCSoap/*.cpp TCSoap/*.h)
file(GLOB_RECURSE sources_WorldThread WorldThread/*.cpp WorldThread/*.h)
file(GLOB sources_localdir *.cpp *.h)

if( USE_COREPCH )
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
endif()

if ( USE_COREPCH AND MSVC )
  set(logonserver_SRCS
    PrecompiledHeaders/logonserverPCH.cpp
    PrecompiledHeaders/logonserverPCH.h
  )
endif()

set(logonserver_SRCS
  ${logonserver_SRCS}
  ${sources_CommandLine}
  ${sources_RemoteAccess}
  ${sources_TCSoap}
  ${sources_WorldThread}
  ${sources_localdir}
)

if( WIN32 )
  set(logonserver_SRCS
    ${logonserver_SRCS}
    ${sources_Debugging}
    worldserver.rc
  )
endif()

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/dep/gsoap
  ${CMAKE_SOURCE_DIR}/dep/sockets/include
  ${CMAKE_SOURCE_DIR}/dep/SFMT
  ${CMAKE_SOURCE_DIR}/dep/mersennetwister
  ${CMAKE_SOURCE_DIR}/src/server/collision
  ${CMAKE_SOURCE_DIR}/src/server/collision/Management
  ${CMAKE_SOURCE_DIR}/src/server/shared
  ${CMAKE_SOURCE_DIR}/src/server/shared/Configuration
  ${CMAKE_SOURCE_DIR}/src/server/shared/Cryptography
  ${CMAKE_SOURCE_DIR}/src/server/shared/Cryptography/Authentication
  ${CMAKE_SOURCE_DIR}/src/server/shared/Database
  ${CMAKE_SOURCE_DIR}/src/server/shared/DataStores
  ${CMAKE_SOURCE_DIR}/src/server/shared/Debugging
  ${CMAKE_SOURCE_DIR}/src/server/shared/Dynamic/CountedReference
  ${CMAKE_SOURCE_DIR}/src/server/shared/Dynamic/LinkedReference
  ${CMAKE_SOURCE_DIR}/src/server/shared/Dynamic
  ${CMAKE_SOURCE_DIR}/src/server/shared/Logging
  ${CMAKE_SOURCE_DIR}/src/server/shared/Packets
  ${CMAKE_SOURCE_DIR}/src/server/shared/Policies
  ${CMAKE_SOURCE_DIR}/src/server/shared/Threading
  ${CMAKE_SOURCE_DIR}/src/server/shared/Utilities
  ${CMAKE_SOURCE_DIR}/src/server/logon
  ${CMAKE_SOURCE_DIR}/src/server/logon/Accounts
  ${CMAKE_SOURCE_DIR}/src/server/logon/Achievements
  ${CMAKE_SOURCE_DIR}/src/server/logon/Addons
  ${CMAKE_SOURCE_DIR}/src/server/logon/AI
  ${CMAKE_SOURCE_DIR}/src/server/logon/AI/CoreAI
  ${CMAKE_SOURCE_DIR}/src/server/logon/AI/EventAI
  ${CMAKE_SOURCE_DIR}/src/server/logon/AI/ScriptedAI
  ${CMAKE_SOURCE_DIR}/src/server/logon/AI/SmartScripts
  ${CMAKE_SOURCE_DIR}/src/server/logon/AuctionHouse
  ${CMAKE_SOURCE_DIR}/src/server/logon/AuctionHouse/AuctionHouseBot
  ${CMAKE_SOURCE_DIR}/src/server/logon/Battlegrounds
  ${CMAKE_SOURCE_DIR}/src/server/logon/Battlegrounds/Zones
  ${CMAKE_SOURCE_DIR}/src/server/logon/Calendar
  ${CMAKE_SOURCE_DIR}/src/server/logon/Chat
  ${CMAKE_SOURCE_DIR}/src/server/logon/Chat/Channels
  ${CMAKE_SOURCE_DIR}/src/server/logon/Chat/Commands
  ${CMAKE_SOURCE_DIR}/src/server/logon/Combat
  ${CMAKE_SOURCE_DIR}/src/server/logon/Conditions
  ${CMAKE_SOURCE_DIR}/src/server/logon/DataStores
  ${CMAKE_SOURCE_DIR}/src/server/logon/DungeonFinding
  ${CMAKE_SOURCE_DIR}/src/server/logon/Entities
  ${CMAKE_SOURCE_DIR}/src/server/logon/Entities/Creature
  ${CMAKE_SOURCE_DIR}/src/server/logon/Entities/Corpse
  ${CMAKE_SOURCE_DIR}/src/server/logon/Entities/DynamicObject
  ${CMAKE_SOURCE_DIR}/src/server/logon/Entities/GameObject
  ${CMAKE_SOURCE_DIR}/src/server/logon/Entities/Item
  ${CMAKE_SOURCE_DIR}/src/server/logon/Entities/Item/Container
  ${CMAKE_SOURCE_DIR}/src/server/logon/Entities/Object
  ${CMAKE_SOURCE_DIR}/src/server/logon/Entities/Object/Updates
  ${CMAKE_SOURCE_DIR}/src/server/logon/Entities/Pet
  ${CMAKE_SOURCE_DIR}/src/server/logon/Entities/Player
  ${CMAKE_SOURCE_DIR}/src/server/logon/Entities/Totem
  ${CMAKE_SOURCE_DIR}/src/server/logon/Entities/Unit
  ${CMAKE_SOURCE_DIR}/src/server/logon/Entities/Vehicle
  ${CMAKE_SOURCE_DIR}/src/server/logon/Entities/Transport
  ${CMAKE_SOURCE_DIR}/src/server/logon/Events
  ${CMAKE_SOURCE_DIR}/src/server/logon/Globals
  ${CMAKE_SOURCE_DIR}/src/server/logon/Grids/Cells
  ${CMAKE_SOURCE_DIR}/src/server/logon/Grids/Notifiers
  ${CMAKE_SOURCE_DIR}/src/server/logon/Grids
  ${CMAKE_SOURCE_DIR}/src/server/logon/Groups
  ${CMAKE_SOURCE_DIR}/src/server/logon/Guilds
  ${CMAKE_SOURCE_DIR}/src/server/logon/Instances
  ${CMAKE_SOURCE_DIR}/src/server/logon/Loot
  ${CMAKE_SOURCE_DIR}/src/server/logon/Mails
  ${CMAKE_SOURCE_DIR}/src/server/logon/Maps
  ${CMAKE_SOURCE_DIR}/src/server/logon/Miscellaneous
  ${CMAKE_SOURCE_DIR}/src/server/logon/Movement
  ${CMAKE_SOURCE_DIR}/src/server/logon/Movement/MovementGenerators
  ${CMAKE_SOURCE_DIR}/src/server/logon/Movement/Waypoints
  ${CMAKE_SOURCE_DIR}/src/server/logon/OutdoorPvP
  ${CMAKE_SOURCE_DIR}/src/server/logon/Pools
  ${CMAKE_SOURCE_DIR}/src/server/logon/PrecompiledHeaders
  ${CMAKE_SOURCE_DIR}/src/server/logon/Quests
  ${CMAKE_SOURCE_DIR}/src/server/logon/Reputation
  ${CMAKE_SOURCE_DIR}/src/server/logon/Scripting
  ${CMAKE_SOURCE_DIR}/src/server/logon/Server/Protocol
  ${CMAKE_SOURCE_DIR}/src/server/logon/Server/Packets
  ${CMAKE_SOURCE_DIR}/src/server/logon/Server/Protocol/Handlers
  ${CMAKE_SOURCE_DIR}/src/server/logon/Server
  ${CMAKE_SOURCE_DIR}/src/server/logon/Skills
  ${CMAKE_SOURCE_DIR}/src/server/logon/Spells
  ${CMAKE_SOURCE_DIR}/src/server/logon/Spells/Auras
  ${CMAKE_SOURCE_DIR}/src/server/logon/Tools
  ${CMAKE_SOURCE_DIR}/src/server/logon/World
  ${CMAKE_SOURCE_DIR}/src/server/authserver/Server
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/CommandLine
  ${CMAKE_CURRENT_SOURCE_DIR}/RemoteAccess
  ${CMAKE_CURRENT_SOURCE_DIR}/TCSoap
  ${CMAKE_CURRENT_SOURCE_DIR}/WorldThread
  ${ACE_INCLUDE_DIR}
  ${MYSQL_INCLUDE_DIR}
  ${OPENSSL_INCLUDE_DIR}
)

set(logonserver_LINK_FLAGS "")

add_executable(logonserver ${logonserver_SRCS})

if( NOT WIN32 )
  add_definitions(-D_TRILLIUM_CORE_CONFIG='"${CONF_DIR}/logonserver.conf"')
endif()

add_dependencies(logonserver revision.h)

if( UNIX )
  set(logonserver_LINK_FLAGS "-pthread -lncurses ${logonserver_LINK_FLAGS}")
endif()

set_target_properties(logonserver PROPERTIES LINK_FLAGS "${logonserver_LINK_FLAGS}")

target_link_libraries(logonserver
  logon
  shared
  scripts
  collision
  g3dlib
  gsoap
  ${JEMALLOC_LIBRARY}
  ${READLINE_LIBRARY}
  ${TERMCAP_LIBRARY}
  ${ACE_LIBRARY}
  ${MYSQL_LIBRARY}
  ${OPENSSL_LIBRARIES}
  ${OPENSSL_EXTRA_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${OSX_LIBS}
)

if( WIN32 )
  add_custom_command(TARGET logonserver
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/logonserver.conf.dist ${CMAKE_BINARY_DIR}/bin/$(ConfigurationName)/
  )
endif()

if( UNIX )
  install(TARGETS logonserver DESTINATION bin)
  install(FILES logonserver.conf.dist DESTINATION etc)
  install(FILES innormative_words.txt.dist DESTINATION bin)
  install(FILES letter_analogs.txt.dist DESTINATION bin)
elseif( WIN32 )
  install(TARGETS logonserver DESTINATION "${CMAKE_INSTALL_PREFIX}")
  install(FILES logonserver.conf.dist DESTINATION "${CMAKE_INSTALL_PREFIX}")
  install(FILES innormative_words.txt.dist DESTINATION "${CMAKE_INSTALL_PREFIX}")
  install(FILES letter_analogs.txt.dist DESTINATION "${CMAKE_INSTALL_PREFIX}")
endif()

# Generate precompiled header
if( USE_COREPCH )
  if(CMAKE_COMPILER_IS_GNUCXX)
    add_precompiled_header(logonserver ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders/logonserverPCH.h)
  elseif(MSVC)
    add_native_precompiled_header(logonserver ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders/logonserverPCH)
  endif()
endif()

