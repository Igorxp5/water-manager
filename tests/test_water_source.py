import asyncio
import logging

import pytest

from .lib.api import APIClient
from .lib.api.models import OperationMode
from .lib.api.exceptions import APIException, APIInvalidRequest

LOGGER = logging.getLogger(__name__)


async def test_create_water_source(api_client: APIClient):
    """Platform should be able to create a water source and get the list of created water sources"""
    await api_client.create_water_source('Compesa water source', 15)
    
    water_sources = await api_client.get_water_source_list()

    assert water_sources == ['Compesa water source']


async def test_max_water_sources(api_client: APIClient):
    """Platform should not allow to create more than 10 water sources"""
    expected_water_sources = [(f'Water source {i}', i) for i in range(1, 11)]

    for name, pin in expected_water_sources:
        free_memory = await api_client.get_free_memory()
        LOGGER.debug(f'Create a water source: {name}...')
        LOGGER.debug(f'Current free memory: {free_memory} bytes')
        await api_client.create_water_source(name, pin)
        assert free_memory > 500
    
    water_sources = await api_client.get_water_source_list()
    
    assert water_sources == [name for name, _ in expected_water_sources]

    with pytest.raises(APIInvalidRequest) as exc_info:
        await api_client.create_water_source('Last water source', 20)

    response = exc_info.value.response
    assert response.exception_type is APIInvalidRequest
    assert response.message == 'Max of water sources reached'

    water_sources = await api_client.get_water_source_list()
    
    assert water_sources == [name for name, _ in expected_water_sources]


async def test_create_already_registered_water_source(api_client: APIClient):
    """Platform should respond with an error when the user creates a already registered water source"""
    water_source_name, water_source_pin = 'Compesa water source', 15
    error_message = 'There is already a water source with that name registered'
    expected_water_sources = [water_source_name]
    
    await api_client.create_water_source(water_source_name, water_source_pin)
    
    water_sources = await api_client.get_water_source_list()

    assert water_sources == expected_water_sources

    expected_free_memory = await api_client.get_free_memory()

    LOGGER.debug(f'Free memory: {expected_free_memory} bytes!')

    for pin in [water_source_pin, 25]:
        with pytest.raises(APIInvalidRequest) as exc_info:
            await api_client.create_water_source(water_source_name, pin)

        free_memory = await api_client.get_free_memory()

        LOGGER.debug(f'Free memory: {free_memory} bytes!')
        
        assert free_memory == expected_free_memory

        response = exc_info.value.response
        assert response.exception_type is APIInvalidRequest
        assert response.message == error_message

        water_sources = await api_client.get_water_source_list()

        assert water_sources == expected_water_sources


async def test_max_length_water_source_name(api_client: APIClient):
    """Platform should not allow more than 20 characters to name a water source. The request is not decodable."""
    name = 'A long water source name is this one'
    
    request = api_client.create_request('createWaterSource', name=name, pin=15)
    payload = api_client.build_request_wrapper(request)

    future = await api_client.send_payload(payload, request.id)

    response = await asyncio.wait_for(api_client.get_error_response(), timeout=7)

    assert response.id == 0
    assert response.exception_type is APIException
    assert response.message == 'Failed to decode the request'


async def test_remove_water_source(api_client: APIClient):
    """
    Platform should be able to remove a water source created before.
    After removing it the memory allocated in RAM for it, it should be deallocated
    """
    name, pin = 'Compesa water source', 15
    await api_client.create_water_source(name, pin)

    water_sources = await api_client.get_water_source_list()
    
    assert water_sources == [name]

    await api_client.remove_water_source(name)

    water_sources = await api_client.get_water_source_list()

    assert water_sources == []

    allocated_memory = await api_client.get_free_memory()

    await api_client.create_water_source(name, pin)

    water_sources = await api_client.get_water_source_list()
    
    assert water_sources == [name]

    await api_client.remove_water_source(name)

    water_sources = await api_client.get_water_source_list()

    assert water_sources == []

    assert await api_client.get_free_memory() <= allocated_memory, 'Memory leak found while removing a water source'


async def test_remove_water_source_keep_ios(api_client: APIClient):
    """Platform should keep IOs created for the water source when removing it"""
    pin = 15

    await api_client.create_water_source('Water source 1', 15)

    await api_client.create_water_source('Water source 2', 15)

    await api_client.set_io_value(pin=pin, value=5)

    await api_client.remove_water_source('Water source 1')

    await api_client.get_io_value(pin=pin) == 5


