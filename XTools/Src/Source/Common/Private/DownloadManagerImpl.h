//////////////////////////////////////////////////////////////////////////
// DownloadManagerImpl.h
//
// Implementation for downloading from http exposed through the SWIG wrappers
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <Public/DownloadManager.h>
#include <Public/DownloadCallback.h>
#include <Public/DownloadBuffer.h>
#include <Private/Utils/TypedLFQueue.h>
#include <string>
#include <map>

// HACK: Don't want to expose the full libcurl header to files outside Common that reference this file,
// so we're declaring the typedefs here.  
typedef void CURL;
typedef void CURLM;

XTOOLS_NAMESPACE_BEGIN

class DownloadManagerImpl : public DownloadManager
{
public:
	DownloadManagerImpl();
	virtual ~DownloadManagerImpl();

	virtual DownloadRequestID	CreateRequestID() XTOVERRIDE;
	virtual void				CreateRequest(const std::string& url, DownloadRequestID requestID, DownloadCallback* callback, bool asyncCallback) XTOVERRIDE;
	virtual void				CreateRequest(const std::string& url, const std::string& cookies, DownloadRequestID requestID, DownloadCallback* callback, bool asyncCallback) XTOVERRIDE;
	virtual void				CancelRequest(DownloadRequestID requestID) XTOVERRIDE;
	virtual void				Update() XTOVERRIDE;

private:

	// Core information about a download request.  Used by both the main and download threads
	struct Request;
	DECLARE_PTR(Request)

	// Container for sending commands to the download thread
	struct Command
	{
		RequestPtr			m_request;			// If this is a cancellation command, this will be null
		DownloadRequestID	m_requestID;
		bool				m_bBeginDownload;	// If false, cancel the download.  
	};

	// Container for sending results to the main thread
	struct Result
	{
		RequestPtr		m_request;
		DownloadError	m_error;
	};

	// Used on the download thread for holding all the data, including libCurl handles, for an active request
	struct CurlRequest;
	DECLARE_PTR(CurlRequest)

	void DownloadThreadFunction();
	void ProcessCommands();
	void CheckRequests();
	void SendError(const RequestPtr& request, DownloadError errorID);

	// Called by libCurl as parts of the downloaded file arrive
	static size_t OnPartialDownloadReceived(void *buffer, size_t size, size_t nmemb, void *userp);

	std::map<CURL*, CurlRequestPtr>				m_handleToRequestMap;
	std::map<DownloadRequestID, CurlRequestPtr>	m_idToRequestMap;
	MemberFuncThreadPtr							m_downloadThread;
	CURLM*										m_curlMultiHandle;
	volatile int								m_stopping;

	TypedLFQueue<Command>						m_commandQueue;
	std::queue<Command>							m_commandBuffer;	// Store commands to push later if we cannot push any more on the lfqueue

	TypedLFQueue<Result>						m_resultQueue;
	std::queue<Result>							m_resultBuffer;		// Store results to push later if we cannot push any more on the lfqueue

	Event										m_requestEvent;
	Mutex										m_counterMutex;
	uint32										m_requestIDCounter;
};

DECLARE_PTR(DownloadManagerImpl)

XTOOLS_NAMESPACE_END
