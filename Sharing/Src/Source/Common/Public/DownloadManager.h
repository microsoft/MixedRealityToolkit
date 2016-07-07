// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// DownloadManager.h
// API for downloading from http exposed through the SWIG wrappers
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "Public/Utils/Receipt.h"
#include <Public/DownloadCommon.h>

XTOOLS_NAMESPACE_BEGIN

class DownloadCallback;

class DownloadManager : public AtomicRefCounted
{
public:

	// Creates a unique ID to identify a request.  Does not initiate a request, just reserves an ID to use when making a new request.
	// Call this first before calling CreateRequest() to pre-assign and ID for your request.  
	virtual DownloadRequestID CreateRequestID() = 0;

	// Initiates a request for a given remote file.  
	//   url - The full URL of the file to download
	//   requestID - A unique ID returned from CreateRequestID().  This needs to be created in advance, rather than returned from this function,
	//				because this function will kicks off the download on another thread, and it is possible for the callback to be called
	//				BEFORE this function returns.  So be sure that your callback handler is ready to process callbacks before calling this function
	//   callback - Pointer to the object that should receive notifications about this requests
	//   asyncCallback - should the callbacks for this request be called synchronously from the Update thread or asynchronously from the download background thread.  
	virtual void CreateRequest(const std::string& url, DownloadRequestID requestID, DownloadCallback* callback, bool asyncCallback) = 0;

	// Initiates a request for a given remote file.  
	//   url - The full URL of the file to download
	//   cookies - cookies string to include when making the request. For example, you can passed in signed cookies as form of download authentication
	//   requestID - A unique ID returned from CreateRequestID().  This needs to be created in advance, rather than returned from this function,
	//				because this function will kicks off the download on another thread, and it is possible for the callback to be called
	//				BEFORE this function returns.  So be sure that your callback handler is ready to process callbacks before calling this function
	//   callback - Pointer to the object that should receive notifications about this requests
	//   asyncCallback - should the callbacks for this request be called synchronously from the Update thread or asynchronously from the download background thread.  
	virtual void CreateRequest(const std::string& url, const std::string& cookies, DownloadRequestID requestID, DownloadCallback* callback, bool asyncCallback) = 0;

	// Cancel a submitted request.  If the request is not active, then this function does nothing
	virtual void CancelRequest(DownloadRequestID requestID) = 0;

	virtual void Update() = 0;
};

DECLARE_PTR(DownloadManager)

XTOOLS_NAMESPACE_END


