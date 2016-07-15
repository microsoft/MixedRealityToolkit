// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// DownloadManagerImpl.cpp
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DownloadManagerImpl.h"
#include "DownloadBufferImpl.h"

XTOOLS_NAMESPACE_BEGIN

static const uint32 kDefaultDownloadBufferSize = 2 * 1024;


//////////////////////////////////////////////////////////////////////////
// Used on the download thread for holding all the data, including libCurl handles, for an active request.  
// Creates and deletes libCurl handles for specific requests. 
struct DownloadManagerImpl::CurlRequest : public AtomicRefCounted
{
	explicit CurlRequest(const RequestPtr& requestSettings)
	: m_request(requestSettings)
	, m_curlHandle(nullptr)
	, m_curlMultiHandle(nullptr)
{
	m_curlHandle = curl_easy_init();
	XTASSERT(m_curlHandle != nullptr);

	// Set the URL to download
	XTVERIFY(curl_easy_setopt(m_curlHandle, CURLOPT_URL, m_request->m_url.c_str()) == CURLE_OK);

	// Set cookies if there are any
	if (!m_request->m_cookies.empty()) {
		XTVERIFY(curl_easy_setopt(m_curlHandle, CURLOPT_COOKIE, m_request->m_cookies.c_str()) == CURLE_OK);
	}
	// Set the callback function to accumulate
	XTVERIFY(curl_easy_setopt(m_curlHandle, CURLOPT_WRITEFUNCTION, &DownloadManagerImpl::OnPartialDownloadReceived) == CURLE_OK);
	XTVERIFY(curl_easy_setopt(m_curlHandle, CURLOPT_WRITEDATA, this) == CURLE_OK);
}

virtual ~CurlRequest()
{
	Deactivate();
	curl_easy_cleanup(m_curlHandle);
}

void Activate(CURLM* multiHandle)
{
	XTASSERT(m_curlMultiHandle == nullptr);

	m_curlMultiHandle = multiHandle;

	// add the individual transfer to the libCurl multi object
	XTVERIFY(curl_multi_add_handle(m_curlMultiHandle, m_curlHandle) == CURLM_OK);
}

void Deactivate()
{
	if (m_curlMultiHandle != nullptr)
	{
		curl_multi_remove_handle(m_curlMultiHandle, m_curlHandle);
		m_curlMultiHandle = nullptr;
	}
}

RequestPtr				m_request;
CURL*					m_curlHandle;
CURLM*					m_curlMultiHandle;
};


//////////////////////////////////////////////////////////////////////////
DownloadManagerImpl::DownloadManagerImpl()
	: m_curlMultiHandle(nullptr)
	, m_stopping(0)
	, m_commandQueue(sizeof(Command) * 100)
	, m_resultQueue(sizeof(Result) * 100)
	, m_requestEvent(false, false)
	, m_requestIDCounter(0)
{
	// Start a download thread to run the update loop. 
	m_downloadThread = new MemberFuncThread(&DownloadManagerImpl::DownloadThreadFunction, this);
}


DownloadManagerImpl::~DownloadManagerImpl()
{
	// trigger the thread exit and wait for it...
	m_stopping = 1;
	m_requestEvent.Set();
	m_downloadThread->WaitForThreadExit();
}


DownloadRequestID DownloadManagerImpl::CreateRequestID()
{
	ScopedLock lock(m_counterMutex);
	return ++m_requestIDCounter;
}


void DownloadManagerImpl::CreateRequest(const std::string& url, DownloadRequestID requestID, DownloadCallback* callback, bool asyncCallback)
{
	CreateRequest(url, "", requestID, callback, asyncCallback);
}

