/**
 * @file dqueue.hpp
 * @author Owen Cochell (owencochell@gmail.com)
 * @brief A DTS tailored queue
 * @version 0.1
 * @date 2024-10-20
 * 
 * @copyright Copyright (c) 2024
 * 
 * This file defines a queue that is designed to work specifically with DTS.
 * These components are utilized internally in the DTStream class,
 * and are designed specifically to work optimally with this project.
 */

#pragma once

#include <list>
#include <mutex>
#include <latch>
#include <cstdint>
#include <atomic>

#include "struct.hpp"

/**
 * @brief A value in the DQueue
 * 
 * This struct represents a value in the DQueue.
 * We hold some DTData that represents the telemetry data,
 * and a semaphore for syncronization.
 * 
 * Callbacks will decrement this latch as they add to the DTData.
 * This latch will be waited upon when removing values,
 * meaning when this latch is successfully acquired,
 * this value will NO LONGER be altered.
 * The components that acquires this value will have EXCLUSIVE ACCESS!
 * This means the data can then be copied, and the value can be destroyed.
 * 
 * This component is used internally and SHOULD NOT be used by anything other than DQueue.
 */
struct DValue {

    /// Data to be stored in the list
    DTData data;

    /// Latch representing the amount of values added
    std::latch latch;

    DValue() : latch(STREAMS) {}
};

/**
 * @brief DRIFT Queue - A queue designed to work with telemetry data.
 * 
 * This is a custom 'queue' implementation that is designed specifically for this project.
 * We have the following goals:
 * 
 * - Fast front removal
 * - Fast back emplacement
 * - Seek to arbitrary positions within the queue
 * - Thread-safety (operations are exclusive)
 * 
 * We use the C++ list under the hood to implement the first three goals.
 * We wrap some entry point functions to ensure that this structure is thread safe.
 * DTValues are the only thing this queue contains,
 * and it contains the data to be worked with and some syncronization primatives
 * to determine when values are ready to be removed.
 * Finally, this class contains a 'count' value,
 * which is NOT the size of the queue,
 * but instead the number of items that have been added to the queue over the course of it's lifetime.
 * 
 * The general idea of this class (and the callback system) is this:
 * 
 * N callbacks (6 at the time of writing) are defined which MAY run in parallel
 * (at the time of writing, MAVSDK only executes one callback at a time, so execution is synchonous).
 * Each callback will be receiving a 'stream' of data, which contains a little bit of info.
 * We want to merge this data into one structure, which contains data from all streams.
 * Each stream may get received faster than others, so we may receive data that is 'in the future',
 * which need to be synchronized and stored accordingly.
 * 
 * Each of the callbacks will iterate over this queue via iterators,
 * and will place their data into the structure.
 * This queue will initialize with one value, so these callbacks will start by altering
 * the value pointed to the 'begin' iterator.
 * Each callback is guaranteed to NOT alter the same attributes,
 * so locking on these attributes is unecessary. 
 * After the data has been saved, then the number of values the callback has added
 * (including the value added in the current call) is compared to the queue count.
 * If this exceeds the queue count, then a new value is emplaced at the back of the queue,
 * and the iterator is incremented.
 * The nice thing about C++ list iterators is they do NOT become invalid when the list changes.
 * They ONLY become invalid if the value they are pointing to is destroyed.
 * This means we can construct these callbacks to report a value is ready to be destroyed AFTER
 * it's iterator has been incremented, meaning we can sidestep this entire issue.
 * Finally, the latch of the current value is decremented, signaling a portion of the data has been filled in.
 * The callback get called again, rinse and repeat!
 * 
 * Getting values from this queue is easier.
 * Users simply call the get_data() method!
 * it will block and return a DTData that represents telemetry data received in a certain order. 
 * 
 * This allows for multiple callbacks to alter the queue and it's contents in parallel.
 * The only locking that is required is locking on this class when allocating or removing values
 * (the count value is atomic).
 * This removes many instances where locking is required, and allows for the above constraints to be met correctly.
 * 
 */
class DQueue {
private:

    /// Underlying list to utilize
    std::list<DValue> list;

    /// Total number of values added to the queue (counts up)
    std::atomic<uint64_t> ccount = 0;

    /// Mutex to utilize for locking and thread safety
    std::mutex mutex;

public:

    DQueue() {

        // ALWAYS have an initial value allocated:

        allocate();
    }

    /**
     * @brief Allocates a DTData value at the end of the list
     * 
     * This thread-safe function will emplace a DTData value
     * at the end of the queue, and will increment the count value.
     * This is the only way to add data to the queue!
     * 
     */
    void allocate();

    /**
     * @brief Removes the front value from the queue
     *
     * This thread-safe function will remove the first value in this queue.
     * The first value will be DESTROYED upon calling this function.
     *
     */
    void pop_front();

    /**
     * @brief Gets the latest data from the queue
     *
     * This thread-safe blocking function will wait
     * until we have valid data at the front of the queue.
     * We will return the data to the user and remove it from the queue.
     *
     * @return Newest DTData
     */
    DTData get_data();

    /**
     * @brief Gets an iterator pointing to the start of the list
     * 
     * @return auto Start iterator
     */
    auto begin() { return list.begin(); }

    /**
     * @brief Gets the count of this queue
     * 
     * The queue 'count' is the total number of values
     * added to this queue.
     * This does NOT represent the current values in the queue
     * (use 'size()' for that).
     * 
     * This function is thread-safe as it utilizes atomic values.
     * 
     * @return uint16_t Total number of values added to this queue
     */
    uint64_t count() const { return this->ccount; }

    /**
     * @brief Gets the size of the queue
     * 
     * This value represents the number of items currently in the list.
     * 
     * @return auto Size of this queue
     */
    auto size() const { return this->list.size(); }
};

/// DQueue iterator type
using dqiter = std::list<DValue>::iterator;
