#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <iostream>
#include <future>
#include <chrono>
#include <csignal>

using namespace mavsdk;
using namespace std::this_thread;
using namespace std::chrono;

int main(int argc, char** argv) {
    Mavsdk mavsdk;

    // Setup connection
    const std::string connection_url = "serial:///dev/cu.usbserial-D30F0LHK:57600";
    ConnectionResult connection_result = mavsdk.add_any_connection(connection_url);

    if (connection_result != ConnectionResult::Success) {
        std::cerr << "Connection failed: " << connection_result << std::endl;
        return -1;
    }

    // Wait for drone to connect
    std::cout << "Waiting for drone to connect..." << std::endl;
    auto prom = std::promise<std::shared_ptr<System>>{};
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

    // Setup telemetry
    auto system = fut.get();
    auto telemetry = Telemetry{system};

    telemetry.subscribe_position([](Telemetry::Position position) {
        std::cout << "Altitude: " << position.relative_altitude_m << " m, "
                  << "Latitude: " << position.latitude_deg << ", "
                  << "Longitude: " << position.longitude_deg << std::endl;
    });

    telemetry.subscribe_battery([](Telemetry::Battery battery) {
        std::cout << "Battery: " << battery.remaining_percent * 100 << "%" << std::endl;
    });

    // Capture Ctrl+C for shutdown
    std::signal(SIGINT, [](int) {
        std::cout << "Shutting down..." << std::endl;
        std::exit(0);
    });

    // Hold the program to stream data
    while (true) {
        sleep_for(seconds(1));
    }

    return 0;
}
