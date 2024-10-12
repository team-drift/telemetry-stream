#include "dts.hpp"

#include <cstddef>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>

#include <mavsdk.h>
#include <plugins/telemetry/telemetry.h>

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

using json = nlohmann::json;

//--Global Variables
json telemetryData = json::object();
std::mutex telemetryDataMutex;

void DTStream::telem_callback(const json &newData, std::size_t index) {

    // Add the JSON data to the queue:

    this->queues[index].push(newData);
}

std::string DTStream::get_data() {

    // Final JSON data:

    json final_data;

    // We need to get a piece of data from each queue

    for (std::size_t i = 0; i < this->queues.size(); ++i) {

        // Get value from this queue:

        final_data[i] = this->queues[i].pop();
    }

    // Return the final data:

    return final_data.dump();
}

// Initialize Drone Connection via UDP Port
bool DTStream::start() {
    // Connects to UDP
    std::cout << "Listening on " << connection_url << '\n';

    const mavsdk::ConnectionResult connection_result = mavsdk.add_any_connection(connection_url);

    if (connection_result != mavsdk::ConnectionResult::Success) {
        std::cerr << "Connection failed: " << connection_result << '\n';
        return false;
    }

    // Waits for connection
    std::cout << "Waiting for drone to connect..." << '\n';
    std::promise<std::shared_ptr<mavsdk::System>> prom;
    std::future<std::shared_ptr<mavsdk::System>> fut = prom.get_future();

    // Add new temporary callback that gets called upon system add:
    // (Callback implemented via lambda)

    const mavsdk::Mavsdk::NewSystemHandle handle = mavsdk.subscribe_on_new_system([this, &prom]() {
        auto systems = mavsdk.systems();
        std::cout << "Number of systems detected: " << systems.size() << '\n';

        if (!systems.empty()) {
            auto system = systems.at(0);
            if (system->has_autopilot()) {
                std::cout << "Drone discovered!" << '\n';

                prom.set_value(system);

            } else {
                std::cout << "Detected system does not have an autopilot."
                          << '\n';
                prom.set_value(nullptr);
            }
        } else {
            std::cout << "No systems found." << '\n';
            prom.set_value(nullptr);
        } 
    });

    // Wait for system to be configured:

    auto system = fut.get();
    if (!system) {
        std::cerr << "Failed to connect to the drone." << '\n';
        return false;
    }

    // Remove system callback:

    mavsdk.unsubscribe_on_new_system(handle);

    if (!system->is_connected()) {
        std::cerr << "System is not connected!" << '\n';
        return false;
    }

    // Initialize Telemetry
    telemetry = std::make_unique<mavsdk::Telemetry>(system);

    // Configure all callback functions

    telemetry->subscribe_position([this](mavsdk::Telemetry::Position position)
                                                         { this->telem_callback({{"relative_altitude_m", position.relative_altitude_m},
                                                                                 {"latitude_deg", position.latitude_deg},
                                                                                 {"longitude_deg", position.longitude_deg}}, 0); });

    telemetry->subscribe_attitude_angular_velocity_body([this](mavsdk::Telemetry::AngularVelocityBody angularVelocity)
                                                        { this->telem_callback({{"roll_rad_s", angularVelocity.roll_rad_s},
                                                                               {"pitch_rad_s", angularVelocity.pitch_rad_s},
                                                                               {"yaw_rad_s", angularVelocity.yaw_rad_s}}, 1); });

    telemetry->subscribe_velocity_ned([this](mavsdk::Telemetry::VelocityNed velocity)
                                      { this->telem_callback({{"north_m_s", velocity.north_m_s},
                                                              {"east_m_s", velocity.east_m_s},
                                                              {"down_m_s", velocity.down_m_s}}, 2); });

    telemetry->subscribe_fixedwing_metrics([this](mavsdk::Telemetry::FixedwingMetrics metrics)
                                           { this->telem_callback({{"airspeed_m_s", metrics.airspeed_m_s},
                                                                   {"throttle_percentage", metrics.throttle_percentage},
                                                                   {"climb_rate_m_s", metrics.climb_rate_m_s}}, 3); });

    telemetry->subscribe_imu([this](mavsdk::Telemetry::Imu imu)
                             {
        json imuData;
        imuData["acceleration_forward_m_s2"] = imu.acceleration_frd.forward_m_s2;
        imuData["angular_velocity_forward_rad_s"] = imu.angular_velocity_frd.forward_rad_s;
        imuData["magnetic_field_forward_gauss"] = imu.magnetic_field_frd.forward_gauss;
        imuData["temperature_degc"] = imu.temperature_degc;
        imuData["timestamp_us"] = imu.timestamp_us;
        this->telem_callback(imuData, 4); });

    telemetry->subscribe_attitude_euler([this](mavsdk::Telemetry::EulerAngle euler_angle)
                                        { this->telem_callback({
                                              {"roll_deg", euler_angle.roll_deg},
                                              {"pitch_deg", euler_angle.pitch_deg},
                                              {"yaw_deg", euler_angle.yaw_deg},
                                              {"timestamp", euler_angle.timestamp_us},
                                          }, 5); });

    return true;
}

void DTStream::stop() {

    // Destroy the MAVSDK object:

    this->mavsdk.~Mavsdk();
}
