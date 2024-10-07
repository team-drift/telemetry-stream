/**
 * @file squeue.hpp
 * @author Owen Cochell (owencochell@gmail.com)
 * @brief A thread safe blocking queue
 * @version 0.1
 * @date 2024-10-06
 * 
 * @copyright Copyright (c) 2024
 * 
 * This file describes a thread safe blocking queue, SQueue!
 * SQueues are special in the sense that they are thread safe (no race conditions occur),
 * and that they are able to block
 * (wait for a timeout until values are present in the queue).
 */

#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>

/**
 * @brief A thread safe blocking queue
 * 
 * This class represents a queue that is both thread safe and blocking.
 * 
 * Thread safe means this class is projected from manipulation from multiple threads,
 * meaning no data races or deadlocks will occur!
 * We have an internal mutex that is utilized to ensure only one thing at a time has access.
 * 
 * Blocking means this queue can block until there are values ready to return.
 * If the queue is asked to retrieve a value, but the queue is empty,
 * then this class has the ability to wait until a value is ready to be returned.
 * We also support timeout values, so one could provide a time period to stop waiting.
 * 
 * @tparam T Type of value this queue will contain
 */
template<typename T>
class SQueue {
private:

    /// Queue object in use, stores all values
    std::queue<T> queue;

    /// Mutex to utilize
    std::mutex mutex;

    /// Condition variable to check for changes
    std::condition_variable cond;

public:

    /**
     * @brief Pushes a value into the queue
     * 
     * This function places a given value into the end of the queue.
     * We ensure the queue is not currently being accessed
     * (and will wait until it is free),
     * and we will notify the condition variable that a new value is ready to be removed.
     * 
     * Again, this function WILL block until the lock has been acquired!
     * 
     * @param val Value to push
     */
    void push(const T& val) {

        // We acquire the mutex in a special namespace,
        // This is to ensure we can release it as soon as possible!
        {
            // Acquire the mutex:

            const std::lock_guard<std::mutex> lock(this->mutex);

            // We have access! Add the value to the queue:

            this->queue.push(val);
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

        // Wait until the queue is empty, or until a timeout occurs
        // This function also prevents waking too early,
        // the lambda ensures the queue has something in it before we consider the contents

        if (!cond.wait_for(lock, timeout, [this] { return !this->queue.empty(); })) {
            return false;
        }

        // We have the mutex! Grab the value from the queue:

        val = std::move(this->queue.front());

        // Delete the value from the queue:

        this->queue.pop();

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

        // Define a value to be retrieved from the queue:

        T val;

        // Get a value from the queue, wait forever:

        this->pop_timeout(val, std::chrono::milliseconds::max());

        // Return the value:

        return val;
    }
};
