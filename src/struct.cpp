/**
 * @file struct.cpp
 * @author Owen Cochell (owencochell@gmail.com)
 * @brief Implementations for structures
 * @version 0.1
 * @date 2024-10-20
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "struct.hpp"

#include <iostream>

json DTData::to_json() const {

    // Define the JSON data to return:

    json data;

    // Add the values from each section:

    data["time"] = this->time;
    data["latitude"] = this->latitude;
    data["longitude"] = this->longitude;
    data["altitude"] = this->altitude;

    data["vnorth"] = this->vnorth;
    data["veast"] = this->veast;
    data["vdown"] = this->vdown;
    data["airspeed"] = this->airspeed;
    data["climb_rate"] = this->climb_rate;

    data["aforward"] = this->aforward;
    data["aright"] = this->aright;
    data["adown"] = this->adown;

    data["avforward"] = this->avforward;
    data["avright"] = this->avright;
    data["avdown"] = this->avdown;

    data["gforward"] = this->gforward;
    data["gright"] = this->gright;
    data["gdown"] = this->gdown;

    data["roll"] = this->roll;
    data["pitch"] = this->pitch;
    data["yaw"] = this->yaw;

    data["vroll"] = this->vroll;
    data["vpitch"] = this->vpitch;
    data["vyaw"] = this->vyaw;

    data["temp"] = this->temp;
    data["throttle_per"] = this->throttle_per;

    // Return the JSON data:

    return data;
}

std::ostream& operator<<(std::ostream& output, DTData const& dt) {

    // Print the data in this class:

    output << "Time (Micro Seconds): " << dt.time << "\n";
    output << "Latitude: " << dt.latitude << "\n";
    output << "Longitude: " << dt.longitude << "\n";
    output << "Relative Altitude (meters): " << dt.altitude << "\n";

    output << "Velocity North (meters per second): " << dt.vnorth << "\n";
    output << "Velocity East (meters per second): " << dt.veast << "\n";
    output << "Velocity Down (meters per second): " << dt.vdown << "\n";
    output << "Airspeed (meters per second): " << dt.airspeed << "\n";
    output << "Climb Rate (meters per second): " << dt.climb_rate << "\n";

    output << "Forward Acceleration (meters per second^2): " << dt.aforward
           << "\n";
    output << "Right Acceleration (meters per second^2): " << dt.aright << "\n";
    output << "Down Acceleration (meters per second^2): " << dt.adown << "\n";

    output << "Angular Velocity Forward (radians per second): " << dt.avforward
           << "\n";
    output << "Angular Velocity Right (radians per second): " << dt.avright
           << "\n";
    output << "Angular Velocity Down (radians per second): " << dt.avdown
           << "\n";

    output << "Magnetic Forward (Gauss): " << dt.gforward << "\n";
    output << "Magnetic Right (Gauss): " << dt.gright << "\n";
    output << "Magnetic Down (Gauss): " << dt.gdown << "\n";

    output << "Roll (degrees): " << dt.roll << "\n";
    output << "Pitch (degrees): " << dt.pitch << "\n";
    output << "Yaw (degrees): " << dt.yaw << "\n";

    output << "Roll Velocity (radians per second): " << dt.vroll << "\n";
    output << "Pitch Velocity (radians per second): " << dt.vpitch << "\n";
    output << "Yaw Velocity (radians per second): " << dt.vyaw << "\n";

    output << "Temperature (celsius): " << dt.temp << "\n";
    output << "Throttle Percentage: " << dt.throttle_per << "\n";

    return output;
}
