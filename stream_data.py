from pymavlink import mavutil
import datetime
import json

# Connect to the vehicle
baud = 57600
mav = mavutil.mavlink_connection(
    "/dev/cu.usbserial-D30F0LHK",
    baud=baud,
)  # Adjust the connection string as needed
print(f"baud is {baud}")
print("Waiting for heartbeat")
mav.wait_heartbeat()

messages = []

print("Heartbeat received")
for i in range(1000):
    # Wait for a new message
    msg = mav.recv_match(type=["GLOBAL_POSITION_INT", "ATTITUDE"], blocking=True) # see docs on blocking=True
    messages.append(msg.to_dict())
    print(i)

dt_string = datetime.datetime.now().strftime("%d%m%Y-%H:%M:%S")
json_data = json.dumps(messages)
with open(f"data_{dt_string}.json", "w") as f:
    f.write(json_data)

# possible baud rates: 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 500000, 921600
#   For Atlas's telemetry radio, 57600 is the baud rate

# to find the connection string on mac:
#   ls /dev/*usb*
# possbile connection strign prefixes: cu., tty.

# on mac for Atlas radio:
# /dev/cu.usbserial-D30BQ4ZZ

# on mac for Atlas 2 radio:
# /dev/cu.usbserial-D30F0LHK