import time

import utils

from protobuf.out.python.api_pb2 import Request, Response


def test_create_water_source(arduino):
    request = Request()
    request.id = utils.generate_request_id()
    request.createWaterSource.name = 'Registro da Compesa'
    request.createWaterSource.pin = 15

    payload = request.SerializeToString()

    utils.send_api_message(arduino, payload)

    response = utils.read_response(arduino)

    assert isinstance(response, Response)
    assert response.error == False
    assert response.id == request.id

    request = Request()
    request.id = utils.generate_request_id()
    request.getWaterSourceList.SetInParent()

    payload = request.SerializeToString()

    utils.send_api_message(arduino, payload)
    response = utils.read_response(arduino)

    assert isinstance(response, Response)
    assert response.error == False
    assert response.id == request.id
    print(type(response.message.listValue))
    print(repr(response.message.listValue))
    print(response.message.listValue)
    assert response.message.listValue == ['Registro da Compesa']

# Turn off water sources when the program crashes

# Test memory deallocation when removing a water source