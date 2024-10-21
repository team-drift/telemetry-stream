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
#include <algorithm>

/// Number of values to retrieve
const int NUM = 500;

int main() {

    int count = 0;  // Number of values we retrieved
    double total = 0;  // Total time elapsed for getting
    double ctotal = 0;  // Total time elapsed for configuring
    double stotal = 0;  // Total time elapsed for stopping

    ///
    // Average Times
    ///

    double position_time = 0;
    double vaxis_time = 0;
    double airspeed_time = 0;
    double aaxis_time = 0;
    double vangular_time = 0;
    double mag_time = 0;
    double euler_time = 0;
    double veuler_time = 0;

    // Time the configure step:

    auto start = std::chrono::high_resolution_clock::now();

    DTStream dstream;

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

        // Get the smallest time value:

        auto min = std::min({val.position_time, val.vaxis_time, val.airspeed_time, val.aaxis_time, val.vangular_time, val.mag_time, val.euler_time, val.veuler_time});

        // Compute differences between each value:

        auto post = std::chrono::duration<double, std::milli>(val.position_time - min).count();
        auto vax = std::chrono::duration<double, std::milli>(val.vaxis_time - min).count();
        auto as = std::chrono::duration<double, std::milli>(val.airspeed_time - min).count();
        auto aa = std::chrono::duration<double, std::milli>(val.aaxis_time - min).count();
        auto vaa = std::chrono::duration<double, std::milli>(val.vangular_time - min).count();
        auto mag = std::chrono::duration<double, std::milli>(val.mag_time - min).count();
        auto euler = std::chrono::duration<double, std::milli>(val.euler_time - min).count();
        auto veuler = std::chrono::duration<double, std::milli>(val.veuler_time - min).count();

        // Add to average times:

        position_time += post;
        vaxis_time += vax;
        airspeed_time += as;
        aaxis_time += aa;
        vangular_time += vaa;
        mag_time += mag;
        euler_time += euler;
        veuler_time += veuler;

        // Output the time values:

        std::cout << "+===================================================+"
                  << "\n";
        std::cout << " --== [Time Stats: ] ==--" << "\n";

        std::cout << "Position: " << post << "\n";
        std::cout << "Axis Velocity: " << vax << "\n";
        std::cout << "Airspeed: " << as << "\n";
        std::cout << "Axis Acceleration: " << aa << "\n";
        std::cout << "Angular Acceleration: " << vaa << "\n";
        std::cout << "Magnetic: " << mag << "\n";
        std::cout << "Euler: " << euler << "\n";
        std::cout << "Euler Velocity: " << veuler << "\n";

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

    std::cout << "+============================================+" << '\n';
    std::cout << "--== [ Time Stats: ] ==--" << "\n";
    std::cout << "(Units are in milliseconds, 1000 milliseconds per second)"
              << "\n";
    std::cout << "Position: " << position_time / count << "\n";
    std::cout << "Axis Velocity: " << vaxis_time / count << "\n";
    std::cout << "Airspeed: " << airspeed_time / count << "\n";
    std::cout << "Axis Acceleration: " << aaxis_time / count << "\n";
    std::cout << "Angular Acceleration: " << vangular_time / count << "\n";
    std::cout << "Magnetic: " << mag_time / count << "\n";
    std::cout << "Euler: " << euler_time / count << "\n";
    std::cout << "Euler Velocity: " << veuler_time / count << "\n";

    return 0;
}
