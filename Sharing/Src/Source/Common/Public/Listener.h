//////////////////////////////////////////////////////////////////////////
// Listener.h
//
// Base class for all type that wish to register to receive callbacks
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class Listener
{
public:
	virtual ~Listener() { UnregisterAll(); }

	void UnregisterAll()
	{ 
		for (int32 i = (int32)m_registrations.size() - 1; i >= 0; --i)
		{
			ReceiptPtr receipt = m_registrations[i].m_receipt;
			m_registrations[i].m_receipt = nullptr;
		}
	}

	bool IsRegistered() const { return !m_registrations.empty(); }

#if !defined(SWIG)
	inline void AddRegistration(const ReceiptPtr& receipt, uint32 key)
	{
		m_registrations.push_back(RegInfo(receipt, key));
	}

	inline void DeleteRegistration(uint32 key)
	{
		for (auto it = m_registrations.begin(); it != m_registrations.end(); ++it)
		{
			if (it->m_key == key)
			{
				if (it->m_receipt) it->m_receipt->Clear(); // Make the receipt not call the deregistration function on destruction
				m_registrations.erase(it);
				break;
			}
		}
	}
#endif

private:
	struct RegInfo 
	{
		RegInfo(const ReceiptPtr& receipt, uint32 key) : m_receipt(receipt), m_key(key) {}
		ReceiptPtr m_receipt;
		uint32 m_key;
	};

	std::vector<RegInfo>	m_registrations;
};

XTOOLS_NAMESPACE_END
