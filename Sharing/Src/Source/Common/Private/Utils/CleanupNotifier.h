// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// CleanupNotifier.h
// Utility class that allows other listeners to register to be notified 
// when decendents of this class are destroyed.  
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class CleanupNotifier
{
public:
	CleanupNotifier() {}
	virtual ~CleanupNotifier() {}

	void RegisterCleanupListener(const ReceiptPtr& receipt) { m_cleanupReceipts.push_back(receipt); }

private:
	ReceiptVector	m_cleanupReceipts;
};

XTOOLS_NAMESPACE_END
