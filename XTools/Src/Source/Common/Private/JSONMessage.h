//////////////////////////////////////////////////////////////////////////
// JSONMessage.h
//
// Wrapper class for constructing or parsing JSON messages in XTools.
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class JSONMessage : public RefCounted
{

public:
	// Create a message of the specific type
	static ref_ptr<JSONMessage> Create(const std::string& messageType);

	// Wrap an incoming message to make it easier to parse
	static ref_ptr<JSONMessage> CreateFromMessage(const std::string& incomingMessage);

	// Serialize the data to a string
	std::string ToString() const;


	// Get the value at the given key
	std::string GetStringValue(const std::string& key) const;
	int32		GetIntValue(const std::string& key) const;
	uint32		GetUIntValue(const std::string& key) const;
	int64		GetLongValue(const std::string& key) const;
	float		GetFloatValue(const std::string& key) const;
	bool		GetBoolValue(const std::string& key) const;
	web::json::value	GetObjectValue(const std::string& key) const;

	// Set the value at the given key
	void SetValue(const std::string& key, const std::string& value);
	void SetValue(const std::string& key, int32 value);
	void SetValue(const std::string& key, uint32 value);
	void SetValue(const std::string& key, int64 value);
	void SetValue(const std::string& key, float value);
	void SetValue(const std::string& key, bool value);
	void SetValue(const std::string& key, web::json::value obj);

protected:
	web::json::value m_content;

	JSONMessage(web::json::value& content);
};

DECLARE_PTR(JSONMessage);

XTOOLS_NAMESPACE_END
