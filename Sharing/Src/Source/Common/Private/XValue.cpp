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
