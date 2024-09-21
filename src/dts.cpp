#include "dts.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cstdlib>
#include <future>
#include <iostream>
#include <mutex>

#include <nlohmann/json.hpp>
#include <mavsdk.h>
#include <plugins/telemetry/telemetry.h>

using namespace mavsdk;
using namespace std::chrono;
using json = nlohmann::json;

//--Global Variables
json telemetryData = json::object();
std::mutex telemetryDataMutex;

void DTStream::telem_callback(const json &newData) {
    std::lock_guard<std::mutex> lock(telemetryDataMutex);

    for (auto &[key, value] : newData.items()) {
        telemetryData[key] = value;
    }
}

std::string DTStream::get_data() {

    std::lock_guard<std::mutex> lock(telemetryDataMutex);
    return telemetryData.dump();
}

// Initialize Drone Connection via UDP Port
bool DTStream::start() {
    // Connects to UDP
    std::cout << "Listening on " << connection_url << std::endl;

    ConnectionResult connection_result = mavsdkConnect.add_any_connection(connection_url);

    if (connection_result != ConnectionResult::Success) {
        std::cerr << "Connection failed: " << connection_result << std::endl;
        return false;
    }

    // Waits for connection
    std::cout << "Waiting for drone to connect..." << std::endl;
    std::promise<std::shared_ptr<System>> prom;
    std::future<std::shared_ptr<System>> fut = prom.get_future();

    // Add new temporary callback that gets called upon system add:
    // (Callback implemented via lambda)

    Mavsdk::NewSystemHandle handle = mavsdkConnect.subscribe_on_new_system([this, &prom, &handle]() {
        auto systems = mavsdkConnect.systems();
        std::cout << "Number of systems detected: " << systems.size() << std::endl;

        if (!systems.empty()) {
            auto system = systems.at(0);
            if (system->has_autopilot()) {
                std::cout << "Drone discovered!" << std::endl;
                
                prom.set_value(system);

            } else {
                std::cout << "Detected system does not have an autopilot." << std::endl;
                prom.set_value(nullptr);
            }
        } else {
            std::cout << "No systems found." << std::endl;
            prom.set_value(nullptr);
        } 
    });

    // Wait for system to be configured:

    auto system = fut.get();
    if (!system) {
        std::cerr << "Failed to connect to the drone." << std::endl;
        return false;
    }

    // Remove system callback:

    mavsdkConnect.unsubscribe_on_new_system(handle);

    if (!system->is_connected()) {
        std::cerr << "System is not connected!" << std::endl;
        return false;
    }

    // Initialize Telemetry
    telemetry = std::make_unique<Telemetry>(system);

    // Configure all callback functions

    auto position_handle = telemetry->subscribe_position([this](Telemetry::Position position)
                                                         { this->telem_callback({{"relative_altitude_m", position.relative_altitude_m},
                                                                                 {"latitude_deg", position.latitude_deg},
                                                                                 {"longitude_deg", position.longitude_deg}}); });

    telemetry->subscribe_attitude_angular_velocity_body([this](Telemetry::AngularVelocityBody angularVelocity)
                                                        { this->telem_callback({{"roll_rad_s", angularVelocity.roll_rad_s},
                                                                               {"pitch_rad_s", angularVelocity.pitch_rad_s},
                                                                               {"yaw_rad_s", angularVelocity.yaw_rad_s}}); });

    telemetry->subscribe_velocity_ned([this](Telemetry::VelocityNed velocity)
                                      { this->telem_callback({{"north_m_s", velocity.north_m_s},
                                                              {"east_m_s", velocity.east_m_s},
                                                              {"down_m_s", velocity.down_m_s}}); });

    telemetry->subscribe_fixedwing_metrics([this](Telemetry::FixedwingMetrics metrics)
                                           { this->telem_callback({{"airspeed_m_s", metrics.airspeed_m_s},
                                                                   {"throttle_percentage", metrics.throttle_percentage},
                                                                   {"climb_rate_m_s", metrics.climb_rate_m_s}}); });

    telemetry->subscribe_imu([this](Telemetry::Imu imu)
                             {
        json imuData;
        imuData["acceleration_forward_m_s2"] = imu.acceleration_frd.forward_m_s2;
        imuData["angular_velocity_forward_rad_s"] = imu.angular_velocity_frd.forward_rad_s;
        imuData["magnetic_field_forward_gauss"] = imu.magnetic_field_frd.forward_gauss;
        imuData["temperature_degc"] = imu.temperature_degc;
        imuData["timestamp_us"] = imu.timestamp_us;
        this->telem_callback(imuData); });

    telemetry->subscribe_attitude_euler([this](Telemetry::EulerAngle euler_angle)
                                        { this->telem_callback({
                                              {"roll_deg", euler_angle.roll_deg},
                                              {"pitch_deg", euler_angle.pitch_deg},
                                              {"yaw_deg", euler_angle.yaw_deg},
                                              {"timestamp", euler_angle.timestamp_us},
                                          }); });

    return true;
}

void DTStream::stop() {

    // Destroy the MAVSDK object:

    this->mavsdkConnect.~Mavsdk();
}
