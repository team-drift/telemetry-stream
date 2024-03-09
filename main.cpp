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


void handle_client(int client_socket, std::shared_ptr<Telemetry> telemetry) {
    /*
    Handles data transmission to Client(Agogos Pipeline)

    Data is transmitted as a string
    */

    telemetry->subscribe_position([client_socket](Telemetry::Position position) {
        /*
        Takes in position to use in socket communcation
        */

        //Construct Message
        std::string message = std::to_string(position.relative_altitude_m) + ", " + std::to_string(position.latitude_deg) + ", " + std::to_string(position.longitude_deg) + "\n";
 
        //Sending Length Data
        int length = message.length();
        if (send(client_socket, &length, sizeof(length), 0) < 0) {
            perror("send length");
            return;
        }

        //Sending Message
        if (send(client_socket, message.c_str(), length, 0) < 0) {
            perror("send message");
            return;
        }
    });

    std::this_thread::sleep_for(seconds(10));
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
            close(clientSocket);
        }).detach();
    }

    // Cleanup
    close(serverSocket);
    return 0;
}