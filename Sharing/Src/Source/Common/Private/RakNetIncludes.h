//////////////////////////////////////////////////////////////////////////
// RakNetIncludes.h
//
// Defines a single point of entry for including the RakNet headers
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

// Defines for RakNet
#define _RAKNET_LIB

// Defines necessary for RakNet to work in WinRT
#if defined(XTOOLS_PLATFORM_WINRT)
# define WINDOWS_STORE_RT
# define _RAKNET_SUPPORT_TCPInterface 0
# define _RAKNET_SUPPORT_PacketizedTCP 0
# define _RAKNET_SUPPORT_EmailSender 0
# define _RAKNET_SUPPORT_HTTPConnection 0
# define _RAKNET_SUPPORT_HTTPConnection2 0
# define _RAKNET_SUPPORT_TelnetTransport 0
# define _RAKNET_SUPPORT_NatTypeDetectionServer 0
# define _RAKNET_SUPPORT_UDPProxyServer 0
# define _RAKNET_SUPPORT_UDPProxyCoordinator 0
# define _RAKNET_SUPPORT_UDPForwarder 0
#endif

#pragma warning(push)
#pragma warning(disable: 4996) // Disable warnings about use of unsafe crt code

#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <PacketLogger.h>
#include <BitStream.h>
#include <RakNetTypes.h>
#include <StringCompressor.h>
#include <PluginInterface2.h>
#include <SignaledEvent.h>

#pragma warning(pop)
