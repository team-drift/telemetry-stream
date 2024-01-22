# Telemetry Stream
`telemetry-stream` is responsible for streaming telemetry data from the drone we are tracking to our [`kinematic-model`](https://team-drift.github.io/delphi/kinematic_model/) which will then perform the necessary calculations needed by [`PTU-SDK`](https://team-drift.github.io/delphi/PTU-SDK/) to move the gimbal controlling the laser. 

In this documentation, we outline how `telemetry-stream` works and how to use it. 
