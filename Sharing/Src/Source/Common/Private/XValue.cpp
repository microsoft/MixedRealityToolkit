//////////////////////////////////////////////////////////////////////////
// XValue.cpp
//
// Convenient class for holding an arbitrary value
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Public/XValue.h>

XTOOLS_NAMESPACE_BEGIN

namespace
{
	static const utility::char_t* kType = U("type");
	static const utility::char_t* kValue = U("value");
}

void XValue::Deserialize(NetworkInMessage& msg)
{
	m_wrappedValue = NULL;

	Type inType = (Type)msg.ReadByte();

	if (inType != XTools::XValue::Unknown)
	{
		switch (inType)
		{
		case XTools::XValue::Bool:
			*this = XValue(msg.ReadByte() != 0);
			break;
		case XTools::XValue::Int:
			*this = XValue(msg.ReadInt32());
			break;
		case XTools::XValue::UInt:
			*this = XValue(msg.ReadUInt32());
			break;
		case XTools::XValue::Int64:
			*this = XValue(msg.ReadInt64());
			break;
		case XTools::XValue::Float:
			*this = XValue(msg.ReadFloat());
			break;
		case XTools::XValue::Double:
			*this = XValue(msg.ReadDouble());
			break;
		case XTools::XValue::String:
			*this = XValue(msg.ReadStdString());
			break;
		default:
			XTASSERT(false);
			break;
		}
	}

	XTASSERT(GetType() == inType);
}

// JSON guts to parse or create the data structure
web::json::value XValue::AsJSON() const
{
	web::json::value result = web::json::value::object();
	Type type = Unknown;

	if (m_wrappedValue != nullptr)
	{
		type = m_wrappedValue->GetType();
	}

	result[kType] = (int)type;

	switch (type)
	{
	case XTools::XValue::Bool: result[kValue] = web::json::value::boolean(*Get<bool>()); break;
	case XTools::XValue::Int: result[kValue] = web::json::value::number(*Get<int32>()); break;
	case XTools::XValue::UInt: result[kValue] = web::json::value::number((double)*Get<uint32>()); break;
	case XTools::XValue::Int64: result[kValue] = web::json::value::number((double)*Get<int64>()); break;
	case XTools::XValue::Float: result[kValue] = web::json::value::number(*Get<float>()); break;
	case XTools::XValue::Double: result[kValue] = web::json::value::number(*Get<double>()); break;
	case XTools::XValue::String: result[kValue] = web::json::value::string(utility::conversions::to_string_t(*Get<std::string>())); break;
	default: result[kValue] = web::json::value(); break;
	}

	return result;
}

XValue XValue::FromJSON(const web::json::object& object)
{
	Type type = (Type)object.at(kType).as_integer();
	switch (type)
	{
	case XTools::XValue::Bool: return XValue(object.at(kValue).as_bool());
	case XTools::XValue::Int:  return XValue(object.at(kValue).as_integer());
	case XTools::XValue::UInt: return XValue((uint32)object.at(kValue).as_double());
	case XTools::XValue::Int64: return XValue((int64)object.at(kValue).as_double());
	case XTools::XValue::Float: return XValue((float)object.at(kValue).as_double());
	case XTools::XValue::Double: return XValue(object.at(kValue).as_double());
	case XTools::XValue::String: return XValue(utility::conversions::to_utf8string(object.at(kValue).as_string()));
	default: return XValue();
	}
}

std::string XValue::ToString() const
{
	char buffer[512];

	switch (m_wrappedValue->GetType())
	{
	case XTools::XValue::Bool:
		sprintf_s(buffer, sizeof(buffer), "%s", (*Get<bool>()) ? "true" : "false");
		break;
	case XTools::XValue::Int:
		sprintf_s(buffer, sizeof(buffer), "%i", *Get<int32>());
		break;
	case XTools::XValue::UInt:
		sprintf_s(buffer, sizeof(buffer), "%u", *Get<uint32>());
		break;
	case XTools::XValue::Int64:
		sprintf_s(buffer, sizeof(buffer), "%lld", *Get<int64>());
		break;
	case XTools::XValue::Float:
		sprintf_s(buffer, sizeof(buffer), "%f", *Get<float>());
		break;
	case XTools::XValue::Double:
		sprintf_s(buffer, sizeof(buffer), "%f", *Get<double>());
		break;
	case XTools::XValue::String:
		sprintf_s(buffer, sizeof(buffer), "%s", Get<std::string>()->c_str());
		break;
	case XTools::XValue::Unknown:
	default:
		sprintf_s(buffer, sizeof(buffer), "%s", "Unknown");
		break;
	}

	return buffer;
}

XTOOLS_NAMESPACE_END
