//////////////////////////////////////////////////////////////////////////
// CleanupNotifier.h
//
// Utility class that allows other listeners to register to be notified 
// when decendents of this class are destroyed.  
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
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