void DownloadManagerImpl::CreateRequest(const std::string& url, const std::string& cookies, DownloadRequestID requestID, DownloadCallback* callback, bool asyncCallback)
{
	if (XTVERIFY(url.length() > 0) &&
		XTVERIFY(requestID != kInvalidDownloadRequestID) &&
		XTVERIFY(callback != nullptr)
		)
	{
		RequestPtr newRequest = new Request();
		newRequest->m_url = url;
		newRequest->m_cookies = cookies;
		newRequest->m_buffer = new DownloadBufferImpl(kDefaultDownloadBufferSize);
		newRequest->m_callback = callback;
		newRequest->m_requestID = requestID;
		newRequest->m_bAsyncCallback = asyncCallback;

		// Push the request onto the queue for the download thread to process
		Command downloadCommand;
		downloadCommand.m_bBeginDownload = true;
		downloadCommand.m_requestID = requestID;
		downloadCommand.m_request = newRequest;

		m_commandBuffer.push(downloadCommand);
	}
}


void DownloadManagerImpl::CancelRequest(DownloadRequestID requestID)
{
	// Push the cancellation request onto the queue for the download thread to process
	Command cancelCommand;
	cancelCommand.m_bBeginDownload = false;
	cancelCommand.m_requestID = requestID;
	cancelCommand.m_request = nullptr;

	m_commandBuffer.push(cancelCommand);
}


void DownloadManagerImpl::Update()
{
	int numResults = 0;
	Result currentResult;

	// Try to push any commands to the download thread
	while (!m_commandBuffer.empty())
	{
		bool pushResult = m_commandQueue.TryPush(m_commandBuffer.front());

		// Signal the download thread to wake up and process the command,
		// whether the queue is full or not
		m_requestEvent.Set();

		if (pushResult)
		{
			m_commandBuffer.pop();
		}
		else
		{
			break;
		}
	}

	// Check to see if any downloads are ready to have their results send to the user callback functions
	while (m_resultQueue.TryPop(currentResult))
	{
		if (XTVERIFY(currentResult.m_request))
		{
			RequestPtr currentRequest = currentResult.m_request;

			// We should only ever receive results that are not to be called asynchronously here
			XTASSERT(!currentRequest->m_bAsyncCallback);

			if (currentResult.m_error == DownloadError::None)
			{
				// No error, pass the downloaded buffer to the callback
				currentRequest->m_callback->OnDownloadCompleted(currentRequest->m_requestID, currentRequest->m_buffer);
			}
			else
			{
				// There was an error
				currentRequest->m_callback->OnDownloadError(currentRequest->m_requestID, currentResult.m_error);
			}
		}

		++numResults;
	}
}


void DownloadManagerImpl::DownloadThreadFunction()
{
	// Initialize libCurl
	XTVERIFY(curl_global_init(CURL_GLOBAL_ALL) == CURLE_OK);

	// Initialize the libCurl handle for the multi interface
	m_curlMultiHandle = curl_multi_init();

	while (m_stopping == 0)
	{
		// Process all incoming commands, creating new requests or canceling existing ones
		ProcessCommands();

		// Update the pending downloads
		int still_running;
		XTVERIFY(curl_multi_perform(m_curlMultiHandle, &still_running) == CURLM_OK);

		// Check to see if any requests have completed or failed, and notify the callbacks of either event
		CheckRequests();

		// Try to push any results to the main thread
		while (!m_resultBuffer.empty() && m_resultQueue.TryPush(m_resultBuffer.front()))
		{
			m_resultBuffer.pop();
		}

		// Make the thread block and wait if there aren't any downloads in flight 
		if (m_handleToRequestMap.empty() && m_resultBuffer.empty())
		{
			m_requestEvent.Wait();
		}
	}

	// We're exiting the thread.  Clean up all the requests that have not finished before returning.
	// Do NOT notify the callbacks of the failure, as the callback object may have already been deleted at this point.
	m_handleToRequestMap.clear();
	m_idToRequestMap.clear();

	XTVERIFY(curl_multi_cleanup(m_curlMultiHandle) == CURLE_OK);
	m_curlMultiHandle = nullptr;

	curl_global_cleanup();
}


