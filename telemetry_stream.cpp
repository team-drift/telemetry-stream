// All Telemetry class methods
// https://mavsdk.mavlink.io/main/en/cpp/guide/telemetry.html#api-overview

#include <chrono>
#include <cstdint>
#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <iostream>
#include <future>
#include <memory>
#include <thread>

using namespace mavsdk;
using std::chrono::seconds;
using std::this_thread::sleep_for;

void usage(const std::string& bin_name)
{
    std::cerr << "Usage : " << bin_name << " <connection_url>\n"
              << "Connection URL format should be :\n"
              << " For TCP : tcp://[server_host][:server_port]\n"
              << " For UDP : udp://[bind_host][:bind_port]\n"
              << " For Serial : serial:///path/to/serial/dev[:baudrate]\n"
              << "For example, to connect to the simulator use URL: udp://:14540\n";
}

// Hold telemetry data received at a given time
// Purely for printing output to console
// Not to be thought of as "insurance" that we are actually
// receiving all data fields at the same time
struct TelemetryReceived {
    std::atomic<double> altitude_m = 0.0;
    std::atomic<double> airspeed_m_s = 0.0;
    std::atomic<double> throttle_percentage = 0.0;
    std::atomic<double> climb_rate_m_s = 0.0;
    std::atomic<double> acceleration_forward_m_s2 = 0.0;
    std::atomic<double> angular_velocity_forward_rad_s = 0.0;
};

// Prints telemetry data that was received at a given time (the same time)
void print_telemetry(const TelemetryReceived& telemetry_received) {
    while (true) {
        std::cout << "Altitude: " << telemetry_received.altitude_m << " m\n";
        std::cout << "Airspeed: " << telemetry_received.airspeed_m_s << " m/s\n";
        std::cout << "Throttle Percentage: " << telemetry_received.throttle_percentage << " %\n";
        std::cout << "Climb rate: " << telemetry_received.climb_rate_m_s << " m/s\n";
        std::cout << "Acceleration (Forward): " << telemetry_received.acceleration_forward_m_s2 << " m/s^2\n";
        std::cout << "Angular Velocity (Forward): " << telemetry_received.angular_velocity_forward_rad_s << " rad/s\n";
        sleep_for(seconds(1));
    }
}


