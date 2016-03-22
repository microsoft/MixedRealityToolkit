//////////////////////////////////////////////////////////////////////////
// DownloadCommon.h
//
// Collection of types that are used in multiple places in the download code
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once


XTOOLS_NAMESPACE_BEGIN

// Enumeration of all the types of error that can be returned from a failed download request
enum DownloadError
{
	None = 0,
	RequestIDAlreadyInUse,
	Unknown
};


typedef uint32 DownloadRequestID;
static const DownloadRequestID kInvalidDownloadRequestID = 0xFFFFFFFF;

XTOOLS_NAMESPACE_END