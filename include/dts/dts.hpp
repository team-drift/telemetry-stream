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
#include <array>

#include <mavsdk.h>
#include <plugins/telemetry/telemetry.h>
#include <nlohmann/json.hpp>

#include "squeue.hpp"

using json = nlohmann::json;

/// Number of streams this component is tracking
const unsigned int STREAMS = 6;

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

    /// Array of queues for each stream
    std::array<SQueue<json>, STREAMS> queues;

    /**
     * @brief Callback for saving telemetry data
     *
     * This function is called by MAVSDK when new telemetry data is available.
     * We will add the incoming data into a data strucure (TODO)
     * that will contain incoming telemetry data.
     *
     * @param data JSON Data to add to the collection
     */
    void telem_callback(const json& data, std::size_t index);

public:

    DTStream() : config(this->component_type), mavsdk(config) {}

    DTStream(const std::string& str) : connection_url(str), config(this->component_type), mavsdk(config) {}
    DTStream(std::string&& str) : connection_url(std::move(str)), config(this->component_type), mavsdk(config) {}

    ~DTStream() { this->stop(); }

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
     * We return this data as a string,
     * so it is up to the caller to decode this data into something usable (like JSON).
     * 
     * @return std::string String JSON data representing the telemetry data
     */
    std::string get_data();

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
