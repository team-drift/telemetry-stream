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

int main(){
    signal(SIGINT, signal_callback_handler);

    if (connect_drone() != 0) {
        return -1;
    }

    std::cout << "Getting Ready To Run..." << std::endl;
    while(running){
        std::cout << get_data() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 1;
}
