//////////////////////////////////////////////////////////////////////////
// ListenerList.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

template<typename T>
class ListenerList : public AtomicRefCounted
{
	typedef ListenerList<T> this_type;

public:
	static ref_ptr<this_type> Create() { return new ListenerList(); }

	void AddListener(T* newListener)
	{
		// Check for a null parameter
		if (newListener == nullptr)
		{
			//LogError("Cannot add a NULL listener");
			return;
		}

		// Check to make sure we aren't adding a duplicate listener
		for (size_t i = 0; i < m_listeners.size(); ++i)
		{
			if (m_listeners[i].m_listener == newListener)
			{
				//LogWarning("Attempting to add the same listener twice");
				return;
			}
		}


		RegistrationReceiptPtr receipt = CreateRegistrationReceipt(ref_ptr<this_type>(this), &this_type::RemoveListener, newListener);
		newListener->AddRegistration(receipt, receipt->GetKey());
		m_listeners.push_back(ListenerEntry(newListener, receipt->GetKey()));
	}


	void RemoveListener(T* oldListener)
	{
		// Check for a null parameter
		if (oldListener == nullptr) return;

		for (size_t i = 0; i < m_listeners.size(); ++i)
		{
			if (m_listeners[i].m_listener == oldListener)
			{
				oldListener->DeleteRegistration(m_listeners[i].m_key);
				m_listeners.erase(m_listeners.begin() + i);
				break;
			}
		}
	}

	void NotifyListeners(void (T::* func) (void))
	{
		for (int32 i = (int32)m_listeners.size() - 1; i >= 0; --i)
		{
			(m_listeners[i].m_listener->*func)();
		}
	}

	template<typename P1, typename T1>
	void NotifyListeners(void (T::* func) (P1), T1& param)
	{
		for (int32 i = (int32)m_listeners.size() - 1; i >= 0; --i)
		{
			(m_listeners[i].m_listener->*func)(param);
		}
	}

	template<typename P1, typename P2, typename T1, typename T2>
	void NotifyListeners(void (T::* func) (P1, P2), T1 param1, T2& param2)
	{
		for (int32 i = (int32)m_listeners.size() - 1; i >= 0; --i)
		{
			(m_listeners[i].m_listener->*func)(param1, param2);
		}
	}

	template<typename P1, typename P2, typename P3, typename T1, typename T2, typename T3>
	void NotifyListeners(void (T::* func) (P1, P2, P3), T1 param1, T2& param2, T3& param3)
	{
		for (int32 i = (int32)m_listeners.size() - 1; i >= 0; --i)
		{
			(m_listeners[i].m_listener->*func)(param1, param2, param3);
		}
	}

	template<typename P1, typename P2, typename T1, typename T2>
	void NotifyListener(int i, void (T::* func) (P1, P2), T1 param1, T2& param2)
	{
		(m_listeners[i].m_listener->*func)(param1, param2);
	}

	int32 GetListenerCount()
	{
		return (int32)m_listeners.size();
	}

private:
	// Ensure that this gets created as a smart pointer
	ListenerList() {}

	class ListenerEntry
	{
	public:
		ListenerEntry() : m_key(0) {}
		ListenerEntry(T* listener, RegistrationKey key) : m_listener(listener), m_key(key) {}

		T*				m_listener;
		RegistrationKey m_key;
	};

	std::vector<ListenerEntry>	m_listeners;
};

XTOOLS_NAMESPACE_END
