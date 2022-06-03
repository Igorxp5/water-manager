import struct
import random
import asyncio
import logging

import pytest

from protobuf.out.python.api_pb2 import Request, Response

from .lib.api import APIClient
from .lib.api.exceptions import APIException

LOGGER = logging.getLogger(__name__)


async def test_can_answer_multiples_requests(api_client: APIClient):
    """
    Platform should be able to received multiples requests at same time and it can
    answer both.
    """
    requests = []

    for pin in range(2):
        request = api_client.create_request('createWaterSource', name=f'Water source {pin}', pin=pin)
        payload = api_client.build_request_wrapper(request)
        future = await api_client.send_payload(payload, request.id)
        requests.append(future)

    for i, response in enumerate(requests):
        try:
            await response
        except asyncio.CancelledError:
            assert False, f'Platform did not answer request {i + 1}'


async def test_error_decode_request(api_client: APIClient, reset_api_timeout):
    """Platform should return an Response error message when a invalid Request is provided"""
    payload = api_client.build_request_wrapper(b'Nothing')

    future = await api_client.send_payload(payload, next(api_client.REQUEST_ID_ITERATOR))

    response = await asyncio.wait_for(api_client.get_error_response(), timeout=7)

    assert response.id == 0
    assert response.exception_type is APIException
    assert response.message == 'Failed to decode the request'


async def test_handle_after_truncated_messages(api_client: APIClient):
    """
    Platform should be able to drop bytes when an incomplete message arrives.
    Platform should answer "Truncated message received"
    """
    request = api_client.create_request('createWaterSource', name=f'Compesa water source', pin=15)
    payload = api_client.build_request_wrapper(request)

    for trunc in range(2, 5):
        truncated_payload = payload[:trunc]

        future = await api_client.send_payload(truncated_payload, request.id)

        response = await asyncio.wait_for(api_client.get_error_response(), timeout=7)

        assert response.id == 0
        assert response.exception_type is APIException
        assert response.message == 'Truncated message received'

    water_sources = await api_client.get_water_source_list()

    assert not water_sources

@pytest.mark.xfail(reason='APIClient currently does not support large requests')
async def test_send_large_invalid_request(api_client: APIClient):
    """
    Platform should not crash when receiving a large request.
    Platform should respond with an error For invalid message type in the request.
    """
    # FIXME The platform is currently supporting larging requests, but the APIClient is not 
    # handling the responses well.
    raise NotImplementedError


async def test_keep_receiving_request_after_50_days(api_client: APIClient):
    """Platform should keep receiving requests after 50 days (long overflow)"""
    MAX_UINT32 = 4294967295

    current_time = await api_client.get_millis()

    offset = MAX_UINT32 - current_time

    await api_client.set_clock_offset(offset)

    assert await api_client.get_millis() < 60 * 1000

    water_source_name = 'Water source'

    await api_client.create_water_source(water_source_name, 10)

    water_source = await api_client.get_water_source(water_source_name)

    assert water_source['name'] == water_source_name 


async def test_reset_with_water_tank_and_source_dependency(api_client: APIClient):
    """Platform should be able to perform reset even when there are water tank/source dependencies"""
    water_tank_name_1, pressure_sensor_1, volume_factor_1, pressure_factor_1 = 'Bottom tank', 1, 1, 1
    water_source_name_1, water_source_pin_1 = 'Compesa water source', 15

    await api_client.create_water_source(water_source_name_1, water_source_pin_1)
    await api_client.create_water_tank(water_tank_name_1, pressure_sensor_1, volume_factor_1, pressure_factor_1, water_source_name_1)
    
    water_tank_name_2, pressure_sensor_2, volume_factor_2, pressure_factor_2 = 'Upper tank', 2, 1, 1
    water_source_name_2, water_source_pin_2 = 'Water pump', 16

    await api_client.create_water_source(water_source_name_2, water_source_pin_2, water_tank_name_1)
    await api_client.create_water_tank(water_tank_name_2, pressure_sensor_2, volume_factor_2, pressure_factor_2, water_source_name_2)

    await api_client.reset()

    assert await api_client.get_water_source_list() == []
    assert await api_client.get_water_tank_list() == []
