import struct
import itertools

from protobuf.out.python.api_pb2 import Response

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
    message_length = struct.unpack('<H', arduino.read(2))[0]
    raw = arduino.read(message_length)
    
    response = None
    if message_type == 1:
        response = Response()
        response.ParseFromString(raw)
    return response

def generate_request_id():
    global REQUEST_ID_ITERATOR
    return next(REQUEST_ID_ITERATOR)
