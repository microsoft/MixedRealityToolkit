//////////////////////////////////////////////////////////////////////////
// XMLElementSerializer.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

class SyncElementSerializer : public RefCounted
{
public:

	virtual bool Save(FILE* file, const ObjectElementConstPtr& root) = 0;
	virtual bool Save(std::ostream& stream, const ObjectElementConstPtr& root) = 0;
	virtual bool Save(std::string& xmlStr, const ObjectElementConstPtr& root) = 0;

	virtual bool Load(FILE* file, const ObjectElementPtr& root) = 0;
	virtual bool Load(std::istream& stream, const ObjectElementPtr& root) = 0;
	virtual bool Load(const std::string& xmlStr, const ObjectElementPtr& root) = 0;
};

DECLARE_PTR(SyncElementSerializer)

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END