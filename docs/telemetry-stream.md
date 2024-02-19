# Telemetry Stream
`telemetry-stream` is responsible for streaming telemetry data from the drone we are tracking to our [`kinematic-model`](https://team-drift.github.io/delphi/kinematic_model/) which will then perform the necessary calculations needed by [`PTU-SDK`](https://team-drift.github.io/delphi/PTU-SDK/) to move the gimbal controlling the laser. 

In this documentation, we (will) outline what is inside `telemetry-stream`, how `telemetry-stream` works, and more importantly how to use it. 

## `main.cpp`
This is a C++ implementation for streaming telemetry data. We primarily utilize [`MAVSDK`](https://mavsdk.mavlink.io/main/en/), a higher level API ["that aims to be fully standards-compliant with MAVLink common microservices"](https://mavlink.io/en/about/implementations.html) such as telemetry, hence why we opted for it. There is no reason not to re-use already fast (C++) code that's already been written for this "basic" functionality (unless `MAVSDK` loses the wide community support it has as of today).

## `deprecated`
The `deprecated` folder contains a python implementation for telemetry streaming which heavily utilizes [`pymavlink`](https://github.com/ArduPilot/pymavlink). Though still functional, we opted to refactor in C++ as performance must be optimized.



## Developer Notes

### Prior Knowledge
Read the following pieces of documentation (in a *BFS* manner) before developing: 

0. [MAVLink](https://mavlink.io/en/): read the `Introduction` section and its subsections

1. [MAVSDK](https://mavsdk.mavlink.io/main/en/): read the `Introduction` and `C++` sections, but do not start the `Quickstart` section yet

2. [Telemetry](https://mavsdk.mavlink.io/main/en/cpp/guide/telemetry.html)

3. [Discrepancies](https://github.com/mavlink/MAVSDK/issues/2221): always watch out for discrepancies between the actual documentation and example code provided by a given tool. However, you should always reference both.

### Setup
To begin developing:

0. Ensure you have `CMake` and compiler such as `GCC`, `Clang`, or `MSVC` setup. 

1. Simply follow the [Quickstart](https://mavsdk.mavlink.io/main/en/cpp/quickstart.html) guide for your machine's OS. We may need to explore [building from source](https://mavsdk.mavlink.io/main/en/cpp/guide/build.html) for better performance.

    Once you reach the 'Setting up a Simulator' subsection, you may want to [set up the PX4 SITL developer environment](https://docs.px4.io/master/en/dev_setup/dev_env.html) rather than use a [pre-built docker container to run PX4 and the simulator](https://github.com/JonasVautherin/px4-gazebo-headless): 

    `docker run --rm -it jonasvautherin/px4-gazebo-headless:1.11.0`.

    @BumbleIV personally experienced issues attempting the latter instruction, so they dived into the former.

    If you do the same, you will find a toggle button dedicated to `Apple M1 Macbook users` instructing them to essentially make an x86 terminal. However, it is not possible to duplicate a terminal for OS versions Ventura and beyond. Fortunately, there is [a great workaround](https://stackoverflow.com/questions/74198234/duplication-of-terminal-in-macos-ventura). 

    Once you complete the workaround, simply continue with the rest of the guide.
    
Once all is complete, you should be set to start developing! [Examples](https://mavsdk.mavlink.io/main/en/cpp/examples/) may be a helpful resource to look at before developing.

# How to use
This section (will soon) outlines how to physically setup telemetry streaming from the drone to the ground station (us). 

## `deprecated`
You can use the `deprecrated/stream_data.py` as a quick validation that your `baud` and `connection_string` is correct. Simply plug in the aformentioned parameters into `stream_data.py` then run the file. 