async def test_remove_invalid_water_source(api_client: APIClient):
    """Platform should be able to remove water source that doesn't exist"""
    with pytest.raises(APIInvalidRequest) as exc_info:
        await api_client.remove_water_source('Water source 1')

    response = exc_info.value.response
    assert response.exception_type is APIInvalidRequest
    assert response.message == 'Could not find a water source with the name provided'


async def test_create_water_source_with_water_tank_source(api_client: APIClient):
    """Platform should be able create a water source using a water tank as sources"""
    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1.5, 2.5
    water_source_name, water_source_pin = 'Compesa water source', 15

    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor)

    await api_client.create_water_source(water_source_name, water_source_pin, water_tank_name)
    
    water_source = await api_client.get_water_source(water_source_name)

    assert water_source['name'] == water_source_name
    assert water_source['pin'] == water_source_pin
    assert water_source['enabled'] == False
    assert water_source['sourceWaterTank'] == water_tank_name


async def test_remove_water_source_associated_to_water_tank(api_client: APIClient):
    """Platform should not allow to remove a water source of a water tank"""
    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1.5, 2.5
    water_source_name, water_source_pin = 'Compesa water source', 15

    await api_client.create_water_source(water_source_name, water_source_pin)

    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor,
                                       pressure_factor, water_source_name)
    
    with pytest.raises(APIInvalidRequest) as exc_info:
        await api_client.remove_water_source(water_source_name)
    
    response = exc_info.value.response
    assert response.exception_type is APIInvalidRequest
    assert response.message == 'Cannot remove the water source, there is a water tank dependent of it'

    water_sources = await api_client.get_water_source_list()

    water_tanks = await api_client.get_water_tank_list()

    assert water_sources == [water_source_name]

    assert water_tanks == [water_tank_name]


async def test_get_water_source(api_client: APIClient):
    """Platform should be able to get a water source registered"""
    name, pin = 'Compesa water source', 15

    await api_client.create_water_source(name, pin)

    water_source = await api_client.get_water_source(name)

    assert water_source['name'] == name
    assert water_source['pin'] == pin
    assert water_source['enabled'] == False
    assert not water_source['sourceWaterTank']


async def test_get_invalid_water_source(api_client: APIClient):
    """Platform should respond with an error when trying to get a invalid water source"""
    name = 'Compesa water source'
    error_message = 'Could not find a water source with the name provided'

    with pytest.raises(APIInvalidRequest) as exc_info:
        await api_client.get_water_source(name)

    response = exc_info.value.response
    assert response.exception_type is APIInvalidRequest
    assert response.message == error_message


async def test_set_water_source_state_manual_mode(api_client: APIClient):
    """Platform should allow to set turn on/off water sources when it's in manual mode"""
    name, pin = 'Compesa water source', 15

    await api_client.create_water_source(name, pin)
    
    for _ in range(2):
        for state in [True, False]:
            await api_client.set_water_source_state(name, state)
            water_source = await api_client.get_water_source(name)
            assert water_source['enabled'] == state
        
        await api_client.set_operation_mode(OperationMode.AUTO)
        await api_client.set_operation_mode(OperationMode.MANUAL)


async def test_set_water_source_state_auto_mode(api_client: APIClient):
    """Platform should not allow to turn on/off water sources when it's in auto mode"""
    name, pin = 'Compesa water source', 15
    error_message = 'Cannot handle a water source in auto mode'

    await api_client.create_water_source(name, pin)

    await api_client.set_operation_mode(OperationMode.AUTO)

    mode = await api_client.get_operation_mode()

    assert mode is OperationMode.AUTO

    for state in [True, False]:
        with pytest.raises(APIInvalidRequest) as exc_info:
            await api_client.set_water_source_state(name, state)
    
        response = exc_info.value.response
        assert response.exception_type is APIInvalidRequest
        assert response.message == error_message


async def test_create_water_source_with_invalid_water_tank(api_client: APIClient):
    """
    Platform should respond with an error when creating a water source with
    name of a water tank that doesn't exist. 
    """
    name, pin = 'Compesa water source', 15

    with pytest.raises(APIInvalidRequest) as exc_info:
        await api_client.create_water_source(name, pin, 'Bottom tank')
    
    response = exc_info.value.response
    assert response.exception_type is APIInvalidRequest
    assert response.message == 'Could not find a water tank with the name provided'    
