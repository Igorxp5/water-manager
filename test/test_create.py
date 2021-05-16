import time

import utils

from protobuf.out.python.api_pb2 import Request


def test_create_water_source(arduino):
    request = Request()
    request.id = utils.generate_request_id()
    request.createWaterSource.name = 'Registro da Compesa'
    request.createWaterSource.pin = 15

    payload = request.SerializeToString()

    print(f'Sending request ({request.id})...')
    print(f'Request payload: {repr(payload)}')
    utils.send_api_message(arduino, payload)
    time.sleep(2)
    print(arduino.read(arduino.in_waiting))

    #TODO

# Turn off water sources when the program crashes
