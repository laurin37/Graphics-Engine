#pragma once

#include "Event.h"
#include <vector>
#include <functional>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <algorithm>

// Event priority levels (lower number = higher priority)
enum class EventPriority 
{
    High = 0,
    Normal = 1,
    Low = 2
};

class EventBus
{
public:
    using EventCallbackFn = std::function<void(Event&)>;
    using SubscriptionId = size_t;

    EventBus() : m_nextId(0), m_debugMode(false) {}

    // Subscribe to a specific event type with priority
    SubscriptionId Subscribe(EventType type, EventCallbackFn callback, EventPriority priority = EventPriority::Normal)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        SubscriptionId id = m_nextId++;
        m_subscribers[type][static_cast<int>(priority)].push_back({id, std::move(callback)});
        
        if (m_debugMode) {
            // LOG_INFO would be called here if available
        }
        
        return id;
    }

    // Subscribe to all events in a category
    SubscriptionId SubscribeByCategory(int categoryFlags, EventCallbackFn callback, EventPriority priority = EventPriority::Normal)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        SubscriptionId id = m_nextId++;
        m_categorySubscribers[static_cast<int>(priority)].push_back({id, categoryFlags, std::move(callback)});
        
        return id;
    }

    // Unsubscribe from a specific event type
    void Unsubscribe(EventType type, SubscriptionId id)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto& priorityMap = m_subscribers[type];
        for (auto& [priority, subscriptions] : priorityMap) {
            auto it = std::remove_if(subscriptions.begin(), subscriptions.end(),
                [id](const Subscription& sub) { return sub.id == id; });
            subscriptions.erase(it, subscriptions.end());
        }
    }

    // Unsubscribe from category subscription
    void UnsubscribeCategory(SubscriptionId id)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        for (auto& [priority, subscriptions] : m_categorySubscribers) {
            auto it = std::remove_if(subscriptions.begin(), subscriptions.end(),
                [id](const CategorySubscription& sub) { return sub.id == id; });
            subscriptions.erase(it, subscriptions.end());
        }
    }

    // Queue an event for deferred processing
    void QueueEvent(std::unique_ptr<Event> event)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_eventQueue.push_back(std::move(event));
    }

    // Publish an event immediately (synchronous)
    void Publish(Event& event)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_debugMode) {
            // LOG_INFO("EventBus: Publishing " + std::string(event.GetName()));
        }
        
        // Process type-specific subscribers (High -> Normal -> Low)
        auto& priorityMap = m_subscribers[event.GetEventType()];
        for (int p = static_cast<int>(EventPriority::High); 
             p <= static_cast<int>(EventPriority::Low); 
             ++p) 
        {
            auto it = priorityMap.find(p);
            if (it != priorityMap.end()) {
                for (auto& subscription : it->second) {
                    subscription.callback(event);
                    if (event.Handled) return;
                }
            }
        }
        
        // Process category subscribers (High -> Normal -> Low)
        int categoryFlags = event.GetCategoryFlags();
        for (int p = static_cast<int>(EventPriority::High); 
             p <= static_cast<int>(EventPriority::Low); 
             ++p) 
        {
            auto it = m_categorySubscribers.find(p);
            if (it != m_categorySubscribers.end()) {
                for (auto& subscription : it->second) {
                    if (categoryFlags & subscription.categoryFlags) {
                        subscription.callback(event);
                        if (event.Handled) return;
                    }
                }
            }
        }
    }

    // Process all queued events
    void ProcessEvents()
    {
        std::vector<std::unique_ptr<Event>> eventsToProcess;
        
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            eventsToProcess = std::move(m_eventQueue);
            m_eventQueue.clear();
        }
        
        for (auto& event : eventsToProcess) {
            Publish(*event);
        }
    }

    // Enable/disable debug logging
    void SetDebugMode(bool enabled) 
    { 
        std::lock_guard<std::mutex> lock(m_mutex);
        m_debugMode = enabled; 
    }

    // Get subscriber count for debugging
    size_t GetSubscriberCount(EventType type) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        size_t count = 0;
        auto it = m_subscribers.find(type);
        if (it != m_subscribers.end()) {
            for (const auto& [priority, subscriptions] : it->second) {
                count += subscriptions.size();
            }
        }
        return count;
    }

private:
    struct Subscription
    {
        SubscriptionId id;
        EventCallbackFn callback;
    };

    struct CategorySubscription
    {
        SubscriptionId id;
        int categoryFlags;
        EventCallbackFn callback;
    };

    mutable std::mutex m_mutex;
    SubscriptionId m_nextId;
    bool m_debugMode;
    
    // EventType -> Priority -> Subscriptions
    std::unordered_map<EventType, std::unordered_map<int, std::vector<Subscription>>> m_subscribers;
    
    // Priority -> Category Subscriptions
    std::unordered_map<int, std::vector<CategorySubscription>> m_categorySubscribers;
    
    // Event queue for deferred processing
    std::vector<std::unique_ptr<Event>> m_eventQueue;
};
