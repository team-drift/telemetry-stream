# Controlling our Simulation
With software testing mainly consisting of simulations, here is a guide on how to move a vehicle in PX4 simulated environments

### Via Code
In python, install the MavSDK package
`pip install mavsdk`

Sample Code
```
import asyncio
from mavsdk import System

async def run():
    # Create a drone instance
    drone = System()
    await drone.connect(system_address="udp://:14540")

    async for state in drone.telemetry.health():
        if state.is_global_position_ok:
            print("Drone discovered!")
            break

    # Arm the drone
    print("Arming drone...")
    await drone.action.arm()

    # Take off to 10 meters
    print("Taking off...")
    await drone.action.takeoff()
    await asyncio.sleep(5)

    await asyncio.sleep(10)

    # Land the drone
    print("Landing...")
    await drone.action.land()

if __name__ == "__main__":
    # Create a new event loop and run the async function
    asyncio.run(run())

```

The above will send commands to PX4's Gazebo, which will allow us to have more control over the drone's movement

### Via QGroundControl
Once connected to QGroundControl (most likely port 14450) and armed, you have the ability to set the location of the drone and orbit around a specific area.

Simply click on the map and choose the command you want


###Via PX4 in Terminal
While this provides the least flexibility, this can be useful for quick tests

`commander takeoff` to takeoff
`commander land` to land
`commander set_position_local <x> <y> <z>` for a specific position

### Via GUI
While requires more steps to setup, there are various ways to have a GUI come with PX4 and allows you to control a drone with your keyboard




 
