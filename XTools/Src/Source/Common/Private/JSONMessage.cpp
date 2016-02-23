//////////////////////////////////////////////////////////////////////////
// JSONMessage.cpp
//
// Wrapper class for constructing or parsing JSON messages in XTools.
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Public/CommonSettings.h>
#include "JSONMessage.h" // is this how other cpps include their .h?

XTOOLS_NAMESPACE_BEGIN

//static 
JSONMessagePtr JSONMessage::Create(const std::string& messageType)
{
	web::json::value content = web::json::value::object();
	JSONMessagePtr msg = new JSONMessage(content);
	msg->SetValue(kSessionMessageTypeKey, messageType);

	return msg;
}


//static 
JSONMessagePtr JSONMessage::CreateFromMessage(const std::string& incomingMessage)
{
	const utility::string_t wst8 = utility::conversions::to_string_t(incomingMessage);

	web::json::value content = web::json::value::parse(wst8);

	return new JSONMessage(content);
}


JSONMessage::JSONMessage(web::json::value& content)
	: m_content(content)
{
	
}


std::string JSONMessage::ToString() const
{
	return utility::conversions::to_utf8string(m_content.serialize());
}


std::string JSONMessage::GetStringValue(const std::string& key) const
{
	utility::string_t typeKey = utility::conversions::to_string_t(key);

	utility::string_t parsedType = m_content.at(typeKey).as_string();

	return utility::conversions::to_utf8string(parsedType);
}


int32 JSONMessage::GetIntValue(const std::string& key) const
{
	utility::string_t typeKey = utility::conversions::to_string_t(key);

	return m_content.at(typeKey).as_number().to_int32();
}


uint32 JSONMessage::GetUIntValue(const std::string& key) const
{
	utility::string_t typeKey = utility::conversions::to_string_t(key);

	return m_content.at(typeKey).as_number().to_uint32();
}


int64 JSONMessage::GetLongValue(const std::string& key) const
{
	utility::string_t typeKey = utility::conversions::to_string_t(key);

	return m_content.at(typeKey).as_number().to_int64();
}


float JSONMessage::GetFloatValue(const std::string& key) const
{
	utility::string_t typeKey = utility::conversions::to_string_t(key);

	return static_cast<float>(m_content.at(typeKey).as_double());
}


bool JSONMessage::GetBoolValue(const std::string& key) const
{
	utility::string_t typeKey = utility::conversions::to_string_t(key);

	return m_content.at(typeKey).as_bool();
}


web::json::value JSONMessage::GetObjectValue(const std::string& key) const
{
	utility::string_t typeKey = utility::conversions::to_string_t(key);

	return m_content.at(typeKey);
}


void JSONMessage::SetValue(const std::string& key, const std::string& value)
{
	utility::string_t keyWString = utility::conversions::to_string_t(key);
	utility::string_t valueWString = utility::conversions::to_string_t(value);

	m_content[keyWString] = web::json::value(valueWString);
}


void JSONMessage::SetValue(const std::string& key, int32 value)
{
	utility::string_t keyWString = utility::conversions::to_string_t(key);

	m_content[keyWString] = web::json::value(value);
}


void JSONMessage::SetValue(const std::string& key, uint32 value)
{
	utility::string_t keyWString = utility::conversions::to_string_t(key);

	m_content[keyWString] = web::json::value(value);
}


void JSONMessage::SetValue(const std::string& key, int64 value)
{
	utility::string_t keyWString = utility::conversions::to_string_t(key);

	m_content[keyWString] = web::json::value(value);
}


void JSONMessage::SetValue(const std::string& key, float value)
{
	utility::string_t keyWString = utility::conversions::to_string_t(key);

	m_content[keyWString] = web::json::value(static_cast<double>(value));
}


void JSONMessage::SetValue(const std::string& key, bool value)
{
	utility::string_t keyWString = utility::conversions::to_string_t(key);

	m_content[keyWString] = web::json::value(value);
}


void JSONMessage::SetValue(const std::string& key, web::json::value obj)
{
	utility::string_t keyWString = utility::conversions::to_string_t(key);

	m_content[keyWString] = obj;
}
XTOOLS_NAMESPACE_END
