import asyncio

import pytest

from .lib.api import APIException, APIInvalidRequest

from protobuf.out.python.api_pb2 import Request, Response


async def test_create_water_source(api_client):
    """Platform should be able to create a water source and get the list of created water sources"""
    await api_client.create_water_source('Compesa water source', 15)
    
    water_sources = await api_client.get_water_source_list()
    
    assert water_sources == ['Compesa water source']


async def test_max_water_sources(api_client):
    """Platform should not allow to create more than 10 water sources"""
    expected_water_sources = [(f'Water source {i}', i) for i in range(1, 11)]

    for name, pin in expected_water_sources:
        await api_client.create_water_source(name, pin)
    
    water_sources = await api_client.get_water_source_list()
    
    assert water_sources == [name for name, _ in expected_water_sources]

    with pytest.raises(APIInvalidRequest) as exc_info:
        await api_client.create_water_source('Last water source', 20)

    response = exc_info.value.response
    assert response.error is APIInvalidRequest
    assert response.message == 'Max of water sources reached'

    water_sources = await api_client.get_water_source_list()
    
    assert water_sources == [name for name, _ in expected_water_sources]


async def test_create_already_registered_water_source(api_client):
    """Platform should respond with an error when the user creates a already registered water source"""
    water_source_name = 'Compesa water source'
    error_message = 'There is already a water source with that name registered'
    expected_water_sources = [water_source_name]
    
    await api_client.create_water_source(water_source_name, 15)
    
    water_sources = await api_client.get_water_source_list()

    assert water_sources == expected_water_sources

    # with the same pin
    with pytest.raises(APIInvalidRequest) as exc_info:
        await api_client.create_water_source(water_source_name, 15)

    response = exc_info.value.response
    assert response.error is APIInvalidRequest
    assert response.message == error_message

    water_sources = await api_client.get_water_source_list()

    assert water_sources == expected_water_sources

    # with a different pin
    with pytest.raises(APIInvalidRequest) as exc_info:
        await api_client.create_water_source(water_source_name, 15)

    response = exc_info.value.response
    assert response.error is APIInvalidRequest
    assert response.message == error_message

    water_sources = await api_client.get_water_source_list()
    
    assert water_sources == expected_water_sources


async def test_max_length_water_source_name(api_client):
    """Platform should not allow more than 20 characters to name a water source. The request is not decodable."""
    name = 'A long water source name is this one'
    
    request = api_client.create_request('createWaterSource', name=name, pin=15)
    payload = api_client.build_request_wrapper(request)

    future = await api_client.send_payload(payload, request.id)

    response = await asyncio.wait_for(api_client.get_error_response(), timeout=7)

    assert response.id == 0
    assert response.error is APIException
    assert response.message == 'Failed to decode the request'


async def test_remove_water_source(api_client):
    """Platform should be able to remove a water source created before.
    After removing it the memory allocated in RAM for it, it should be deallocated
    """
    name, pin = 'Compesa water source', 15
    await api_client.create_water_source(name, pin)

    water_sources = await api_client.get_water_source_list()
    
    assert water_sources == [name]

    await api_client.remove_water_source(name)

    water_sources = await api_client.get_water_source_list()

    assert not water_sources

    allocated_memory = await api_client.get_memory_free()

    await api_client.create_water_source(name, pin)

    water_sources = await api_client.get_water_source_list()
    
    assert water_sources == [name]

    await api_client.remove_water_source(name)

    water_sources = await api_client.get_water_source_list()

    assert not water_sources

    assert await api_client.get_memory_free() <= allocated_memory, 'Memory leak found while removing a water source'


async def test_remove_water_source_keep_ios(api_client):
    """Platform should keep IOs created for the water source when removing it"""
    name, pin = 'Compesa water source', 15

    await api_client.create_water_source(name, pin)

    await api_client.remove_water_source(name)

    await api_client.get_io_value(pin=pin)


def test_create_water_source_with_water_tank_source(api_client):
    """Platform should be able create a water source using a water tank as sources"""
    raise NotImplementedError


def test_remove_water_source_associated_to_water_tank(api_client):
    """Platform should not allow to remove a water source of a water tank"""
    raise NotImplementedError


async def test_get_water_source(api_client):
    """Platform should be able to get a water source registered"""
    name, pin = 'Compesa water source', 15

    await api_client.create_water_source(name, pin)

    water_source = await api_client.get_water_source(name)

    assert water_source['name'] == name
    assert water_source['pin'] == pin
    assert water_source['enabled'] == False
    assert not water_source['sourceWaterTank']


async def test_get_invalid_water_source(api_client):
    """Platform should respond with an error when trying to get a invalid water source"""
    name = 'Compesa water source'
    error_message = 'Could not find a water source with the name provided'

    with pytest.raises(APIInvalidRequest) as exc_info:
        await api_client.get_water_source(name)

    response = exc_info.value.response
    assert response.error is APIInvalidRequest
    assert response.message == error_message
