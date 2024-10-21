/**
 * @file dtstream.hpp
 * @author Owen Cochell (owencochell@gmail.com)
 * @brief Main header file for DRIFT telemetry stream
 * @version 0.1
 * @date 2024-09-16
 * 
 * @copyright Copyright (c) 2024
 * 
 * This file contains some functions that are required for the DRIFT telemetry stream.
 * This project handles the configuration and extraction process for MAVSDK.
 * The idea is to simplify the process of getting data from MAVADK,
 * and provdes a standard data format that all DRIFT projects will adhear to.
 */

#pragma once

#include <utility>
#include <memory>

#include <mavsdk.h>
#include <plugins/telemetry/telemetry.h>
#include <nlohmann/json.hpp>

#include "struct.hpp"
#include "dqueue.hpp"

using json = nlohmann::json;

/**
 * @brief Entry point for all telemetry operations
 * 
 * This class represents a MAVSDK telemetry stream.
 * We automatically configure and connect a MAVSDK instance,
 * allowing users to retrieve telemetry data.
 * Users can call our class methods and we will return telemetry data in JSON format.
 * 
 * In addition, this class also manages the process of 'merging' streams together,
 * allowing for incoming data on alternate streams to be collected and considered as one.
 * 
 */
class DTStream {
private:

    /// Connection URL to utilize
    std::string connection_url = "udp://:14540";

    /// Component type (we hardcode to ground station)
    mavsdk::Mavsdk::ComponentType component_type = mavsdk::Mavsdk::ComponentType::GroundStation;

    /// MAVSDK configuration instance
    mavsdk::Mavsdk::Configuration config;

    /// MAVSDK instance to utilize
    mavsdk::Mavsdk mavsdk;

    /// Telemetry pointer
    std::unique_ptr<mavsdk::Telemetry> telemetry;

    /**
     * @brief Updates the count and iter values for a callback
     *
     * Each stream callback maintains a count
     * (how many values have been added to the queue)
     * and an iterator
     * (read/write access to a particular value in the queue).
     *
     * This function updates each of these things,
     * along with altering the queue size if necessary.
     * We also handle the semaphore state so other components
     * can identify when a value is ready to be removed.
     * 
     * Callbacks should invoke this function AFTER alterations are complete.
     * Doing so before can lead to skipping and weird queue states!
     *
     * @param count Current count of the callback
     * @param iter Current iterator of the callback
     */
    void call_update(uint64_t& count, dqiter& iter);

public:

    DTStream() : config(this->component_type), mavsdk(config) {}

    DTStream(std::string str) : connection_url(std::move(str)), config(this->component_type), mavsdk(config) {}

    ~DTStream() { this->stop(); }

    /// Special drift queue, blah blah blah
    DQueue dq;

    /**
     * @brief Sets the connection string
     * 
     * This string must be set BEFORE this class is started!
     * 
     * @param cstr New connection string to utilize
     */
    void set_cstr(std::string cstr) { this->connection_url = std::move(cstr); }

    /**
     * @brief Gets the connection string
     * 
     * @return const std::string& Connection string utilized
     */
    const std::string& get_cstr() const { return this->connection_url; }

    /**
     * @brief Gets the latest telemetry packet
     * 
     * We retrieve the latest packet and remove it from the internal structure.
     * We utilize the DTData struct to represent the telemetry data.
     * 
     * @return DTData Struct containing telemetry data 
     */
    DTData get_data();

    /**
     * @brief Preforms all required start operations
     * 
     * This function prepares this instance for communicating
     * with a system via MAVLINK.
     * We preform the following:
     * 
     * - Create required components and structures
     * - Connect to any added systems and determine if they are eligible
     * - Add callback functions to react to incoming telemetry data
     * 
     * All these steps are REQUIRED for proper functionality,
     * and this function MUST be called before any operations are preformed. 
     * 
     * @return bool true if successful, false if not
     */
    bool start();

    /**
     * @brief Preforms all required stop operations
     * 
     * This function destroys the MAVSDK object.
     * This will ensure all resources are freed and all background threads are killed.
     * This function will be called automatically when this object is destroyed,
     * but it can be called automatically if necessary.
     * 
     * Once a telemetry object is stopped,
     * then it CAN'T be restarted or used again!
     */
    void stop();
};
