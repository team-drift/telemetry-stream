// #include <mavsdk/mavsdk.h>
#include <mavsdk.h>
#include <plugins/telemetry/telemetry.h>
#include <future>
#include <chrono>
#include <csignal>
#include <iostream>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <atomic>
#include <vector>
#include <nlohmann/json.hpp>
#include <mutex>
#include <cstdlib>
#include <unistd.h>

#include "dts.h"

// #include <pybind11/pybind11.h>

using namespace mavsdk;
using namespace std::chrono;
using json = nlohmann::json;

//--Global Variables
json telemetryData = json::object();
std::mutex telemetryDataMutex;

// Create Global Connection for MavSDK
Mavsdk::ComponentType componentType = Mavsdk::ComponentType::GroundStation;
Mavsdk::Configuration config(componentType);
Mavsdk mavsdkConnect(config);

//MavSDK's Telemetry
std::shared_ptr<Telemetry> telemetry;

std::atomic<bool> running(true);

std::thread telemetry_thread;

void DTStream::telem_callback(const json &newData) {
    std::lock_guard<std::mutex> lock(telemetryDataMutex);

    for (auto &[key, value] : newData.items())
    {
        telemetryData[key] = value;
    }
}

std::string DTStream::get_data() {

}

// Initialize Drone Connection via UDP Port
bool DTStream::start()
{
    // Connects to UDP
    std::cout << "Listening on " << connection_url << std::endl;

    ConnectionResult connection_result = mavsdkConnect.add_any_connection(connection_url);

    if (connection_result != ConnectionResult::Success)
    {
        std::cerr << "Connection failed: " << connection_result << std::endl;
        return -1;
    }

    // Waits for connection
    std::cout << "Waiting for drone to connect..." << std::endl;
    std::promise<std::shared_ptr<System>> prom;
    std::future<std::shared_ptr<System>> fut = prom.get_future();

    Mavsdk::NewSystemHandle handle = mavsdkConnect.subscribe_on_new_system([&prom, &handle]()
                                                                           {
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
        } });

    auto system = fut.get();
    if (!system)
    {
        std::cerr << "Failed to connect to the drone." << std::endl;
        return -1;
    }

    mavsdkConnect.unsubscribe_on_new_system(handle);

    if (!system->is_connected())
    {
        std::cerr << "System is not connected!" << std::endl;
        return -1;
    }

    // Allows for subscription to various telemetry data
    std::thread telemetry_thread(subscribe, system);
    telemetry_thread.detach();

    return 0;
}

//Updates Data
void updateTelemetryData(const json& newData) {
    std::lock_guard<std::mutex> lock(telemetryDataMutex);

    for (auto& [key, value] : newData.items()) {
        telemetryData[key] = value;
    }
}

//Allows latest data to be recieved in form of JSON structured string
std::string get_data() {
    std::lock_guard<std::mutex> lock(telemetryDataMutex);
    return telemetryData.dump();
}

