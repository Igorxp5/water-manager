import struct
import itertools

from protobuf.out.python.api_pb2 import Response
from test_protobuf.test_pb2 import _TestRequest, _TestResponse

PACKET_FORMAT = '<BH'
REQUEST_ID_ITERATOR = itertools.cycle(range(1, 65535))

def send_api_message(arduino, message):
    payload = struct.pack(PACKET_FORMAT, 1, len(message)) + message
    arduino.write(payload)

def send_test_message(arduino, message):
    payload = struct.pack(PACKET_FORMAT, 2, len(message)) + message
    arduino.write(payload)

def read_response(arduino):
    message_type = struct.unpack('B', arduino.read(1))[0]

    if message_type == 3:
        print('[DEBUG]', arduino.read_until())
        return read_response(arduino)  # Ignore DEBUG messages, wait for next message

    message_length = struct.unpack('<H', arduino.read(2))[0]
    raw = arduino.read(message_length)
    
    response = None
    if message_type == 1:
        response = Response()
        response.ParseFromString(raw)
    elif message_type == 2:
        response = _TestResponse()
        response.ParseFromString(raw)
    return response

def generate_request_id():
    global REQUEST_ID_ITERATOR
    return next(REQUEST_ID_ITERATOR)


def set_pin_value(arduino, pin, value):
    request = _TestRequest()
    request.id = generate_request_id()
    request.createIO.pin = pin

    payload = request.SerializeToString()
    send_test_message(arduino, payload)
    response = read_response(arduino)

    # The response is irrelevant (we may get an error due a pin already set)

    request = _TestRequest()
    request.id = generate_request_id()
    request.setIOValue.pin = pin
    request.setIOValue.value = value

    payload = request.SerializeToString()
    send_test_message(arduino, payload)
    response = read_response(arduino)

    assert isinstance(response, _TestResponse) and response.error == False


def get_pin_value(arduino, pin):
    request = _TestRequest()
    request.id = generate_request_id()
    request.getIOValue.pin = pin

    payload = request.SerializeToString()
    send_test_message(arduino, payload)
    response = read_response(arduino)

    assert isinstance(response, _TestResponse)
    assert response.error == False

    return response.message.intValue

def clear_pins(arduino):
    request = _TestRequest()
    request.id = generate_request_id()
    request.clearIOs.SetInParent()

    payload = request.SerializeToString()
    send_test_message(arduino, payload)
    response = read_response(arduino)

    assert isinstance(response, _TestResponse)
    assert response.error == False
