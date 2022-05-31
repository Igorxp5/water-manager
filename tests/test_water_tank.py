import asyncio
import logging

import pytest

from .lib.api import APIClient
from .lib.api.exceptions import APIException, APIInvalidRequest

LOGGER = logging.getLogger(__name__)


async def test_create_water_tank(api_client: APIClient):
    """Platform should be able to create a water tank and get the list of created water tanks"""
    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1.5, 2.5

    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor)

    water_tanks = await api_client.get_water_tank_list()

    assert water_tanks == [water_tank_name]


async def test_create_water_tank_with_water_source(api_client: APIClient):
    """
    Platform should be able to create a water tank associated to a water source
    and get the list of created water tanks
    """
    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1.5, 2.5
    water_source_name, water_source_pin = 'Compesa water source', 15

    await api_client.create_water_source(water_source_name, water_source_pin)
    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor, water_source_name)

    water_tanks = await api_client.get_water_tank_list()

    assert water_tanks == [water_tank_name]


async def test_max_water_tanks(api_client: APIClient):
    """Platform should not allow to create more than 10 water tanks"""
    volume_factor, pressure_factor = 1.5, 2.5
    expected_water_tanks = [(f'Water tank {i}', i) for i in range(1, 11)]

    for name, pressure_sensor in expected_water_tanks:
        free_memory = await api_client.get_free_memory()
        LOGGER.debug(f'Create a water tank: {name}...')
        LOGGER.debug(f'Current free memory: {free_memory} bytes')
        await api_client.create_water_tank(name, pressure_sensor, volume_factor, pressure_factor)
        assert free_memory > 500

    water_tanks = await api_client.get_water_tank_list()
    
    assert water_tanks == [name for name, _ in expected_water_tanks]

    with pytest.raises(APIInvalidRequest) as exc_info:
        await api_client.create_water_tank('Last water tank', 20, volume_factor, pressure_factor)

    response = exc_info.value.response
    assert response.error is APIInvalidRequest
    assert response.message == 'Max of water tanks reached'

    water_tanks = await api_client.get_water_tank_list()
    
    assert water_tanks == [name for name, _ in expected_water_tanks]


async def test_create_already_registered_water_tank(api_client: APIClient):
    """Platform should respond with an error when the user creates a already registered water tank"""
    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1.5, 2.5
    error_message = 'There is already a water tank with that name registered'
    expected_water_tanks = [water_tank_name]
    
    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor)
    
    water_tanks = await api_client.get_water_tank_list()

    assert water_tanks == expected_water_tanks

    expected_free_memory = await api_client.get_free_memory()

    LOGGER.debug(f'Free memory: {expected_free_memory} bytes!')

    for pin in [pressure_sensor, 25]:
        with pytest.raises(APIInvalidRequest) as exc_info:
            await api_client.create_water_tank(water_tank_name, pin, volume_factor, pressure_factor)

        free_memory = await api_client.get_free_memory()

        LOGGER.debug(f'Free memory: {free_memory} bytes!')
        
        assert free_memory == expected_free_memory

        response = exc_info.value.response
        assert response.error is APIInvalidRequest
        assert response.message == error_message

        water_tanks = await api_client.get_water_tank_list()

    assert water_tanks == expected_water_tanks


async def test_max_length_water_tank_name(api_client: APIClient):
    """Platform should not allow more than 20 characters to name a water tank. The request is not decodable."""
    water_tank_name = 'A long water tank name is this one'
    pressure_sensor, volume_factor, pressure_factor = 1, 1.5, 2.5
    
    request = api_client.create_request('createWaterTank', name=water_tank_name, pressureSensorPin=pressure_sensor,
                                        volumeFactor=volume_factor, pressureFactor=pressure_factor)
    payload = api_client.build_request_wrapper(request)

    future = await api_client.send_payload(payload, request.id)

    response = await asyncio.wait_for(api_client.get_error_response(), timeout=7)

    assert response.id == 0
    assert response.error is APIException
    assert response.message == 'Failed to decode the request'


async def test_remove_water_tank(api_client: APIClient):
    """
    Platform should be able to remove a water tank created before.
    After removing it the memory allocated in RAM for it, it should be deallocated
    """
    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1.5, 2.5
    
    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor)

    water_tanks = await api_client.get_water_tank_list()

    assert water_tanks == [water_tank_name]

    await api_client.remove_water_tank(water_tank_name)

    water_tanks = await api_client.get_water_tank_list()

    assert water_tanks == []

    allocated_memory = await api_client.get_free_memory()

    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor)

    water_tanks = await api_client.get_water_tank_list()
    
    assert water_tanks == [water_tank_name]

    await api_client.remove_water_tank(water_tank_name)

    water_tanks = await api_client.get_water_tank_list()

    assert water_tanks == []

    assert await api_client.get_free_memory() <= allocated_memory, 'Memory leak found while removing a water source'


