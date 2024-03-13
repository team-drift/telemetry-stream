# Telemetry Stream
`telemetry-stream` is responsible for streaming telemetry data from the drone we are tracking to our [`kinematic-model`](https://team-drift.github.io/delphi/kinematic_model/) which will then perform the necessary calculations needed by [`PTU-SDK`](https://team-drift.github.io/delphi/PTU-SDK/) to move the gimbal controlling the laser. 

In this documentation, we (will) outline what is inside `telemetry-stream`, how `telemetry-stream` works, and more importantly how to use it. 


## `main.cpp`
*As of 02/19/2024, `main.cpp` is NOT functional.* 

This is a C++ implementation for streaming telemetry data. We primarily utilize [`MAVSDK`](https://mavsdk.mavlink.io/main/en/), a higher level API ["that aims to be fully standards-compliant with MAVLink common microservices"](https://mavlink.io/en/about/implementations.html) such as telemetry, hence why we opted for it. There is no reason not to re-use already fast (C++) code that's already been written for this "basic" functionality (unless `MAVSDK` loses the wide community support it has as of today).


## `deprecated`
The `deprecated` folder contains a python implementation for telemetry streaming which heavily utilizes [`pymavlink`](https://github.com/ArduPilot/pymavlink). Though still functional, we opted to refactor in C++ as performance must be optimized.


## Prior Knowledge
Read the following pieces of documentation (in a *BFS* manner) before developing: 

0. [MAVLink](https://mavlink.io/en/): read the `Introduction` section and its subsections

1. [MAVSDK](https://mavsdk.mavlink.io/main/en/): read the `Introduction` and `C++` sections, but do not start the `Quickstart` section yet

2. [Telemetry](https://mavsdk.mavlink.io/main/en/cpp/guide/telemetry.html)

3. [Discrepancies](https://github.com/mavlink/MAVSDK/issues/2221): always watch out for discrepancies between the actual documentation and example code provided by a given tool. However, you should always reference both.


## Setup
To begin developing, read the notes below as you follow the whole [C++ Quickstart](https://mavsdk.mavlink.io/main/en/cpp/quickstart.html?q=) for your machine's OS.


After the [Install MAVSDK library](https://mavsdk.mavlink.io/main/en/cpp/quickstart.html#install-mavsdk-library) section, check its version. For example, if you're on macOS, then `brew info mavsdk`. You may run into a versioning discrepancy issue between the `mavsdk` library installed during the Install MAVSDK Library section and the `mavsdk` repository in the [Build and Try Example](https://mavsdk.mavlink.io/main/en/cpp/quickstart.html#build_examples) section. 

If the versions between the library and the repository differ, simply pull the whole version of the repo that matches the library, or alternatively  copy and paste the `takeoff_and_land` example code from the version of the repo that matches the library. To switch to the appropiate version of the MAVSDK repo: 
- go to the [MAVSDK repository](https://github.com/mavlink/MAVSDK) 
- Click on "main" -> "Tags"
- Select the appropriate version


Once you reach the `Setting up a Simulator` subsection, you may want to [set up the PX4 SITL developer environment](https://docs.px4.io/master/en/dev_setup/dev_env.html) rather than use a [pre-built docker container to run PX4 and the simulator](https://github.com/JonasVautherin/px4-gazebo-headless).

If you do the latter, you will find a toggle button dedicated to "Apple M1 Macbook users" instructing them to essentially make an x86 terminal. However, it is not possible to duplicate a terminal for OS versions Ventura and beyond. Fortunately, there is [a great workaround](https://stackoverflow.com/questions/74198234/duplication-of-terminal-in-macos-ventura). 


## How to use
This section (will soon) outlines how to physically setup telemetry streaming from the drone to the ground station (us).

## Installations 
Run the command: 

brew install nlohmann-json 

You can use the `deprecrated/stream_data.py` as a quick validation that your `baud` and `connection_string` is correct. Simply plug in the aformentioned parameters into `stream_data.py` then run the file. 

Otherwise, you can follow the steps below to setup a simulated drone rather than a physical drone to verify telemetry data streaming is working.
### Test Simulation
1. Download [QGroundControl](http://qgroundcontrol.com/downloads/) and simply open it.

2. Download PX-4 and build the simulation.
    
    Open a terminal. Run the following:
    ```
    git clone https://github.com/PX4/PX4-Autopilot.git --recursive
    cd PX4-Autopilot
    make px4_sitl jmavsim
    ```

3. Ensure Proper Configurations 
    
    Ensure QGroundControl ports and \<insert testing file such as `takeoff_and_land.cpp`\> ports are set to `14540` (Or, whatever port we want. Just have it be uniform.)

    Go to the top left of QGroundCountrol -> Application Settings -> MAVLink, and check if the hostname is `localhost:14445`


4. Start up the simulation 
    
    Simply follow the rest of the instructions outlined [here](https://mavsdk.mavlink.io/main/en/cpp/quickstart.html#build_examples).




Run the telemetry stream 

1. start up px4 jmav
2. in this repo run the command:
    ./telemetry-stream 

you can run 
    commander takeoff
    commander land

this run simulation.py in agogos