void DownloadManagerImpl::ProcessCommands()
{
	Command newCommand;
	while (m_commandQueue.TryPop(newCommand))
	{
		// Begin a new download
		if (newCommand.m_bBeginDownload)
		{
			RequestPtr request = newCommand.m_request;

			// Check that the request ID is not already in use
			if (m_idToRequestMap.find(request->m_requestID) != m_idToRequestMap.end())
			{
				// Report the error
				SendError(request, DownloadError::RequestIDAlreadyInUse);
			}
			else
			{
				CurlRequestPtr newCurlRequest = new CurlRequest(request);

				// Add the request to both maps, so it can be looked up by ID or by handle
				m_handleToRequestMap[newCurlRequest->m_curlHandle] = newCurlRequest;
				m_idToRequestMap[request->m_requestID] = newCurlRequest;

				newCurlRequest->Activate(m_curlMultiHandle);
			}
		}
		// Cancel a download
		else
		{
			// Find the active request
			auto requestIter = m_idToRequestMap.find(newCommand.m_requestID);
			if (requestIter != m_idToRequestMap.end())
			{
				CurlRequestPtr request = requestIter->second;
				m_idToRequestMap.erase(requestIter);
				m_handleToRequestMap.erase(m_handleToRequestMap.find(request->m_curlHandle));
			}
		}
	}
}


void DownloadManagerImpl::CheckRequests()
{
	CURLMsg *message;
	int pending;

	// Process all pending messages
	while ((message = curl_multi_info_read(m_curlMultiHandle, &pending)) != nullptr)
	{
		switch (message->msg)
		{
		case CURLMSG_DONE: // A download has finished
		{
			// Find the completed request
			auto requestIter = m_handleToRequestMap.find(message->easy_handle);
			if (requestIter != m_handleToRequestMap.end())
			{
				RequestPtr request = requestIter->second->m_request;

				// Did the download complete successfully?
				if (message->data.result == CURLcode::CURLE_OK)
				{
					if (request->m_bAsyncCallback)
					{
						// pass the downloaded buffer to the callback
						request->m_callback->OnDownloadCompleted(request->m_requestID, request->m_buffer);
					}
					else
					{
						Result newResult;
						newResult.m_error = DownloadError::None;
						newResult.m_request = request;
						m_resultBuffer.push(newResult);
					}
				}
				// Download was unsuccessful
				else
				{
					// TODO: identify more informative error types
					DownloadError errorID = DownloadError::Unknown;

					if (request->m_bAsyncCallback)
					{
						// pass the downloaded buffer to the callback
						request->m_callback->OnDownloadError(request->m_requestID, errorID);
					}
					else
					{
						Result newResult;
						newResult.m_error = errorID;
						newResult.m_request = request;
						m_resultBuffer.push(newResult);
					}
				}

				// The request is done; remove it from both maps
				m_handleToRequestMap.erase(requestIter);
				m_idToRequestMap.erase(m_idToRequestMap.find(request->m_requestID));
			}

			break;
		}

		default:
			//fprintf(stderr, "CURLMSG default\n");
			break;
		}
	}
}


void DownloadManagerImpl::SendError(const RequestPtr& request, DownloadError errorID)
{
	XTASSERT(request);

	if (request->m_bAsyncCallback)
	{
		request->m_callback->OnDownloadError(request->m_requestID, errorID);
	}
	else
	{
		Result newResult;
		newResult.m_request = request;
		newResult.m_error = errorID;
		m_resultBuffer.push(newResult);
	}
}


// static
size_t DownloadManagerImpl::OnPartialDownloadReceived(void *buffer, size_t size, size_t nmemb, void *userp)
{
	size_t totalSize = size * nmemb;

	CurlRequest* requestInfo = reinterpret_cast<CurlRequest*>(userp);

	// Accumulate the partial downloaded data into the download buffer
	requestInfo->m_request->m_buffer->Append(buffer, (uint32)totalSize);

	return totalSize;
}


XTOOLS_NAMESPACE_END
