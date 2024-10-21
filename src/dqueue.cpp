/**
 * @file dqueue.cpp
 * @author Owen Cochell (owencochell@gmail.com)
 * @brief Implementations for DQueue 
 * @version 0.1
 * @date 2024-10-20
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "dqueue.hpp"

#include <mutex>
#include <iostream>

#include "struct.hpp"

void DQueue::allocate() {

    // Acquire the lock:

    const std::lock_guard<std::mutex> lock(this->mutex);

    // Emplace a value at the back of the list:

    list.emplace_back();

    // Increment the count:

    ++ccount;
}

void DQueue::pop_front() {

    // Acquire the mutex:

    const std::lock_guard<std::mutex> lock(this->mutex);

    // Remove the front element:

    list.pop_front();
}

DTData DQueue::get_data() {

    // Acquire the semaphore at the front:

    std::cout << "Waiting sep..." << "\n";

    list.begin()->latch.wait();

    // The semaphore is acquired, we can get data:
    // Nobody will write to this value going forward,
    // so we have exclusive access!

    DTData data = list.front().data;

    // Remove the front value in the list:

    pop_front();

    // Finally, return the data:

    return data;
}
