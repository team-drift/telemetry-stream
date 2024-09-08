import _telemetry_stream
import time

result = _telemetry_stream.start_server("udp://0.0.0.0:14540", 12345)


while(True):
    print(_telemetry_stream.get_data())
    
    time.sleep(1)