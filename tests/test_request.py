import pytest
import asyncio

from protobuf.out.python.api_pb2 import Request, Response

from .lib.api import APIException


async def test_can_answer_multiples_requests(api_client):
    """
    Platform should be able to received multiples requests at same time and it can
    answer both.
    """
    requests = []

    for pin in range(2):
        request = api_client.create_request('createWaterSource', name=f'Registro {pin}', pin=pin)
        payload = api_client.build_request_wrapper(request)
        future = await api_client.send_payload(payload, request.id)
        requests.append(future)

    for i, response in enumerate(requests):
        try:
            await response
        except asyncio.CancelledError:
            assert False, f'Platform did not answer request {i + 1}'


async def test_error_decode_request(api_client):
    """Platform should return an Response error message when a invalid Request is provided"""
    payload = api_client.build_request_wrapper(b'Nothing')

    future = await api_client.send_payload(payload, next(api_client.REQUEST_ID_ITERATOR))

    response = await asyncio.wait_for(api_client.get_error_response(), timeout=7)

    assert response.id == 0
    assert response.error is APIException
    assert response.message == 'Failed to decode the request'


async def test_handle_after_truncated_messages(api_client):
    """
    Platform should be able to drop bytes when an incomplete message arrives.
    Platform should answer "Truncated message received"
    """
    request = api_client.create_request('createWaterSource', name=f'Registro da Compesa', pin=15)
    payload = api_client.build_request_wrapper(request)

    for trunc in range(2, 5):
        truncated_payload = payload[:trunc]

        future = await api_client.send_payload(truncated_payload, request.id)

        response = await asyncio.wait_for(api_client.get_error_response(), timeout=7)

        assert response.id == 0
        assert response.error is APIException
        assert response.message == 'Truncated message received'

    assert not await api_client.get_water_source_list()


# TODO: Program should not crash due a large request
