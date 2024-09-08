#include <pybind11/pybind11.h>
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
#include <nlohmann/json.hpp>
#include <mutex>

// Pybind11 namespace
namespace py = pybind11;
using namespace mavsdk;
using namespace std::chrono;
using json = nlohmann::json;

// Global telemetry data
json telemetryData = json::object();
std::mutex telemetryDataMutex;
std::thread server_thread;
bool server_running = false;

void updateTelemetryData(const json &newData) {
    std::lock_guard<std::mutex> lock(telemetryDataMutex);
    for (auto &[key, value] : newData.items()) {
        telemetryData[key] = value;
    }
}

void subscribeTelemetry(std::shared_ptr<Telemetry> telemetry) {
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

    telemetry->subscribe_attitude_euler([](Telemetry::EulerAngle euler_angle) {
        updateTelemetryData({
            {"roll_deg", euler_angle.roll_deg},
            {"pitch_deg", euler_angle.pitch_deg},
            {"yaw_deg", euler_angle.yaw_deg},
            {"timestamp", euler_angle.timestamp_us},
        });
    });
}

std::string getData(){
    std::lock_guard<std::mutex> lock(telemetryDataMutex);
    
    // Convert the updated telemetry data to a string to send
    std::string message_str = telemetryData.dump() + "\n";
    return message_str;

}

void handle_client(int client_socket, std::shared_ptr<Telemetry> telemetry) {
    subscribeTelemetry(telemetry);
}

void start_server(std::string connection_url, int port) {
    if (server_running) {
        throw std::runtime_error("Server is already running.");
    }

    Mavsdk::ComponentType componentType = Mavsdk::ComponentType::Autopilot;
    Mavsdk::Configuration config(componentType);

    Mavsdk mavsdk(config);

    ConnectionResult connection_result = mavsdk.add_any_connection(connection_url);
    
    auto prom = std::promise<std::shared_ptr<System>>();
    auto fut = prom.get_future();

    mavsdk.subscribe_on_new_system([&mavsdk, &prom]() {
        auto system = mavsdk.systems().at(0);
        if (system->has_autopilot()) {
            prom.set_value(system);
        }
    });

    if (fut.wait_for(seconds(5)) == std::future_status::timeout) {
        throw std::runtime_error("No drone found, exiting.");
    }

    auto system = fut.get();
    auto telemetry = std::make_shared<Telemetry>(system);

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        throw std::runtime_error("Failed to create socket.");
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        throw std::runtime_error("Failed to bind socket.");
    }

    if (listen(serverSocket, 5) == -1) {
        throw std::runtime_error("Failed to listen on socket.");
    }

    server_running = true;

    // Run the server in a separate thread
    server_thread = std::thread([serverSocket, telemetry]() {
        while (server_running) {
            int clientSocket = accept(serverSocket, NULL, NULL);
            if (clientSocket == -1) {
                std::cerr << "Failed to accept client." << std::endl;
                continue;
            }

            std::thread([clientSocket, telemetry]() {
                handle_client(clientSocket, telemetry);
            }).detach();
        }

        close(serverSocket);
    });
}

void stop_server() {
    server_running = false;
    if (server_thread.joinable()) {
        server_thread.join();
    }
}

// Binding code for Python
PYBIND11_MODULE(_telemetry_stream, m) {
    m.def("start_server", &start_server, "Start telemetry server",
          py::arg("connection_url"), py::arg("port"));

    m.def("stop_server", &stop_server, "Stop the telemetry server");

    m.def("get_data", &getData, "Retrieve JSON with Full Data");
}