int main(int argc, char** argv)
{
    // Ensure 2 arguments are provided (the program name and the connection URL)
    if (argc != 2) {
        usage(argv[0]);
        return 1;
    }

    const std::string program_name = argv[0];
    const std::string connection_url = argv[1];

    std::cout << "Program Name: " << program_name << '\n';
    std::cout << "Connection URL: " << connection_url << '\n';

    // Initialize mavsdk object with GroundStation component type; establish connection to drone
    Mavsdk mavsdk;
    ConnectionResult connection_result = mavsdk.add_any_connection(connection_url);

    // Ensure connection is successful
    if (connection_result != ConnectionResult::Success) {
        std::cerr << "Connection failed: " << connection_result << '\n';
        return 1;
    }

    // Search for autopilot (drone) system
    auto systems = mavsdk.systems();
    if (!system) {
        std::cerr << "Timed out waiting for system\n";
        return 1;
    }

    // Instantiate plugins
    auto system = systems.at(0);

    auto telemetry = Telemetry{system};
    auto action = Action{system};

    // Set rate for position updates
    // https://mavsdk.mavlink.io/main/en/cpp/api_reference/classmavsdk_1_1_telemetry.html#classmavsdk_1_1_telemetry_1a665439f3d5f8c58b3ef3dd427cf4782b
    const auto set_rate_position_result = telemetry.set_rate_position(1.0);
    if (set_rate_position_result != Telemetry::Result::Success) {
        std::cerr << "Setting rate for Position failed: " << set_rate_position_result << '\n';
        return 1;
    }


    // Set rate for FixedwingMetrics updates
    // https://mavsdk.mavlink.io/main/en/cpp/api_reference/classmavsdk_1_1_telemetry.html#classmavsdk_1_1_telemetry_1ab345a5925d132c27e0a5e1ab65a1e2c1
    const auto set_rate_fixedwing_metrics_result = telemetry.set_rate_fixedwing_metrics(1.0);
    if (set_rate_fixedwing_metrics_result != Telemetry::Result::Success) {
        std::cerr << "Setting rate for FixedwingMetrics failed: " << set_rate_fixedwing_metrics_result << '\n';
        return 1;
    }

    // Set rate for Imu updates
    // https://mavsdk.mavlink.io/main/en/cpp/api_reference/classmavsdk_1_1_telemetry.html#classmavsdk_1_1_telemetry_1a4e0d1dc2350e06f68f472d85dc69d175
    const auto set_rate_imu_result = telemetry.set_rate_imu(1.0);
    if (set_rate_imu_result != Telemetry::Result::Success) {
        std::cerr << "Setting rate for Imu failed: " << set_rate_imu_result << '\n';
        return 1;
    }
    

    TelemetryReceived telemetry_received;
    std::thread print_thread(print_telemetry, std::ref(telemetry_received));

    // Set up callback to monitor <insert data fields> data fields of Position
    // https://mavsdk.mavlink.io/main/en/cpp/api_reference/structmavsdk_1_1_telemetry_1_1_position.html
    telemetry.subscribe_position([&telemetry_received](Telemetry::Position position) {
        telemetry_received.altitude_m = position.relative_altitude_m;

        // std::cout << "Altitude: " << position.relative_altitude_m << " m\n";
    });
    
    // Set up callback to monitor ALL data fields of FixedwingMetrics
    // https://mavsdk.mavlink.io/main/en/cpp/api_reference/structmavsdk_1_1_telemetry_1_1_fixedwing_metrics.html
    telemetry.subscribe_fixedwing_metrics([&telemetry_received](Telemetry::FixedwingMetrics fixedwing_metrics) {
        telemetry_received.airspeed_m_s = fixedwing_metrics.airspeed_m_s;
        telemetry_received.throttle_percentage = fixedwing_metrics.throttle_percentage;
        telemetry_received.climb_rate_m_s = fixedwing_metrics.climb_rate_m_s;

        // std::cout << "Airspeed: " << fixedwing_metrics.airspeed_m_s << " m/s\n";
        // std::cout << "Throttle Percentage: " << fixedwing_metrics.throttle_percentage << " %\n";
        // std::cout << "Climb rate: " << fixedwing_metrics.climb_rate_m_s << " m/s\n";
    });

    // Set up callback to monitor <insert data fields> data fields of Imu
    // Imu data fields: https://mavsdk.mavlink.io/main/en/cpp/api_reference/structmavsdk_1_1_telemetry_1_1_imu.html
    telemetry.subscribe_imu([&telemetry_received](Telemetry::Imu imu) {
        telemetry_received.acceleration_forward_m_s2 = imu.acceleration_frd.forward_m_s2;
        telemetry_received.angular_velocity_forward_rad_s = imu.angular_velocity_frd.forward_rad_s;


        // std::cout << "Acceleration (Forward): " << imu.acceleration_frd.forward_m_s2 << " m/s^2\n";
        // std::cout << "Angular Velocity (Forward): " << imu.angular_velocity_frd.forward_rad_s << " rad/s\n";
    });

    // Check until vehicle is ready to arm
    while (telemetry.health_all_ok() != true) {
        std::cout << "Vehicle is getting ready to arm\n";
        sleep_for(seconds(1));
    }

    // Arm vehicle
    std::cout << "Arming...\n";
    const Action::Result arm_result = action.arm();

    if (arm_result != Action::Result::Success) {
        std::cerr << "Arming failed: " << arm_result << '\n';
        return 1;
    }

    // We are relying on auto-disarming but let's keep watching the telemetry for a bit longer.
    sleep_for(seconds(100));
    std::cout << "Finished...\n";

    return 0;
}