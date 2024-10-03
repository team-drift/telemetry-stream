import asyncio
from mavsdk import System

async def run():
    # Create a drone instance
    drone = System()
    
    # Connect to the drone
    await drone.connect(system_address="udp://:14540")

    print("Waiting for drone to connect...")

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
