/**
 * @file struct.hpp
 * @author Owen Cochell (owencochell@gmail.com)
 * @brief Various structures to be utilized DTS
 * @version 0.1
 * @date 2024-10-08
 * 
 * @copyright Copyright (c) 2024
 * 
 * This file describes some structures that are used throughout this project.
 * We define the final structure that is returned to the user,
 * as well as the structure that represents a value in the queue.
 */

#pragma once

#include <cstdint>
#include <semaphore>
#include <mutex>
#include <list>

/**
 * @brief Represents telemetry data
 * 
 * This struct contains all the necessary info required
 * for tracking a drone in flight.
 * 
 */
struct DData {

    /// Current timestamp in microseconds
    uint64_t time;

    ///
    // Global Position Info
    ///

    /// Latitude in degrees
    double latitude;

    /// Longitude in degrees
    double longitude;

    ///
    // Axis Velocity
    ///

    /// Velocity in north axis in meters per second
    float snorth;

    /// Velocity in east axis in meters per second
    float seast;

    /// Velocity in down axis in meters per second
    float sdown;

    /// Current airspeed in meters per second
    float airspeed;

    /// Current climb rate in meters per second
    float climb_rate;

    ///
    // Axis Acceleration
    ///

    /// Acceleration in forward axis in meters per second^2
    float aforward;

    /// Acceleration in right axis in meters per second^2
    float aright;

    /// Acceleration in down axis in meters per second^2
    float adown;

    ///
    // Angular Velocity
    ///

    /// Angular velocity in forward axis in radians per second
    float avforward;

    /// Angular velocity in right axis in radians per second
    float avright;

    /// Angular velocity in down axis in radians per second
    float avdown;

    ///
    // Magnetic Field
    ///

    /// Magnetic filed in forward axis in Gauss
    float gforward;

    /// Magnetic field in right axis in Gauss
    float gright;

    /// Magnetic field in down axis in Gauss
    float gdown;

    ///
    // Euler Angles
    ///

    /// Roll angle in degrees
    float roll;

    /// Pitch angle in degrees
    float pitch;

    /// Yaw angle in degrees
    float yaw;

    ///
    // Euler Angle Velocity
    ///

    /// Roll speed in radians per second
    float sroll;

    /// Pitch speed in radians per second
    float spitch;

    /// Yaw speed in radians per second
    float syaw;

    ///
    // Other Values
    ///

    /// Temperature in celsius
    float temp;

    /// Current throttle setting percentage, 0-100
    float throttle_per;

};

struct QValue {

    /// Data to be stored in the list
    DData data;

    /// Mutex to utilize for locking
    std::mutex mutex;

    /// Semaphore representing the amount of values added
    std::counting_semaphore<6> seph;

    QValue() : seph(0) {}
};

class DQueue {
private:

    std::list<QValue> tlist;

    uint64_t count;

    std::mutex mutex;

public:

    DQueue() =default;

    void allocate() {

        // Acquire the lock:

        const std::lock_guard<std::mutex> lock(this->mutex);

        // Emplace a value at the back of the list:

        tlist.emplace_back();

        // Increment the count:

        ++count;
    }

    QValue& front() {

        return tlist.front();
    }

    auto begin() {

        return tlist.begin();
    }

    uint16_t get_count() const {

        const std::lock_guard<std::mutex> lock(this->mutex);

        return this->count;
    }

    void pop_front() {

        // Acquire the mutex:

        const std::lock_guard<std::mutex> lock(this->mutex);

        // Remove the front element:

        tlist.pop_front();
    }
};

void junk() {

    DQueue q;

    // Define static value for keeping count:

    static uint16_t count = 0;

    // Determine if we need to allocate new list value:

    if (count > q.get_count()) {
        q.allocate();
    }

    static auto iter = q.begin();

    // Determine if we should advance:

    if (count != 0) {
        // Increment:

        ++iter;
    }

    // Alter the fields:

    iter->data.adown = 0;
    iter->data.aforward = 0;

    // Release the sephamore:

    iter->seph.release();

    // Finally, increment the count:

    ++count;
}

void get_list() {

    DQueue q;

    // Get the front of the queue:

    q.front().seph.acquire();

    // Front of the queue is good, save it:

    DData val = q.front().data;

    // Remove the first value:

    q.pop_front();
}
