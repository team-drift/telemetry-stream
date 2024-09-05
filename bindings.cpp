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

// Pybind11 namespace
namespace py = pybind11;
using namespace mavsdk;
using namespace std::chrono;
using json = nlohmann::json;

// Global telemetry data
json telemetryData = json::object();

void updateTelemetryData(const json &newData) {
    for (auto &[key, value] : newData.items()) {
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

    // Subscribe to other telemetry streams (attitude, velocity, etc.)
    telemetry->subscribe_attitude_euler([clientSocket](Telemetry::EulerAngle euler_angle) {
        updateTelemetryData({
            {"roll_deg", euler_angle.roll_deg},
            {"pitch_deg", euler_angle.pitch_deg},
            {"yaw_deg", euler_angle.yaw_deg},
            {"timestamp", euler_angle.timestamp_us},
        });

        if (telemetryData.contains("relative_altitude_m") &&
            telemetryData.contains("latitude_deg") &&
            telemetryData.contains("longitude_deg")) {

            std::string message_str = telemetryData.dump() + "\n";
            int length = message_str.length();

            ssize_t sent_length = send(clientSocket, &length, sizeof(length), 0);
            if (sent_length == -1) {
                perror("send length");
                return;
            }

            ssize_t sent_data = send(clientSocket, message_str.c_str(), length, 0);
            if (sent_data == -1) {
                perror("send message");
                return;
            }
        }
    });
}

void handle_client(int client_socket, std::shared_ptr<Telemetry> telemetry) {
    subscribeTelemetry(telemetry, client_socket);
}

void start_server(std::string connection_url, int port) {
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

    close(serverSocket);
}


// Binding code for Python
PYBIND11_MODULE(test, m) {
    m.def("start_server", &start_server, "Start telemetry server",
          py::arg("connection_url"), py::arg("port"));

    m.def("add", &add, "test");
}
