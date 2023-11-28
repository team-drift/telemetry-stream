from pymavlink import mavutil
import datetime
import json

# Connect to the vehicle
baud = 57600
mav = mavutil.mavlink_connection(
    "/dev/cu.usbmodem21201",
    baud=baud,
)  # Adjust the connection string as needed
print(f"baud is {baud}")
print("Waiting for heartbeat")
mav.wait_heartbeat()

# Request data streams
data_streams = ["ALL"]#, "GLOBAL_POSITION_INT", "ATTITUDE"]
stream_rate = 200  # Adjust the stream rate as needed (messages per second)

for data_stream in data_streams:
    mav.mav.request_data_stream_send(
        mav.target_system, mav.target_component,
        mavutil.mavlink.MAV_DATA_STREAM_ALL if data_stream == 'ALL' else getattr(mavutil.mavlink, f'MAV_DATA_STREAM_{data_stream}'),
        stream_rate,
        1  # 1 to start sending, 0 to stop
    )

messages = []
data_streams = ["GLOBAL_POSITION_INT", "ATTITUDE"]
print("Data streams requested")
try:
    for i in range(3000):
        # Wait for a new message
        msg = mav.recv_match(type=data_streams, blocking=True)  # see docs on blocking=True
        messages.append(msg.to_dict())
        print(i)
except KeyboardInterrupt:
    pass
finally:
    dt_string = datetime.datetime.now().strftime("%d%m%Y-%H:%M:%S")
    json_data = json.dumps(messages)
    with open(f"data_{dt_string}.json", "w") as f:
        f.write(json_data)
    mav.close()

# possible baud rates: 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 500000, 921600
#   For Atlas's telemetry radio, 57600 is the baud rate

# to find the connection string on mac:
#   ls /dev/*usb*
# possbile connection strign prefixes: cu., tty.

# on mac for Atlas radio:
# /dev/cu.usbserial-D30BQ4ZZ

# on mac for Atlas 2 radio:
# /dev/cu.usbserial-D30F0LHK