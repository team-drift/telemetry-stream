
## Telemetry Stream
A guide on how to recieve data within Python from QGroundControl

### QGroundControl
QGroundControl is a Ground Control Station (GCS) made specifically for the MavLink Protocol
    * Mavlink is a messaging protocol for communicating with Drones

The GCS works directly with ArduPilot and PX4, which are two popular Open Source Flight Control Software systems for Unmanned Vehicles
    * Within DRIFT, we will be using ArduPilot for our physical drone and JMAVSim from PX4 for local testing

### telemetry_stream setup within Python
Currently building/testing

### Utilizing telemetry_stream within Python
connect_drone(connection_url)
    Configures MAVSDK to expect an AutoPilot component

    connection_url: A string meant to establish the UDP port for MAVSDK to listen for data
        14540 is standard PX4 UDP port to connect with offboardAPIs
        14550 is standard to connect with QGroundControl
   
    subscribeToData: Optional boolean argument to allow for data to be subscribed to
        Set to True by default

get_data()
    Returns string with JSON structure with data that was previously subscribed too


#### Current State of program
The current state of the program supports a connection with a drone (simulation) within a local computer, when physical hardware is setup, might need to be refactored
    
    

