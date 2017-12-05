////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

#include <mutex>
#include <memory>
#include <vector>

namespace Neso
{
    ////////////////////////////////////////////////////////////////////////////////
    // ListenerCollection
    // Collection of weak_ptr's and helper functions to remove objects that don't exist anymore
    template<typename T>
    class ListenerCollection
    {
    public:
        void Add(std::shared_ptr<T> listener)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_listeners.push_back(std::move(listener));
        }

        void Remove(std::shared_ptr<T> listener)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            erase_if(&m_listeners, [&listener](const std::weak_ptr<T>& weak)
            {
                std::shared_ptr<T> ptr = weak.lock();
                return ptr == listener;
            });
        }

        std::vector<std::shared_ptr<T>> PurgeAndGetListeners()
        {
            std::vector<std::shared_ptr<T>> result;
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                erase_if(&m_listeners, [&result](const std::weak_ptr<T>& weak)
                {
                    std::shared_ptr<T> ptr = weak.lock();
                    if (ptr)
                    {
                        result.push_back(std::move(ptr));
                        return false;
                    }
                    else
                    {
                        return true;
                    }
                });
            }
            return result;
        }

        virtual ~ListenerCollection()
        {
            fail_fast_if(!m_listeners.empty());
        }

    private:
        std::mutex m_mutex;
        std::vector<std::weak_ptr<T>> m_listeners;
    };
}
