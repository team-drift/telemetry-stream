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
 * We define the final structure that is returned to the user.
 */

#pragma once

#include <cstdint>
#include <semaphore>
#include <mutex>
#include <list>
#include <iostream>
#include <chrono>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

/// Number of streams this component is tracking
const unsigned int STREAMS = 6;

/**
 * @brief Struct containing telemetry data
 * 
 * This struct contains all the fields that are tracked by DTS.
 * All the incoming data streams are merged into this struct,
 * which can then be read and manipulated by higher level components.
 * 
 * All units are defined in the documentation, and each section
 * uses the same units.
 * 
 * We contain the following sections:
 * 
 * Global Position Info - Latitude, Longitude, and Relative Altitude
 * Axis Velocity - Velocity in each axis (north, east, up) in meters per second
 * Axis Acceleration - Acceleration in each axis (forward, right, down) in meters per second^2
 * Airspeed Metrics - Airspeed and climb-rate
 * Angular Velocity - Angular velocity in each axis in radians per second
 * Magnetic Fields - Magnetic field readings in each axis in Gauss
 * Euler Angles - Rotations in each rotation axis (roll, pitch, yaw) in degrees
 * Euler Angle Velocity - Velocity in each rotation axis in radians
 * Other Values - Values that do not fall within specific catagories
 * 
 * TODO:
 * 
 * Change axis names from forward, right, down to normal north, east, up?
 * Standardize rotation values, such as all degrees to radians and vice a versa?
 * Better methods for converting to-and-from formats?
 */
struct DTData {

    /// Current timestamp in microseconds
    uint64_t time = 0;

    ///
    // Global Position Info
    ///

    /// Latitude in degrees
    double latitude = 0;

    /// Longitude in degrees
    double longitude = 0;

    /// Relative altitude from the base station in meters
    float altitude = 0;

    /// Time the global position data was added to struct
    std::chrono::time_point<std::chrono::high_resolution_clock> position_time;

    ///
    // Axis Velocity
    ///

    /// Velocity in north axis in meters per second
    float vnorth = 0;

    /// Velocity in east axis in meters per second
    float veast = 0;

    /// Velocity in down axis in meters per second
    float vdown = 0;

    /// Time the velocity axis data was added to struct
    std::chrono::time_point<std::chrono::high_resolution_clock> vaxis_time;

    ///
    // Airspeed Metrics
    ///

    /// Current airspeed in meters per second
    float airspeed = 0;

    /// Current climb rate in meters per second
    float climb_rate = 0;

    /// Current throttle setting percentage, 0-100
    float throttle_per = 0;

    /// Time the airspeed data was added to struct
    std::chrono::time_point<std::chrono::high_resolution_clock> airspeed_time;

    ///
    // Axis Acceleration
    ///

    /// Acceleration in forward axis in meters per second^2
    float aforward = 0;

    /// Acceleration in right axis in meters per second^2
    float aright = 0;

    /// Acceleration in down axis in meters per second^2
    float adown = 0;

    /// Time the axis acceleration axis data was added to struct
    std::chrono::time_point<std::chrono::high_resolution_clock> aaxis_time;

    ///
    // Angular Velocity
    ///

    /// Angular velocity in forward axis in radians per second
    float avforward = 0;

    /// Angular velocity in right axis in radians per second
    float avright = 0;

    /// Angular velocity in down axis in radians per second
    float avdown = 0;

    /// Time the angular velocity data was added to struct
    std::chrono::time_point<std::chrono::high_resolution_clock> vangular_time;

    ///
    // Magnetic Field
    ///

    /// Magnetic filed in forward axis in Gauss
    float gforward = 0;

    /// Magnetic field in right axis in Gauss
    float gright = 0;

    /// Magnetic field in down axis in Gauss
    float gdown = 0;

    /// Time the magnetic field data was added to struct
    std::chrono::time_point<std::chrono::high_resolution_clock> mag_time;

    ///
    // Euler Angles
    ///

    /// Roll angle in degrees
    float roll = 0;

    /// Pitch angle in degrees
    float pitch = 0;

    /// Yaw angle in degrees
    float yaw = 0;

    /// Time the euler anglers data was added to struct
    std::chrono::time_point<std::chrono::high_resolution_clock> euler_time;

    ///
    // Euler Angle Velocity
    ///

    /// Roll velocity in radians per second
    float vroll = 0;

    /// Pitch velocity in radians per second
    float vpitch = 0;

    /// Yaw velocity in radians per second
    float vyaw = 0;

    /// Time the euler angle velocity data was added to struct
    std::chrono::time_point<std::chrono::high_resolution_clock> veuler_time;

    ///
    // Other Values
    ///

    /// Temperature in celsius
    float temp = 0;

    /**
     * @brief Converts to JSON data
     *
     * This function coverts the data within this struct
     * into a JSON instance
     * (provided by nlohmann JSON library).
     * 
     * This instance can then be converted into a string or used directly.
     *
     * @return json JSON instance of the data within this class
     */
    json to_json() const;
};

/**
 * @brief Pretty prints data to an output stream
 *
 * This function will pretty-print the data in this class
 * to the provided ostream.
 * This data will NOT be easily worked with by computers,
 * and is instead intended for human consumption.
 *
 * @param output Output stream
 * @return std::ostream&
 */
std::ostream& operator<<(std::ostream& output, DTData const& dt);
