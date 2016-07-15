// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// DownloadCallback.h
// Interface class for objects that want to receive notifications when
// downloading
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <Public/DownloadCommon.h>
#include <Public/DownloadBuffer.h>

XTOOLS_NAMESPACE_BEGIN

class DownloadCallback 
{
public:
	virtual ~DownloadCallback() {}
	virtual void OnDownloadCompleted(DownloadRequestID requestID, const DownloadBufferPtr& buffer) = 0;
	virtual void OnDownloadError(DownloadRequestID requestID, DownloadError errorID) = 0;
};

XTOOLS_NAMESPACE_END
