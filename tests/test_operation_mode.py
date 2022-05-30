import asyncio

import pytest

from .lib.api.models import OperationMode
from .lib.api.exceptions import APIInvalidRequest


async def test_set_get_auto_mode(api_client):
    """Platform should be able to set and get current operation mode"""
    default_mode = await api_client.get_operation_mode()

    assert default_mode == OperationMode.MANUAL

    await api_client.set_operation_mode(OperationMode.AUTO)

    mode = await api_client.get_operation_mode()

    assert mode is OperationMode.AUTO


async def test_reset_back_to_manual_mode(api_client):
    """Platform should back to manual mode when the API is reset"""
    await api_client.set_operation_mode(OperationMode.AUTO)

    mode = await api_client.get_operation_mode()

    assert mode is OperationMode.AUTO

    await api_client.reset()

    mode = await api_client.get_operation_mode()

    assert mode is OperationMode.MANUAL


async def test_reset_close_water_sources(api_client):
    """Platform should turn off all water sources before resetting the API"""
    name, pin = 'Compesa water source', 15

    await api_client.create_water_source(name, pin)
    
    await api_client.set_io_value(pin, 1)

    await api_client.reset()

    assert await api_client.get_io_value(pin) == 0


async def test_set_invalid_operation_mode(api_client):
    request = api_client.create_request('setMode', mode=20)
    payload = api_client.build_request_wrapper(request)

    with pytest.raises(APIInvalidRequest) as exc_info:
        future = await api_client.send_payload(payload, request.id)
        await asyncio.wait_for(future, timeout=7)

    response = exc_info.value.response
    assert response.id == request.id
    assert response.error is APIInvalidRequest
    assert response.message == 'Invalid operation mode'
