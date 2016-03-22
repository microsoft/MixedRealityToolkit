//////////////////////////////////////////////////////////////////////////
// NewSessionReply.h
//
// This class wraps the RequestNewSession message (JSON message at the network level).  
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class NewSessionReply
{
public:
	// Successfully created
	NewSessionReply(uint32 sessionID, SessionType sessionType, const std::string& name, const std::string& address, uint16 port);

	// Creation failed
	NewSessionReply(const std::string& failureReason);

	// Load received values
	NewSessionReply(const JSONMessagePtr& message);

	static const std::string& GetMessageType();

	std::string		ToJSONString() const;

	bool			GetResult() const;
	uint32			GetSessionID() const;
	SessionType		GetSessionType() const;
	std::string		GetSessionName() const;
	std::string		GetAddress() const;
	uint16			GetPort() const;

	std::string		GetFailureReason() const;

private:
	JSONMessagePtr	m_message;

	static const std::string gMessageType;
	static const std::string gSessionResult;
	static const std::string gSessionID;
	static const std::string gSessionType;
	static const std::string gSessionName;
	static const std::string gNewSessionAddress;
	static const std::string gNewSessionPort;
	static const std::string gFailureReason;
};

DECLARE_PTR(NewSessionReply)

XTOOLS_NAMESPACE_END