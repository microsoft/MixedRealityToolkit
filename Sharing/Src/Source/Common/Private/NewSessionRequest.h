//////////////////////////////////////////////////////////////////////////
// NewSessionRequest.h
//
// This class wraps the RequestNewSession message (JSON message at the network level).  
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class NewSessionRequest 
{
public:
    NewSessionRequest();
    explicit NewSessionRequest(const JSONMessagePtr& message);

	static const std::string& GetMessageType();

    virtual std::string ToJSONString() const;

	virtual std::string GetSessionName() const;
	virtual void		SetSessionName(const std::string& name);

	virtual SessionType GetSessionType() const;
	virtual void		SetSessionType(SessionType type);
    
private:
	JSONMessagePtr	m_message;

	static const std::string gMessageType;
	static const std::string gSessionNameKey;
	static const std::string gSessionTypeKey;
};

DECLARE_PTR(NewSessionRequest)

XTOOLS_NAMESPACE_END
