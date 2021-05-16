import time
import struct
import utils

from protobuf.out.python.api_pb2 import Response

def test_error_decode_request(arduino):
    payload = b'Nothing'
    utils.send_api_message(arduino, payload)
    
    message_type = struct.unpack('B', arduino.read(1))[0]
    assert message_type == 1

    message_length = struct.unpack('<H', arduino.read(2))[0]
    raw = arduino.read(message_length)
    
    response = Response()
    response.ParseFromString(raw)

    assert response.id == 0
    assert response.error == True
    assert response.message.stringValue == 'Failed to decode the request'
