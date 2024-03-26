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

using namespace mavsdk;
using namespace std::chrono;

#include <nlohmann/json.hpp>

// For convenience
using json = nlohmann::json;

// Global variable to hold telemetry data
json telemetryData = json::object();

void updateTelemetryData(const json& newData) {
    for (auto& [key, value] : newData.items()) {
        telemetryData[key] = value;
    }
}

void subscribeTelemetry(std::shared_ptr<Telemetry> telemetry, int clientSocket) {
    telemetry->subscribe_position([](Telemetry::Position position) {
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

    telemetry->subscribe_attitude_euler([clientSocket](Telemetry::EulerAngle euler_angle) {
        updateTelemetryData({
            {"roll_deg", euler_angle.roll_deg},
            {"pitch_deg", euler_angle.pitch_deg},
            {"yaw_deg", euler_angle.yaw_deg},
            {"timestamp", euler_angle.timestamp_us},
        });

        if (telemetryData.contains("relative_altitude_m") &&
            telemetryData.contains("latitude_deg") &&
            telemetryData.contains("longitude_deg") &&
            telemetryData.contains("roll_rad_s") &&
            telemetryData.contains("pitch_rad_s") &&
            telemetryData.contains("yaw_rad_s") &&
            telemetryData.contains("north_m_s") &&
            telemetryData.contains("east_m_s") &&
            telemetryData.contains("down_m_s")) {

            // Convert the updated telemetry data to a string to send
            std::string message_str = telemetryData.dump() + "\n";

            int length = message_str.length();

            // Use ssize_t for the return type of send
            ssize_t sent_length = send(clientSocket, &length, sizeof(length), 0);
            if (sent_length == -1) {
                perror("send length");
                return;
            } else if (sent_length < static_cast<ssize_t>(sizeof(length))) {
                // Handle partial send if necessary
                std::cerr << "Incomplete send for length" << std::endl;
                return;
            }

            // Send the actual telemetry data as a string
            ssize_t sent_data = send(clientSocket, message_str.c_str(), length, 0);
            if (sent_data == -1) {
                perror("send message");
                return;
            } else if (sent_data < length) {
                // Handle partial send if necessary
                std::cerr << "Incomplete send for message" << std::endl;
                return;
            }
        }
    });
}


void handle_client(int client_socket, std::shared_ptr<Telemetry> telemetry) {
    subscribeTelemetry(telemetry, client_socket);

}

int main() {

    //Create connection for MavSDK
    Mavsdk mavsdk;
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

    //Create Socker Communication between Server(this) and Client(Agogos)
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Failed to create socket." << std::endl;
        return -1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Failed to bind socket." << std::endl;
        return -1;
    }

    if (listen(serverSocket, 5) == -1) {
        std::cerr << "Failed to listen on socket." << std::endl;
        return -1;
    }

    //Server Connected
    std::cout << "Server listening on port 12345" << std::endl;

    while (true) {
        int clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == -1) {
            std::cerr << "Failed to accept client." << std::endl;
            continue;
        }

        std::thread([clientSocket, telemetry]() {
            handle_client(clientSocket, telemetry);
        }).detach();
    }

    // Cleanup
    close(serverSocket);
    return 0;
}