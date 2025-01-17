/**
 * @file deque.hpp
 * @author Swabhan Katkoori
 * @brief A thread safe deque to retrieve latest available data or data next in line
 * @version 0.1
 * @date 2024-10-06
 * 
 * @copyright Copyright (c) 2024
 * 
 * This file describes a deque to be used within DRIFT Telemetry Stream (DTS)
 */

#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <deque>

/**
 * @brief A thread safe Deque
 * 
 * This class represents a Deque that is used to retrieve the latest data
 * 
 * Thread safe! Refer to squeue.hpp for more details
 * 
 * Blocking! Refer to squeue.hpp for more details
 * 
 * @tparam T Type of value this queue will contain
 */
template<typename T>
class Deque {
private:

    /// Queue object in use, stores all values
    std::deque<T> deque;

    /// Mutex to utilize
    std::mutex mutex;

    /// Condition variable to check for changes
    std::condition_variable cond;

public:

    void push(const T& val) {

        // We acquire the mutex in a special namespace,
        // This is to ensure we can release it as soon as possible!
        {
            // Acquire the mutex:

            const std::lock_guard<std::mutex> lock(this->mutex);

            // We have access! Add the value to the queue:

            this->deque.push_front(val);
        }

        // We released the mutex, update the condition variable:

        this->cond.notify_one();
    }

    /**
     * @brief Pops a value from the queue with timeout
     * 
     * This function removes a value from the queue.
     * We ensure the queue is not current being accessed
     * (and will wait until it is free),
     * and we will block until there is a new value placed into the queue,
     * or until the timeout is reached, whatever comes first.
     * 
     * @param timeout Queue timeout in milliseconds
     * @param val Variable queue contents are placed into
     */
    bool pop_timeout(T& val, std::chrono::milliseconds timeout) {

        // Create a lock using our mutex:

        std::unique_lock<std::mutex> lock(this->mutex);

        // Wait until the queue has values to return, or until a timeout occurs
        // This function also prevents waking too early,
        // the lambda ensures the queue has something in it before we consider the contents

        if (!cond.wait_for(lock, timeout, [this] { return !this->deque.empty(); })) {
            return false;
        }

        // We have the mutex! Grab the value from the queue:

        val = std::move(this->deque.front());

        // Delete the value from the queue:

        this->deque.pop_front();

        return true;
    }

    /**
     * @brief Pops a value from the queue
     *
     * This function removes a value from the queue.
     * We ensure the queue is not current being accessed
     * (and will wait until it is free),
     * and we will block until there is a new value placed into the queue.
     *
     * @return T Value popped from queue
     */
    T pop() {

        // Create a lock using our mutex:

        std::unique_lock<std::mutex> lock(this->mutex);

        // Wait until the queue has values to return
        // This function also prevents waking too early,
        // the lambda ensures the queue has something in it before we consider the contents

        cond.wait(lock, [this] { return !this->deque.empty(); });

        // We have the mutex! Grab the value from the queue:

        T val = std::move(this->deque.front());

        // Delete the value from the queue:

        this->deque.pop_front();

        return std::move(val);
    }
};
