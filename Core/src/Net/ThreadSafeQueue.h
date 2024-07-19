#pragma once

#include <deque>
#include <mutex>

// Queue
template<typename T>
class ThreadSafeQueue 
{
public:
    ThreadSafeQueue() = default;
    ThreadSafeQueue(const ThreadSafeQueue<T>&) = delete; // Delete copy constructor
    virtual ~ThreadSafeQueue()
    {
        clear();
    }

public:
    // Returns and maintains item at front of Queue
    const T& front()
    {
        std::scoped_lock lock(m_queueMutex);
        return m_deque.front();
    }

    // Returns and maintains item at back of Queue
    const T& back()
    {
        std::scoped_lock lock(m_queueMutex);
        return m_deque.back();
    }

    // Removes and returns item from front of Queue
    T pop_front()
    {
        std::scoped_lock lock(m_queueMutex);
        auto t = std::move(m_deque.front());
        m_deque.pop_front();
        return t;
    }

    // Removes and returns item from back of Queue
    T pop_back()
    {
        std::scoped_lock lock(m_queueMutex);
        auto t = std::move(m_deque.back());
        m_deque.pop_back();
        return t;
    }

    // Adds an item to back of Queue
    void push_back(const T& item)
    {
        std::scoped_lock lock(m_queueMutex);
        m_deque.emplace_back(std::move(item));

        //std::unique_lock<std::mutex> ul(m_blockingMutex);
        m_cvBlocking.notify_one();
    }

    // Adds an item to front of Queue
    void push_front(const T& item)
    {
        std::scoped_lock lock(m_queueMutex);
        m_deque.emplace_front(std::move(item));

        //std::unique_lock<std::mutex> ul(m_blockingMutex);
        m_cvBlocking.notify_one();
    }

    // Returns true if Queue has no items
    bool empty()
    {
        std::scoped_lock lock(m_queueMutex);
        return m_deque.empty();
    }

    // Returns number of items in Queue
    size_t count()
    {
        std::scoped_lock lock(m_queueMutex);
        return m_deque.size();
    }

    // Clears Queue
    void clear()
    {
        std::scoped_lock lock(m_queueMutex);
        m_deque.clear();
    }

    void wait()
    {
        //std::unique_lock<std::mutex> ul(m_blockingMutex);
        //while (empty()) {
        //    m_cvBlocking.wait(ul);
        //}

        //std::unique_lock<std::mutex> ul(m_queueMutex);
        //while (empty()) {
        //    m_cvBlocking.wait(m_queueMutex, [&]() {return !empty(); });
        //}
    }

protected:
    //std::mutex m_blockingMutex;
    std::mutex m_queueMutex;
    std::deque<T> m_deque;
    std::condition_variable m_cvBlocking;
};