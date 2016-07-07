// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// NewSessionRequest.h
// This class wraps the RequestNewSession message (JSON message at the network level).  
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
    
private:
	JSONMessagePtr	m_message;

	static const std::string gMessageType;
	static const std::string gSessionNameKey;
};

DECLARE_PTR(NewSessionRequest)

XTOOLS_NAMESPACE_END
