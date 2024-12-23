/**
 * @file bench.cpp
 * @author Owen Cochell (owencochell@gmail.com)
 * @brief Benchmark for telemetry stream
 * @version 0.1
 * @date 2024-10-07
 * 
 * @copyright Copyright (c) 2024
 * 
 * This file describes a benchmark for measuring the time it
 * takes to configure and retrieve data from the stream.
 */

#include "dts.hpp"

#include <chrono>
#include <iostream>
#include <ratio>

/// Number of values to retrieve
const int NUM = 500;

int main() {

    int count = 0;  // Number of values we retrieved
    double total = 0;  // Total time elapsed for getting
    double ctotal = 0;  // Total time elapsed for configuring
    double stotal = 0;  // Total time elapsed for stopping

    // Time the configure step:

    auto start = std::chrono::high_resolution_clock::now();

    DTStream dstream;

    dstream.set_drop_rate(0);

    // Start the stream:

    const bool stat = dstream.start();

    if (!stat) {
        // Failed to start

        return -1;
    }

    auto stop = std::chrono::high_resolution_clock::now();

    auto diff = stop - start;

    ctotal += std::chrono::duration<double, std::milli>(diff).count();

    // Iterate a number of times:

    for (int i = 0; i < NUM; ++i) {

        // Get a value and time it:

        auto start = std::chrono::high_resolution_clock::now();

        auto val = dstream.get_data();

        auto stop = std::chrono::high_resolution_clock::now();

        auto diff = stop - start;

        // Compute duration in milliseconds:

        const double ddiff = std::chrono::duration<double, std::milli>(diff).count();

        total += ddiff;

        // Output the results:

        std::cout << "[" << i << "]: " << ddiff << '\n';
        std::cout << val << '\n';

        ++count;
    }

    // Stop the stream, we are done:

    start = std::chrono::high_resolution_clock::now();

    dstream.stop();

    stop = std::chrono::high_resolution_clock::now();

    diff = stop - start;

    // Compute duration in milliseconds:

    stotal = std::chrono::duration<double, std::milli>(diff).count();

    // Compute the final values:

    const double average = total / count;

    std::cout << "+============================================+" << '\n';
    std::cout << "Configure time: " << ctotal << '\n';
    std::cout << "Stop time: " << stotal << '\n';
    std::cout << "Average Get Time: " << average << '\n';
    std::cout << "Iterations: " << count << '\n';

    return 0;
}
