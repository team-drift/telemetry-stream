/**
 * @file stream_demo.cpp
 * @author Owen Cochell (owencochell@gmail.com)
 * @brief A simple demo of telemetry stream functionality
 * @version 0.1
 * @date 2024-09-16
 * 
 * @copyright Copyright (c) 2024
 * 
 * This demo provides a simple showcase of how the components here are utilized.
 * We simply create a connection and read from it.
 */

#include <csignal>
#include <chrono>
#include <iostream>
#include <atomic>
#include <thread>

#include "dts.hpp"

/// Boolean determining if we are running
std::atomic<bool> running(true);

void signal_callback_handler(int signum) {
    std::cout << "Caught signal " << signum << '\n';
    running = false;
}

int main() {
    signal(SIGINT, signal_callback_handler);

    // Create stream instance:

    DTStream dstream;

    // Start the stream:

    bool const stat = dstream.start();

    if (!stat) {

        // Stream failed to start!

        return -1;
    }

    // Preform while loop until completetion

    std::cout << "Getting Ready To Run..." << '\n';
    while(running) {
        std::cout << dstream.get_data().to_json().dump() << '\n';
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
