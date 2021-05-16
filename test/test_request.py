import time
import struct
import utils

from protobuf.out.python.api_pb2 import Request, Response

def test_can_answer_multiples_requests(arduino):
    """
    Platform should be able to received multiples requests at same time and i can
    answer both.
    """
    requests = []

    for _ in range(2):
        request = Request()
        request.id = utils.generate_request_id()
        request.createWaterSource.name = 'Registro da Compesa'
        request.createWaterSource.pin = 15
        requests.append(request)

        payload = request.SerializeToString()
        utils.send_api_message(arduino, payload)
    
    for i, request in enumerate(requests):
        response = utils.read_response(arduino)
        assert isinstance(response, Response), f'Platform did not answer request {i + 1}'
        assert response.id == request.id


def test_error_decode_request(arduino):
    """
    Platform should return an Response error message when a invalid Request is provided
    """
    payload = b'Nothing'

    utils.send_api_message(arduino, payload)
    
    response = utils.read_response(arduino)

    assert isinstance(response, Response)
    assert response.id == 0
    assert response.error == True
    assert response.message.stringValue == 'Failed to decode the request'


def test_handle_after_truncated_messages(arduino):
    """
    Platform should be able to drop bytes when an incomplete message arrives.
    Platform should answer "Truncated message received"
    """
    request = Request()
    request.id = utils.generate_request_id()
    request.createWaterSource.name = 'Registro da Compesa'
    request.createWaterSource.pin = 15

    payload = request.SerializeToString()

    for trunc in range(2, 5):
        truncated_payload = struct.pack(utils.PACKET_FORMAT, 1, len(payload)) + payload
        truncated_payload = truncated_payload[:trunc]
        print(f'Testing truncated data: {repr(truncated_payload)}...')
        
        arduino.write(truncated_payload)

        response = utils.read_response(arduino)
        assert isinstance(response, Response)
        assert response.id == 0
        assert response.error == True
        assert response.message.stringValue == 'Truncated message received'

    # Sending valid message
    utils.send_api_message(arduino, payload)

    response = utils.read_response(arduino)

    assert isinstance(response, Response)
    assert response.id == request.id