//Subscribe to Various Data inputs from MavLink
void subscribe(std::shared_ptr<System> system){
    if (!system) {
        std::cerr << "System is null in subscribe function" << std::endl;
        return;
    }
    std::cout << "System is valid to subscribe" << std::endl;

    //Initialize Telemetry
    telemetry = std::make_shared<Telemetry>(system);

    auto position_handle = telemetry->subscribe_position([](Telemetry::Position position) {
        updateTelemetryData({
            {"relative_altitude_m", position.relative_altitude_m},
            {"latitude_deg", position.latitude_deg},
            {"longitude_deg", position.longitude_deg}
        });
    });
    

    telemetry->subscribe_attitude_angular_velocity_body([](Telemetry::AngularVelocityBody angularVelocity) {
        updateTelemetryData({
            {"roll_rad_s", angularVelocity.roll_rad_s},
            {"pitch_rad_s", angularVelocity.pitch_rad_s},
            {"yaw_rad_s", angularVelocity.yaw_rad_s}
        });
    });


    telemetry->subscribe_velocity_ned([](Telemetry::VelocityNed velocity) {
        updateTelemetryData({
            {"north_m_s", velocity.north_m_s},
            {"east_m_s", velocity.east_m_s},
            {"down_m_s", velocity.down_m_s}
        });
    });

    telemetry->subscribe_fixedwing_metrics([](Telemetry::FixedwingMetrics metrics) {
        updateTelemetryData({
            {"airspeed_m_s", metrics.airspeed_m_s},
            {"throttle_percentage", metrics.throttle_percentage},
            {"climb_rate_m_s", metrics.climb_rate_m_s}
        });
    });


    telemetry->subscribe_imu([](Telemetry::Imu imu) {
        json imuData;
        imuData["acceleration_forward_m_s2"] = imu.acceleration_frd.forward_m_s2;
        imuData["angular_velocity_forward_rad_s"] = imu.angular_velocity_frd.forward_rad_s;
        imuData["magnetic_field_forward_gauss"] = imu.magnetic_field_frd.forward_gauss;
        imuData["temperature_degc"] = imu.temperature_degc;
        imuData["timestamp_us"] = imu.timestamp_us;
        updateTelemetryData(imuData);
    });

    telemetry->subscribe_attitude_euler([](Telemetry::EulerAngle euler_angle) {
        updateTelemetryData({
            {"roll_deg", euler_angle.roll_deg},
            {"pitch_deg", euler_angle.pitch_deg},
            {"yaw_deg", euler_angle.yaw_deg},
            {"timestamp", euler_angle.timestamp_us},
        });
    });

    while (running) {
        std::this_thread::sleep_for(seconds(1));
    }

}

//Initialize Drone Connection via UDP Port
int connect_drone(std::string connection_url = "udp://:14540", bool subscribeToData = true){
    //Connects to UDP
    std::cout << "Listening on " << connection_url << std::endl;

    ConnectionResult connection_result = mavsdkConnect.add_any_connection(connection_url);

    if (connection_result != ConnectionResult::Success) {
        std::cerr << "Connection failed: " << connection_result << std::endl;
        return -1;
    }

    //Waits for connection
    std::cout << "Waiting for drone to connect..." << std::endl;
    std::promise<std::shared_ptr<System>> prom;
    std::future<std::shared_ptr<System>> fut = prom.get_future();

    Mavsdk::NewSystemHandle handle = mavsdkConnect.subscribe_on_new_system([&prom, &handle]() {
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

    auto system = fut.get();
    if (!system) {
        std::cerr << "Failed to connect to the drone." << std::endl;
        return -1;
    }

    mavsdkConnect.unsubscribe_on_new_system(handle);

    if (!system->is_connected()) {
        std::cerr << "System is not connected!" << std::endl;
        return -1;
    }
    
    //Allows for subscription to various telemetry data
    std::thread telemetry_thread(subscribe, system);
    telemetry_thread.detach();

    return 0;
}

//Allows exit
void signal_callback_handler(int signum) {
   std::cout << "Caught signal " << signum << std::endl;
   running = false;
   if (telemetry_thread.joinable()) {
       telemetry_thread.join();
   }
}

//Main Function prints out data as recieved
int main(){
    signal(SIGINT, signal_callback_handler);

    if (connect_drone() != 0) {
        return -1;
    }

    std::cout << "Getting Ready To Run..." << std::endl;
    while(running){
        std::cout << get_data() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    return 1;
}

// //Allow functions to be called from Python
// PYBIND11_MODULE(telemetry_stream, m) {
//     m.doc() = "C++ program packaged for Python, allows retrieval of latest data from simulated drone";

//     m.def("connect_drone", &connect_drone, "Initialize program for Autonomous Vehicle Connection");
//     m.def("get_data", &get_data, "Recieve latest telemetry data as string representing JSON structure");
// }