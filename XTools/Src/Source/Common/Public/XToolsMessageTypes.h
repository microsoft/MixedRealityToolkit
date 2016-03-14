//////////////////////////////////////////////////////////////////////////
// XToolsMessageTypes.h
//
// Defines the different options that can be used when sending messages 
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

/// Define the IDs of all the network messages used by XTools internally here.
/// The message ID is used as the first byte of the network message payload to
/// identify what is in the rest of the payload.  
/// Its important to make sure that these IDs do not overlap with the IDs used
/// internally by RakNet or by those used by consumers of these APIs.  
enum MessageID : byte
{
	///////////////////////////////////////
	// DO NOT CHANGE!!!
	Start = 134,		/// This MUST match RakNet's ID_USER_PACKET_ENUM to avoid conflicts.  
						/// Enforced by a static assert within the Common lib, so as not to 
						/// expose RakNet APIs outside of Common

	///////////////////////////////////////
	// Ok to change these
	StatusOnly = Start,		/// Use this only if you want to know about connects and disconnects but don't want to pass messages
	Broadcast,				/// The message should be forwarded to all other users
    SendTo,					/// The message should be forward to a particular user that is not the one that we are directly connected to
    SessionControl,			/// Session related messages. 
	MouseXToClient,
	MouseXToServer,
	SyncMessage,			/// Messages related to the sync system
	Tunnel,					/// Tunneled message
	TunnelControl,			/// Tunneling state updates
	AudioSamples,
	Handshake,				/// Messages related to the network handshake when connecting
    UserPresenceChange,
	AvatarBroadcast,
	TestAutomation,			/// Messages used for coordinating multiple remote XTools devices for automated testing
	Profiling,
	InternalSyncMessage,	/// Messages used by the internal sync system
	RoomAnchor,				/// Messages related to sharing room anchors

	///////////////////////////////////////
	// DO NOT CHANGE!!!
	UserMessageIDStart = 134 + 50	/// Reserve 50 message IDs for XTools internal use.  User code should use IDs from UserMessageIDStart to 255
};


/// Define the priority of the message relative to others.  Messages with higher priority will be sent sooner than lower priority messages
enum MessagePriority : byte
{
	Immediate = 0,	/// These message trigger sends immediately, and are generally not buffered or aggregated into a single datagram.
	
	/// Messages at this priority and lower are buffered to be sent in groups at 10 millisecond intervals to reduce UDP overhead and better measure congestion control. 
	High,			/// For every 2 Immediate priority messages, 1 High priority will be sent.
	Medium,			/// For every 2 High priority messages, 1 Medium priority will be sent.
	Low				/// For every 2 Medium priority messages, 1 Low priority will be sent.
};


/// Used to specify how hard the system should try to ensure that the messages arrive and arrive in order
enum MessageReliability : byte
{
	Unreliable = 0,			/// Same as regular UDP, except that it will also discard duplicate datagrams.  
	UnreliableSequenced,	/// Messages in the same channel will arrive in the sequence you sent it, but are not guaranteed to arrive.  Out or order messages will be dropped. 
	Reliable,				/// The message is sent reliably, but not necessarily in any order.  
	ReliableOrdered,		/// Message is reliable and will arrive in the order you sent it with other messages in the channel.  Messages will be delayed while waiting for out of order messages. 
	ReliableSequenced		/// Message is reliable and will arrive in the sequence you sent it with other messages in the channel.  Out or order messages will be dropped. 
};


/// Messages in the same channel sent with an ordered or sequenced level of reliability will arrive in the order sent.  
/// XTools internal ones are defined here; user-defined channels should start at UserMessageChannelStart.  
/// Note that ordered messages in different channels can arrive in a different order from each other.  This can create hard to 
/// find bugs, so only used channels other than Default for messages that really require it.  
enum MessageChannel : byte
{
	///////////////////////////////////////
	// DO NOT CHANGE!!!
	Default = 0,	/// The main channel for data

	///////////////////////////////////////
	// Ok to change these
	Mouse,				/// Mouse position updates
	Avatar,				/// Traffic related to avatar position and movement
	Audio,				/// Voice audio traffic
	ProfileChannel,		/// Traffic with updates for the profiler
	RoomAnchorChannel,	/// Traffic related to uploading and downloading anchor data

	///////////////////////////////////////
	// DO NOT CHANGE!!!
	UserMessageChannelStart = 16,	/// Reserve 16 channels for XTools internal use.  User code should use IDs from UserMessageChannelStart to MessageChannelMax
	MessageChannelMax = 31			/// The maximum allowed value that can be used for message channels
};


/// Define the role that the client should take.  This defines the connection behavior of the client.  
enum ClientRole : byte
{
	/// A Primary client 
	/// - Connects directly to the session server itself
	/// - Is responsible for joining and leaving sessions
	/// - Can tunnel the network traffic of a secondary client
	/// - Has sync auth level Medium
	Primary = 0,

	/// A Secondary client
	/// - Connects to a Primary client, which tunnels its network traffic to the server
	/// - Is not able to join or leave sessions on its own. 
	/// - Has sync auth level Low
	Secondary,

	/// The role of the client is not defined.  
	/// Used by the NetworkConnection.SendTo() method to say that the message should be sent to 
	/// both the primary and secondary clients of the user
	Unspecified = 255
};

XTOOLS_NAMESPACE_END
