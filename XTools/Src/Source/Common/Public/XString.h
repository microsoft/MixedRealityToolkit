//////////////////////////////////////////////////////////////////////////
// XString.h
//
// String class that is exposed across the interface boundary.  Using this
// class allows us to reduce the number of times full strings need to be passed
// back and forth across the boundary, which can be expensive.  
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef SWIG
#include <string>
#endif

XTOOLS_NAMESPACE_BEGIN

DECLARE_PTR_PRE(XString)

class XString : public AtomicRefCounted
{
public:
	XString();
	XString(const std::string& str);
	virtual ~XString();

	// Return the number of characters in the string
	uint32 GetLength() const;

	// Returns true if the given string matches this one
	bool IsEqual(const ref_ptr<XString>& otherStr) const;

	// Returns the string.  Swig will convert this automatically to the default string type in Java or C#
	const std::string& GetString() const;

private:
	std::string m_string;
};

DECLARE_PTR_POST(XString)

XTOOLS_NAMESPACE_END
