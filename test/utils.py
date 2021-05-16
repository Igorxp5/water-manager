import struct
import itertools

PACKET_FORMAT = '<BH'
REQUEST_ID_ITERATOR = itertools.cycle(range(1, 65535))

def send_api_message(arduino, message):
    payload = struct.pack(PACKET_FORMAT, 1, len(message)) + message
    arduino.write(payload)

def send_test_message(arduino, message):
    payload = struct.pack(PACKET_FORMAT, 2, len(message)) + message
    arduino.write(payload)

def random_request_id():
    global REQUEST_ID_ITERATOR
    return next(REQUEST_ID_ITERATOR)
