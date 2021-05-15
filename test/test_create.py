import time

from protobuf.out.python.api_pb2 import CreateWaterSource


def test_create_water_source(arduino):
    create_water_source = CreateWaterSource()
    create_water_source.name = 'Registro da Compesa'
    create_water_source.pin = 15
    
    arduino.write(b'' + create_water_source.SerializeToString())
    time.sleep(1)
    print(arduino.read(arduino.in_waiting))

# Turn off water sources when the program crashes
