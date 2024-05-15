#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
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
#include <cstdlib>

using namespace mavsdk;
using namespace std::chrono;

#include <nlohmann/json.hpp>

// For convenience
using json = nlohmann::json;

class Telemetry_Stream {
    private:
        //To allow handling of Keyboard Interrupts
        std::atomic<bool> keepRunning = true;

        // Global variable to hold telemetry data
        json telemetryData = json::object();

    public:
        void updateTelemetryData(const json& newData) {
            for (auto& [key, value] : newData.items()) {
                telemetryData[key] = value;
            }
        }

        void subscribeTelemetry(std::shared_ptr<Telemetry> telemetry) {
            telemetry->subscribe_position([this](Telemetry::Position position) {
                updateTelemetryData({
                    {"relative_altitude_m", position.relative_altitude_m},
                    {"latitude_deg", position.latitude_deg},
                    {"longitude_deg", position.longitude_deg}
                });
            });

            telemetry->subscribe_attitude_angular_velocity_body([this](Telemetry::AngularVelocityBody angularVelocity) {
                updateTelemetryData({
                    {"roll_rad_s", angularVelocity.roll_rad_s},
                    {"pitch_rad_s", angularVelocity.pitch_rad_s},
                    {"yaw_rad_s", angularVelocity.yaw_rad_s}
                });
            });

            telemetry->subscribe_velocity_ned([this](Telemetry::VelocityNed velocity) {
                updateTelemetryData({
                    {"north_m_s", velocity.north_m_s},
                    {"east_m_s", velocity.east_m_s},
                    {"down_m_s", velocity.down_m_s}
                });
            });

            telemetry->subscribe_fixedwing_metrics([this](Telemetry::FixedwingMetrics metrics) {
                updateTelemetryData({
                    {"airspeed_m_s", metrics.airspeed_m_s},
                    {"throttle_percentage", metrics.throttle_percentage},
                    {"climb_rate_m_s", metrics.climb_rate_m_s}
                });
            });

            telemetry->subscribe_imu([this](Telemetry::Imu imu) {
                json imuData;

                imuData["acceleration_forward_m_s2"] = imu.acceleration_frd.forward_m_s2;
                imuData["angular_velocity_forward_rad_s"] = imu.angular_velocity_frd.forward_rad_s;
                imuData["magnetic_field_forward_gauss"] = imu.magnetic_field_frd.forward_gauss;
                imuData["temperature_degc"] = imu.temperature_degc;
                imuData["timestamp_us"] = imu.timestamp_us;

                updateTelemetryData(imuData);
            });

            telemetry->subscribe_attitude_euler([this](Telemetry::EulerAngle euler_angle) {
                updateTelemetryData({
                    {"roll_deg", euler_angle.roll_deg},
                    {"pitch_deg", euler_angle.pitch_deg},
                    {"yaw_deg", euler_angle.yaw_deg},
                    {"timestamp", euler_angle.timestamp_us},
                });
            });
        }

    std::string get_packet(){
        // Convert the updated telemetry data to a string, easy to read
        std::string message_str = telemetryData.dump() + "\n";

        return message_str;
    }


    int start_process(){
        //Create connection for MavSDK
        Mavsdk::ComponentType componentType = Mavsdk::ComponentType::Autopilot;

        // Create a configuration object for Mavsdk
        Mavsdk::Configuration config(componentType);

        // Create an instance of Mavsdk using the configuration
        Mavsdk mavsdk(config);
        
        const std::string connection_url = "udp://0.0.0.0:14540";
        ConnectionResult connection_result = mavsdk.add_any_connection(connection_url);

        if (connection_result != ConnectionResult::Success) {
            std::cerr << "Connection failed: " << connection_result << std::endl;
            return -1;
        }

        //Connect Drone
        std::cout << "Waiting for drone to connect..." << std::endl;
        auto prom = std::promise<std::shared_ptr<System>>();
        auto fut = prom.get_future();
        mavsdk.subscribe_on_new_system([&mavsdk, &prom]() {
            auto system = mavsdk.systems().at(0);

            if (system->has_autopilot()) {
                std::cout << "Drone discovered!" << std::endl;
                prom.set_value(system);
            }
        });

        if (fut.wait_for(seconds(2)) == std::future_status::timeout) {
            std::cerr << "No drone found, exiting." << std::endl;
            return -1;
        }

        //Initialize Telemetry
        auto system = fut.get();
        auto telemetry = std::make_shared<Telemetry>(system);

        //Starts process of receiving telemetry data
        subscribeTelemetry(telemetry);

        return 1;
    }
};

int main() {
    Telemetry_Stream t = Telemetry_Stream();
    t.start_process();

    std::cout << t.get_packet();
    return 0;
}