async def test_remove_water_tank_keep_ios(api_client: APIClient):
    """Platform should keep IOs created for the water tank when removing it"""
    pressure_sensor, volume_factor, pressure_factor = 1, 1.5, 2.5
    
    await api_client.create_water_tank('Water Tank 1', pressure_sensor, volume_factor, pressure_factor)

    await api_client.set_io_value(pin=pressure_sensor, value=5)

    await api_client.create_water_tank('Water Tank 2', pressure_sensor, volume_factor, pressure_factor)

    await api_client.remove_water_tank('Water Tank 1')

    await api_client.get_io_value(pin=pressure_sensor) == 5


async def test_remove_invalid_water_tank(api_client: APIClient):
    """Platform should be able to remove water tank that doesn't exist"""
    with pytest.raises(APIInvalidRequest) as exc_info:
        await api_client.remove_water_tank('Water tank 1')

    response = exc_info.value.response
    assert response.error is APIInvalidRequest
    assert response.message == 'Could not find a water tank with the name provided'


async def test_get_water_tank(api_client: APIClient):
    """Platform should be able to get a water tank registered"""
    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1.5, 2.5

    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor)

    water_tank = await api_client.get_water_tank(water_tank_name)

    assert water_tank['name'] == water_tank_name
    assert water_tank['pressureSensorPin'] == pressure_sensor
    assert water_tank['isFilling'] == False
    assert water_tank['volumeFactor'] == volume_factor
    assert water_tank['pressureFactor'] == pressure_factor
    assert water_tank['minimumVolume'] == 0
    assert water_tank['maxVolume'] == 0
    assert water_tank['zeroVolumePressure'] == 0
    assert water_tank['rawPressureValue'] == 0
    assert water_tank['pressure'] == 0
    assert water_tank['volume'] == 0
    assert water_tank['waterSource'] is None


async def test_get_water_tank_with_water_source(api_client: APIClient):
    """Platform should be able get a water tank that uses a water source"""
    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1.5, 2.5
    water_source_name, water_source_pin = 'Compesa water source', 15

    await api_client.create_water_source(water_source_name, water_source_pin)
    
    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor,
                                       pressure_factor, water_source_name)

    water_tank = await api_client.get_water_tank(water_tank_name)

    assert water_tank['name'] == water_tank_name
    assert water_tank['pressureSensorPin'] == pressure_sensor
    assert water_tank['isFilling'] == False
    assert water_tank['volumeFactor'] == volume_factor
    assert water_tank['pressureFactor'] == pressure_factor
    assert water_tank['minimumVolume'] == 0
    assert water_tank['maxVolume'] == 0
    assert water_tank['zeroVolumePressure'] == 0
    assert water_tank['rawPressureValue'] == 0
    assert water_tank['pressure'] == 0
    assert water_tank['volume'] == 0
    assert water_tank['waterSource'] == water_source_name

async def test_remove_water_tank_associated_to_water_source(api_client: APIClient):
    """Platform should not allow to remove a water tank of a water source"""
    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1.5, 2.5
    water_source_name, water_source_pin = 'Compesa water source', 15

    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor)

    await api_client.create_water_source(water_source_name, water_source_pin, water_tank_name)

    
    with pytest.raises(APIInvalidRequest) as exc_info:
        await api_client.remove_water_tank(water_tank_name)

    response = exc_info.value.response
    assert response.error is APIInvalidRequest
    assert response.message == 'Cannot remove the water tank, there is a water source dependent of it'

    water_sources = await api_client.get_water_source_list()

    water_tanks = await api_client.get_water_tank_list()

    assert water_sources == [water_source_name]

    assert water_tanks == [water_tank_name]


async def test_get_invalid_water_tank(api_client: APIClient):
    """Platform should respond with an error when trying to get a invalid water tank"""
    water_tank_name = 'Bottom tank'

    error_message = 'Could not find a water tank with the name provided'

    with pytest.raises(APIInvalidRequest) as exc_info:
        await api_client.get_water_tank(water_tank_name)

    response = exc_info.value.response
    assert response.error is APIInvalidRequest
    assert response.message == error_message

async def test_create_water_tank_with_invalid_water_source(api_client: APIClient):
    """
    Platform should respond with an error when creating a water tank with
    name of a water source that doesn't exist. 
    """
    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1.5, 2.5

    with pytest.raises(APIInvalidRequest) as exc_info:
        await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor, 'Compesa water source')

    response = exc_info.value.response
    assert response.error is APIInvalidRequest
    assert response.message == 'Could not find a water source with the name provided'    
