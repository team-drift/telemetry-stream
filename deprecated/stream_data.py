from pymavlink import mavutil
import datetime
import json
import os
import time

# https://github.com/ArduPilot/pymavlink/blob/master/tools/mavtelemetry_datarates.py
# ESTIMATOR_STATUS -> pos_horiz_accuracy

def connect(connection_string, baud=14445):
    # Connect to the vehicle
    mav = mavutil.mavlink_connection(
        connection_string,
        baud=baud,
    )
    print("Waiting for heartbeat")
    mav.wait_heartbeat()
    return mav

def request_data_stream(mav, stream_id=mavutil.mavlink.MAV_DATA_STREAM_ALL, stream_rate=200):
    mav.mav.request_data_stream_send(
        mav.target_system,
        mav.target_component,
        stream_id,
        stream_rate,
        1,  # 1 to start sending, 0 to stop
    )

def record_data(mav):
    unique_mavpackettypes = set()
    messages = []
    try:
        i = 0
        start_time = time.time()
        while True:
            # Wait for a new message
            msg = mav.recv_match(blocking=True).to_dict()
            messages.append(msg)
            unique_mavpackettypes.add(msg.get('mavpackettype'))
            i += 1
            if i % 100 == 0:
                end_time = time.time()
                elapsed_time = end_time - start_time
                average_hz = 100 / elapsed_time if elapsed_time > 0 else 0
                print(f"{i} messages received, Average Hz: {average_hz:.2f}, Number of unique mavpackettypes: {len(unique_mavpackettypes)}")
                start_time = time.time()
    except Exception as e:
        print(f"Error: {e}")
    finally:
        dt_string = datetime.datetime.now().strftime("%Y_%m_%d_%H_%M_%S")
        json_data = json.dumps(messages)
        if not os.path.exists("./data/"):
            os.makedirs("./data/")
        with open(f"./data/{dt_string}.json", "w") as f:
            f.write(json_data)
        mav.close()

if __name__ == "__main__":
    connection_string = "udp:localhost:14445"
    mav = connect(connection_string)
    request_data_stream(mav, stream_rate=3)
    request_data_stream(mav, stream_id=mavutil.mavlink.MAV_DATA_STREAM_POSITION, stream_rate=50) # position
    request_data_stream(mav, stream_id=mavutil.mavlink.MAV_DATA_STREAM_EXTRA1, stream_rate=50) # attitude

    record_data(mav)
    # lower the stream rate
    request_data_stream(mav, stream_rate=3)
