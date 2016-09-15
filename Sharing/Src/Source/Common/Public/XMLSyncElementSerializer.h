//////////////////////////////////////////////////////////////////////////
// XMLElementSerializer.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "SyncElementSerializer.h"

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

class XMLSyncElementSerializer : public SyncElementSerializer
{
public:

	XMLSyncElementSerializer(bool writeHeader);

	virtual bool Save(FILE* file, const ObjectElementConstPtr& root) XTOVERRIDE;
	virtual bool Save(std::ostream& stream, const ObjectElementConstPtr& root) XTOVERRIDE;;
	virtual bool Save(std::string& xmlStr, const ObjectElementConstPtr& root) XTOVERRIDE;;

	virtual bool Load(FILE* file, const ObjectElementPtr& root) XTOVERRIDE;;
	virtual bool Load(std::istream& stream, const ObjectElementPtr& root) XTOVERRIDE;;
	virtual bool Load(const std::string& xmlStr, const ObjectElementPtr& root) XTOVERRIDE;;

private:

	bool m_writeHeader;
};

DECLARE_PTR(XMLSyncElementSerializer);

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END