#include "dts.hpp"

#include <future>
#include <iostream>
#include <memory>
#include <cstdint>

#include <mavsdk.h>
#include <connection_result.h>
#include <plugins/telemetry/telemetry.h>

#include "dqueue.hpp"
#include "struct.hpp"

void DTStream::call_update(uint64_t& count, dqiter& iter) {

    // Increment the count:

    count++;

    // Determine if we need to allocate new list value:

    if (count > this->dq.count()) {
        this->dq.allocate();
    }

    // Grab a pointer to the old data:

    DValue& odata = *iter;

    // Increment the iterator:

    ++iter;

    // Release the semaphore:
    // (We are saying we are ready for the value to be removed)

    odata.latch.count_down();
}

DTData DTStream::get_data() {

    // Get and return the data from the queue:

    return dq.get_data();
}

// Initialize Drone Connection via UDP Port
bool DTStream::start() {
    // Connects to UDP
    std::cout << "Listening on " << connection_url << '\n';

    const mavsdk::ConnectionResult connection_result = mavsdk.add_any_connection(connection_url);

    if (connection_result != mavsdk::ConnectionResult::Success) {
        std::cerr << "Connection failed: " << connection_result << '\n';
        return false;
    }

    // Waits for connection
    std::cout << "Waiting for drone to connect..." << '\n';
    std::promise<std::shared_ptr<mavsdk::System>> prom;
    std::future<std::shared_ptr<mavsdk::System>> fut = prom.get_future();

    // Add new temporary callback that gets called upon system add:
    // (Callback implemented via lambda)

    const mavsdk::Mavsdk::NewSystemHandle handle = mavsdk.subscribe_on_new_system([this, &prom]() {
        auto systems = mavsdk.systems();
        std::cout << "Number of systems detected: " << systems.size() << '\n';

        if (!systems.empty()) {
            auto system = systems.at(0);
            if (system->has_autopilot()) {
                std::cout << "Drone discovered!" << '\n';

                prom.set_value(system);

            } else {
                std::cout << "Detected system does not have an autopilot."
                          << '\n';
                prom.set_value(nullptr);
            }
        } else {
            std::cout << "No systems found." << '\n';
            prom.set_value(nullptr);
        } 
    });

    // Wait for system to be configured:

    auto system = fut.get();
    if (!system) {
        std::cerr << "Failed to connect to the drone." << '\n';
        return false;
    }

    // Remove system callback:

    mavsdk.unsubscribe_on_new_system(handle);

    if (!system->is_connected()) {
        std::cerr << "System is not connected!" << '\n';
        return false;
    }

    // Initialize Telemetry
    telemetry = std::make_unique<mavsdk::Telemetry>(system);

    // Configure all callback functions

    telemetry->subscribe_position([this](mavsdk::Telemetry::Position position) {

        // Static variable for keeping count:

        static uint64_t count = 1;

        // Define the iterator to utilize:

        static auto iter = this->dq.begin();

        // Operate upon the data:

        iter->data.latitude = position.latitude_deg;
        iter->data.longitude = position.longitude_deg;
        iter->data.altitude = position.relative_altitude_m;

        // Alter the queue state:

        this->call_update(count, iter);
    });

    telemetry->subscribe_attitude_angular_velocity_body(
        [this](mavsdk::Telemetry::AngularVelocityBody angularVelocity) {

            // Static variable for keeping count:

            static uint64_t count = 1;

            // Define iterator to utilize:

            static auto iter = this->dq.begin();

            // Operate upon the data:

            iter->data.vroll = angularVelocity.roll_rad_s;
            iter->data.vpitch = angularVelocity.pitch_rad_s;
            iter->data.vyaw = angularVelocity.yaw_rad_s;

            // Alter queue state:

            this->call_update(count, iter);
        });

    telemetry->subscribe_velocity_ned(
        [this](mavsdk::Telemetry::VelocityNed velocity) {

            // Static variable for keeping count:

            static uint64_t count = 1;

            // Define iterator to utilize:

            static auto iter = this->dq.begin();

            // Operate upon the data:

            iter->data.vnorth = velocity.north_m_s;
            iter->data.veast = velocity.east_m_s;
            iter->data.vdown = velocity.down_m_s;

            // Alter queue state:

            this->call_update(count, iter);
        });

    telemetry->subscribe_fixedwing_metrics(
        [this](mavsdk::Telemetry::FixedwingMetrics metrics) {

            // Static variable for keeping count:

            static uint64_t count = 1;

            // Define iterator to utilize:

            static auto iter = this->dq.begin();

            // Operate upon the data:

            iter->data.airspeed = metrics.airspeed_m_s;
            iter->data.throttle_per = metrics.throttle_percentage;
            iter->data.climb_rate = metrics.climb_rate_m_s;

            // Alter queue state:

            this->call_update(count, iter);
        });

    telemetry->subscribe_imu([this](mavsdk::Telemetry::Imu imu) {

        // Static variable for keeping count:

        static uint64_t count = 1;

        // Define iterator to utilize:

        static auto iter = this->dq.begin();

        // Operate upon the data:

        iter->data.aforward = imu.acceleration_frd.forward_m_s2;
        iter->data.aright = imu.acceleration_frd.right_m_s2;
        iter->data.adown = imu.acceleration_frd.down_m_s2;

        iter->data.avforward = imu.angular_velocity_frd.forward_rad_s;
        iter->data.avright = imu.angular_velocity_frd.right_rad_s;
        iter->data.avdown = imu.angular_velocity_frd.down_rad_s;

        iter->data.gforward = imu.magnetic_field_frd.forward_gauss;
        iter->data.gright = imu.magnetic_field_frd.right_gauss;
        iter->data.gdown = imu.magnetic_field_frd.down_gauss;

        iter->data.temp = imu.temperature_degc;
        iter->data.time = imu.timestamp_us;

        // Alter queue state:

        this->call_update(count, iter);
    });

    telemetry->subscribe_attitude_euler(
        [this](mavsdk::Telemetry::EulerAngle euler_angle) {

            // Static variable for keeping count:

            static uint64_t count = 1;

            // Define iterator to utilize:

            static auto iter = this->dq.begin();

            // Operate upon the data:

            iter->data.roll = euler_angle.roll_deg;
            iter->data.pitch = euler_angle.pitch_deg;
            iter->data.yaw = euler_angle.yaw_deg;

            // TODO: This also provides a timestamp

            // Alter queue state:

            this->call_update(count, iter);
        });

    return true;
}

void DTStream::stop() {

    // Destroy the MAVSDK object:

    this->mavsdk.~Mavsdk();
}
