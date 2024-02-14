# Telemetry Stream
`telemetry-stream` is responsible for streaming telemetry data from the drone we are tracking to our [`kinematic-model`](https://team-drift.github.io/delphi/kinematic_model/) which will then perform the necessary calculations needed by [`PTU-SDK`](https://team-drift.github.io/delphi/PTU-SDK/) to move the gimbal controlling the laser. 

In this documentation, we (will) outline how `telemetry-stream` works and more importantly how to use it. However, we highly recommend reading through the [mavlink documentation](https://mavlink.io/en/) first.

# cpp
This folder contains a C++ implementation for streaming telemetry data. We primarily utilize [`MAVSDK`](https://mavsdk.mavlink.io/main/en/), a higher level API ["that aims to be fully standards-compliant with MAVLink common microservices"](https://mavlink.io/en/about/implementations.html) such as telemetry, hence why we opted for it. There is no reason not to re-use already fast (C++) code that's already been written for this "basic" functionality (unless `MAVSDK` loses the wide community support and community it has as of today).

## Developer Notes
Read the following documentation before developing: [Telemetry](https://mavsdk.mavlink.io/main/en/cpp/guide/telemetry.html). 

# python
The `python` folder is simply a deprecated `python` implementation. Though still functional, we opted to refactor in C++ as performance must be optimized.

# How to use
This section (will soon) outlines how to setup telemetry streaming. 