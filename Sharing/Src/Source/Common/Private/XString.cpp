//////////////////////////////////////////////////////////////////////////
// XString.cpp
//
// String class that is exposed across the interface boundary.  Using this
// class allows us to reduce the number of times full strings need to be passed
// back and forth across the boundary, which can be expensive.  
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Public/XString.h"

XTOOLS_NAMESPACE_BEGIN


XString::XString()
{

}


XString::XString(const std::string& str)
	: m_string(str)
{

}


XString::~XString()
{
	// For debugging: set the string to a known value to detect if was are accessing deleted objects
	m_string = "DeletedXString";
}


uint32 XString::GetLength() const
{
	return static_cast<uint32>(m_string.length());
}


bool XString::IsEqual(const XStringPtr& otherStr) const
{
	return ((otherStr != NULL) && (m_string == otherStr->m_string));
}


const std::string& XString::GetString() const
{
	return m_string;
}

XTOOLS_NAMESPACE_END