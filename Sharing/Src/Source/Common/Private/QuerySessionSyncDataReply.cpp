//////////////////////////////////////////////////////////////////////////
// QuerySessionSyncDataReply.cpp
//
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "QuerySessionSyncDataReply.h"

XTOOLS_NAMESPACE_BEGIN

namespace
{
	static const std::string kMessageType("QuerySessionSyncDataReply");
	static const std::string kSyncDataSessions("syncDataSessions");
	static const utility::char_t* kUris = U("uris");
	static const utility::char_t* kSessionNames = U("sessionNames");
	static const utility::char_t* kValues = U("values");
	static const XValue kInvalidXValue;
}

bool QuerySessionSyncDataReply::SyncData::Verify() const
{
	return XTVERIFY((Uris.size() * SessionNames.size()) == Values.size());
}

void QuerySessionSyncDataReply::SyncData::Clear()
{
	Uris.resize(0);
	SessionNames.resize(0);
	Values.resize(0);
}

const XValue& QuerySessionSyncDataReply::SyncData::GetValue(size_t uriIndex, size_t sessionIndex) const
{
	if (!XTVERIFY(uriIndex < Uris.size()))
	{
		return kInvalidXValue;
	}

	if (!XTVERIFY(sessionIndex < SessionNames.size()))
	{
		return kInvalidXValue;
	}

	size_t index = (sessionIndex * Uris.size()) + uriIndex;

	if (!XTVERIFY(index < Values.size()))
	{
		return kInvalidXValue;
	}

	return Values[index];
}

QuerySessionSyncDataReply::QuerySessionSyncDataReply(const SyncData& syncData)
	: m_message(JSONMessage::Create(kMessageType))
{
	web::json::value syncDataObject = web::json::value::object();

	if (syncData.Verify())
	{
		// Add uri array to sync data object.
		{
			web::json::value uris = web::json::value::array(syncData.Uris.size());

			for (size_t i = 0; i < syncData.Uris.size(); ++i)
			{
				uris[i] = web::json::value::string(utility::conversions::to_string_t(syncData.Uris[i]));
			}

			syncDataObject[kUris] = uris;
		}

		// Add session name array to sync data object.
		{
			web::json::value sessionNames = web::json::value::array(syncData.SessionNames.size());

			for (size_t i = 0; i < syncData.SessionNames.size(); ++i)
			{
				sessionNames[i] = web::json::value::string(utility::conversions::to_string_t(syncData.SessionNames[i]));
			}

			syncDataObject[kSessionNames] = sessionNames;
		}

		// Add XValue array to sync data object.
		{
			web::json::value values = web::json::value::array(syncData.Values.size());

			for (size_t i = 0; i < syncData.Values.size(); ++i)
			{
				values[i] = syncData.Values[i].AsJSON();
			}

			syncDataObject[kValues] = values;
		}
	}

	m_message->SetValue(kSyncDataSessions, syncDataObject);
}


QuerySessionSyncDataReply::QuerySessionSyncDataReply(const JSONMessagePtr& message)
	: m_message(message)
{

}

void QuerySessionSyncDataReply::GetSyncData(SyncData& out) const
{
	out.Clear();

	web::json::value syncDataValue = m_message->GetObjectValue(kSyncDataSessions);
	web::json::object& syncDataObject = syncDataValue.as_object();

	// Add each uri to the output.
	web::json::object::iterator uris = syncDataObject.find(kUris);
	if (XTVERIFY(uris != syncDataObject.end()))
	{
		web::json::array& uriArray = uris->second.as_array();
		out.Uris.reserve(uriArray.size());

		for (auto uri : uriArray)
		{
			out.Uris.push_back(utility::conversions::to_utf8string(uri.as_string()));
		}
	}

	// Add each session name to the output.
	web::json::object::iterator sessionNames = syncDataObject.find(kSessionNames);
	if (XTVERIFY(sessionNames != syncDataObject.end()))
	{
		web::json::array& sessionNameArray = sessionNames->second.as_array();
		out.SessionNames.reserve(sessionNameArray.size());

		for (auto sessionName : sessionNameArray)
		{
			out.SessionNames.push_back(utility::conversions::to_utf8string(sessionName.as_string()));
		}
	}

	// Add each uri to the output.
	web::json::object::iterator values = syncDataObject.find(kValues);
	if (XTVERIFY(values != syncDataObject.end()))
	{
		web::json::array& valueArray = values->second.as_array();
		out.Values.reserve(valueArray.size());

		for (auto value : valueArray)
		{
			out.Values.push_back(XValue::FromJSON(value.as_object()));
		}
	}

	if (!XTVERIFY(out.Verify()))
	{
		out.Clear();
	}
}

//static 
const std::string& QuerySessionSyncDataReply::GetMessageType()
{
	return kMessageType;
}


std::string QuerySessionSyncDataReply::ToJSONString() const
{
	return m_message->ToString();
}

XTOOLS_NAMESPACE_END
