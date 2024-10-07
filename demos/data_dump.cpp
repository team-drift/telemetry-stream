/**
 * @file data_dump.cpp
 * @author Owen Cochell (owencochell@gmail.com)
 * @brief Dumps incoming telemetry data to a JSON file
 * @version 0.1
 * @date 2024-10-03
 * 
 * @copyright Copyright (c) 2024
 * 
 * This file dumps any and all flight data to a JSON file for later analysis.
 * The path to this file is hardcoded within this program,
 * so if one wishes to save to another location they MUST change and recompile.
 * 
 * TODO:
 * 
 * Add a program CLI!
 * Would be nice to define output file.
 * It would also be nice to define what kind of data we want.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <atomic>
#include <csignal>

#include "dts.hpp"

/// Define path to output data
const std::string PATH = "out.json";

/// Boolean determining if we are running
std::atomic<bool> running(true);

void signal_callback_handler(int signum) {
    std::cout << "Caught signal " << signum << '\n';
    running = false;
}

int main() {

    // Configure signal handler:

    signal(SIGINT, signal_callback_handler);

    // Create stream instance:

    DTStream dstream;

    // Start the stream:

    auto stat = dstream.start();

    // Do something if we fail:

    if (!stat) {
        return -1;
    }

    // Open file for reading:

    std::ofstream ofile(PATH);

    // Iterate until completion:

    while (running) {

        // Output data to file:

        ofile << dstream.get_data();
    }

    // Close the fstream:

    ofile.close();

    return 0;
}
