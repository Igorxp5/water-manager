import math
import asyncio
import logging

import pytest

from .lib.api import APIClient
from .lib.api.models import OperationMode
from .lib.api.exceptions import APIException, APIInvalidRequest

LOGGER = logging.getLogger(__name__)
FLOAT_ERROR_TOLERANCE = 0.03  # 3%
MAX_WATER_TANKS = 5


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
    """Platform should not allow to create more than 5 water tanks"""
    volume_factor, pressure_factor = 1.5, 2.5
    expected_water_tanks = [(f'Water tank {i}', i) for i in range(1, MAX_WATER_TANKS + 1)]

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
    assert response.exception_type is APIInvalidRequest
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
        assert response.exception_type is APIInvalidRequest
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
    assert response.exception_type is APIException
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
    assert response.exception_type is APIInvalidRequest
    assert response.message == 'Could not find a water tank with the name provided'


async def test_get_water_tank(api_client: APIClient):
    """Platform should be able to get a water tank registered"""
    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1.5, 2.5

    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor)

    water_tank = await api_client.get_water_tank(water_tank_name)

    assert water_tank['name'] == water_tank_name
    assert water_tank['pressureSensorPin'] == pressure_sensor
    assert water_tank['filling'] == False
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
    assert water_tank['filling'] == False
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
    assert response.exception_type is APIInvalidRequest
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
    assert response.exception_type is APIInvalidRequest
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
    assert response.exception_type is APIInvalidRequest
    assert response.message == 'Could not find a water source with the name provided'    


async def test_set_water_tank_minimum_volume(api_client: APIClient):
    """Platform should be able to set water tank minimum volume"""
    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1.5, 2.5
    
    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor)

    await api_client.set_water_tank_minimum_volume(water_tank_name, 20)

    water_tank = await api_client.get_water_tank(water_tank_name)

    assert water_tank['minimumVolume'] == 20


async def test_set_water_tank_max_volume(api_client: APIClient):
    """Platform should be able to set water tank max volume"""
    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1.5, 2.5
    
    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor)

    await api_client.set_water_tank_max_volume(water_tank_name, 100)

    water_tank = await api_client.get_water_tank(water_tank_name)

    assert water_tank['maxVolume'] == 100


async def test_set_water_tank_volume_factor(api_client: APIClient):
    """Platform should be able to set water tank volume factor"""
    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1.5, 2.5
    
    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor)

    for volume_factor in [5, 10, 100, 500.25, 1000,  5000.5, 10000]:

        await api_client.set_water_tank_volume_factor(water_tank_name, volume_factor)

        water_tank = await api_client.get_water_tank(water_tank_name)

        assert water_tank['volumeFactor'] == volume_factor

        for raw_pressure_value in [1, 10, 100, 500, 1000]:
            expected_pressure = raw_pressure_value * pressure_factor
            expected_volume = expected_pressure * volume_factor
            await api_client.set_io_value(pressure_sensor, raw_pressure_value)

            water_tank = await api_client.get_water_tank(water_tank_name)

            assert water_tank['volumeFactor'] == volume_factor
            assert math.isclose(water_tank['rawPressureValue'], raw_pressure_value, rel_tol=FLOAT_ERROR_TOLERANCE)
            assert math.isclose(water_tank['volume'], expected_volume, rel_tol=FLOAT_ERROR_TOLERANCE)
            assert math.isclose(water_tank['pressure'], expected_pressure, rel_tol=FLOAT_ERROR_TOLERANCE)

            await asyncio.sleep(0.01)


async def test_set_water_tank_pressure_factor(api_client: APIClient):
    """Platform should be able to set water tank pressure factor"""
    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1.5, 2.5

    zero_volume_pressure = 0
    
    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor)

    for pressure_factor in [5, 10, 100, 500.25, 1000,  5000.5, 10000]:

        await api_client.set_water_tank_pressure_factor(water_tank_name, pressure_factor)

        water_tank = await api_client.get_water_tank(water_tank_name)

        assert water_tank['pressureFactor'] == pressure_factor

        for raw_pressure_value in [1, 10, 100, 500, 1000]:
            expected_pressure = raw_pressure_value * pressure_factor
            expected_volume = max(0, (expected_pressure * volume_factor) - zero_volume_pressure)
            await api_client.set_io_value(pressure_sensor, raw_pressure_value)

            water_tank = await api_client.get_water_tank(water_tank_name)

            assert water_tank['pressureFactor'] == pressure_factor
            assert math.isclose(water_tank['rawPressureValue'], raw_pressure_value, rel_tol=FLOAT_ERROR_TOLERANCE)
            assert math.isclose(water_tank['volume'], expected_volume, rel_tol=FLOAT_ERROR_TOLERANCE)
            assert math.isclose(water_tank['pressure'], expected_pressure, rel_tol=FLOAT_ERROR_TOLERANCE)

            await asyncio.sleep(0.01)


async def test_set_water_tank_zero_volume_factor(api_client: APIClient):
    """Platform should be able to set water tank zero volume factor"""
    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1.5, 2.5
    
    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor)

    for zero_volume_pressure in [5, 10, 100, 500.25, 1000,  5000.5, 10000]:

        await api_client.set_water_tank_zero_volume_pressure(water_tank_name, zero_volume_pressure)

        water_tank = await api_client.get_water_tank(water_tank_name)

        assert water_tank['zeroVolumePressure'] == zero_volume_pressure

        for raw_pressure_value in [1, 10, 100, 500, 1000]:
            expected_pressure = raw_pressure_value * pressure_factor
            expected_volume = max(0, (expected_pressure * volume_factor) - zero_volume_pressure)
            await api_client.set_io_value(pressure_sensor, raw_pressure_value)

            water_tank = await api_client.get_water_tank(water_tank_name)

            assert water_tank['zeroVolumePressure'] == zero_volume_pressure
            assert math.isclose(water_tank['rawPressureValue'], raw_pressure_value, rel_tol=FLOAT_ERROR_TOLERANCE)
            assert math.isclose(water_tank['volume'], expected_volume, rel_tol=FLOAT_ERROR_TOLERANCE)
            assert math.isclose(water_tank['pressure'], expected_pressure, rel_tol=FLOAT_ERROR_TOLERANCE)

            await asyncio.sleep(0.01)


async def test_set_water_tank_pressure_changing_value(api_client: APIClient):
    """Platform should be able to set water tank pressure changing value"""
    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1.5, 2.5
    
    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor)

    water_tank = await api_client.get_water_tank(water_tank_name)

    assert math.isclose(water_tank['pressureChangingValue'], 0.2, rel_tol=FLOAT_ERROR_TOLERANCE)

    await api_client.set_water_tank_pressure_changing_value(water_tank_name, 1)

    water_tank = await api_client.get_water_tank(water_tank_name)

    assert math.isclose(water_tank['pressureChangingValue'], 1, rel_tol=FLOAT_ERROR_TOLERANCE)


async def test_fill_invalid_water_tank(api_client: APIClient):
    """
    Platform should be respond with an error when trying to fill a 
    invalid water tank manually in manual mode
    """
    with pytest.raises(APIInvalidRequest) as exc_info:
        await api_client.fill_water_tank('A water tank', True)
    
    response = exc_info.value.response
    assert response.exception_type is APIInvalidRequest
    assert response.message == 'Could not find a water tank with the name provided'


async def test_stop_fill_invalid_water_tank(api_client: APIClient):
    """
    Platform should be respond with an error when trying to fill a 
    invalid water tank manually in manual mode
    """
    with pytest.raises(APIInvalidRequest) as exc_info:
        await api_client.fill_water_tank('A water tank', False)
    
    response = exc_info.value.response
    assert response.exception_type is APIInvalidRequest
    assert response.message == 'Could not find a water tank with the name provided'


async def test_fill_water_tank_manually_in_auto_mode(api_client: APIClient):
    """Platform should respond with an error when trying to fill water tank manually in auto mode"""
    await api_client.set_operation_mode(OperationMode.AUTO)

    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1.5, 2.5
    water_source_name, water_source_pin = 'Compesa water source', 15

    await api_client.create_water_source(water_source_name, water_source_pin)
    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor, water_source_name)

    with pytest.raises(APIInvalidRequest) as exc_info:
        await api_client.fill_water_tank(water_tank_name, True)
    
    response = exc_info.value.response
    assert response.exception_type is APIInvalidRequest
    assert response.message == 'Cannot handle a water tank in auto mode'
    
    water_tank = await api_client.get_water_tank(water_tank_name)

    assert not water_tank['filling']


async def test_fill_water_tank_manually_in_manual_mode(api_client: APIClient):
    """Platform should be able to fill a water tank manually in manual mode"""
    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1.5, 2.5
    water_source_name, water_source_pin = 'Compesa water source', 15

    await api_client.create_water_source(water_source_name, water_source_pin)
    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor, water_source_name)

    await api_client.set_water_tank_max_volume(water_tank_name, 20)

    await api_client.fill_water_tank(water_tank_name, True)

    water_tank = await api_client.get_water_tank(water_tank_name)

    assert water_tank['filling']


async def test_stop_fill_water_tank_in_auto_mode(api_client: APIClient):
    """
    Platform should respond with an error when trying 
    to stop to fill water tank manually in auto mode.
    """
    await api_client.set_operation_mode(OperationMode.AUTO)

    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1.5, 2.5
    water_source_name, water_source_pin = 'Compesa water source', 15

    await api_client.create_water_source(water_source_name, water_source_pin)
    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor, water_source_name)

    with pytest.raises(APIInvalidRequest) as exc_info:
        await api_client.fill_water_tank(water_tank_name, False)
    
    response = exc_info.value.response
    assert response.exception_type is APIInvalidRequest
    assert response.message == 'Cannot handle a water tank in auto mode'


async def test_stop_fill_water_tank_in_manual_mode(api_client: APIClient):
    """Platform should be able to stop to fill a water tank manually in manual mode"""
    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1.5, 2.5
    water_source_name, water_source_pin = 'Compesa water source', 15

    await api_client.create_water_source(water_source_name, water_source_pin)
    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor, water_source_name)

    await api_client.set_water_tank_max_volume(water_tank_name, 20)

    await api_client.fill_water_tank(water_tank_name, True)
    
    water_tank = await api_client.get_water_tank(water_tank_name)

    assert water_tank['filling']
    
    await api_client.fill_water_tank(water_tank_name, False)

    water_tank = await api_client.get_water_tank(water_tank_name)

    assert not water_tank['filling']


async def test_fill_water_tank_with_max_volume(api_client: APIClient):
    """
    Platform should respond with an error when trying to fill water tank 
    with volume in its maximum allowed.
    """
    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1.5, 2.5
    water_source_name, water_source_pin = 'Compesa water source', 15

    await api_client.create_water_source(water_source_name, water_source_pin)
    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor, water_source_name)

    await api_client.set_water_tank_max_volume(water_tank_name, 20)
    await api_client.set_io_value(pressure_sensor, 20)

    with pytest.raises(APIInvalidRequest) as exc_info:
        await api_client.fill_water_tank(water_tank_name, True)
    
    response = exc_info.value.response
    assert response.exception_type is APIInvalidRequest
    assert response.message == 'Cannot fill the water tank, maximum threshold reached'

    water_tank = await api_client.get_water_tank(water_tank_name)

    assert not water_tank['filling']


async def test_fill_water_tank_with_max_volume_force(api_client: APIClient):
    """
    Platform should be able to fill water tank even when its volume reaches 
    the maximum value allowed if force flag is present.
    """
    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1.5, 2.5
    water_source_name, water_source_pin = 'Compesa water source', 15

    await api_client.create_water_source(water_source_name, water_source_pin)
    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor, water_source_name)

    await api_client.set_water_tank_max_volume(water_tank_name, 20)
    await api_client.set_io_value(pressure_sensor, 20)

    await api_client.fill_water_tank(water_tank_name, True, True)

    water_tank = await api_client.get_water_tank(water_tank_name)

    assert water_tank['filling']


async def test_fill_water_tank_without_water_source(api_client: APIClient):
    """
    Platform should respond with an error when trying to fill water tank 
    without water source
    """
    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1.5, 2.5

    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor)

    await api_client.set_water_tank_max_volume(water_tank_name, 20)

    with pytest.raises(APIInvalidRequest) as exc_info:
        await api_client.fill_water_tank(water_tank_name, True)
    
    response = exc_info.value.response
    assert response.exception_type is APIInvalidRequest
    assert response.message == 'Cannot fill a water tank without setting a water source for it'

    water_tank = await api_client.get_water_tank(water_tank_name)

    assert not water_tank['filling']


async def test_fill_water_tank_without_water_source_force(api_client: APIClient):
    """
    Platform should not be able to fill water tank when it doesn't have a water source 
    even the force flag is present.
    """
    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1.5, 2.5

    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor)

    await api_client.set_water_tank_max_volume(water_tank_name, 20)

    with pytest.raises(APIInvalidRequest) as exc_info:
        await api_client.fill_water_tank(water_tank_name, True, True)
    
    response = exc_info.value.response
    assert response.exception_type is APIInvalidRequest
    assert response.message == 'Cannot fill a water tank without setting a water source for it'

    water_tank = await api_client.get_water_tank(water_tank_name)

    assert not water_tank['filling']


async def test_set_water_tank_active(api_client: APIClient):
    """
    Platform should be able to active and deactivate a water tank.
    Platform should responnd with an error when trying to fill a deactivateed water tank.
    Platform should stop to fill water tank when it's deactivated.
    """
    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1.5, 2.5
    water_source_name, water_source_pin = 'Compesa water source', 15

    await api_client.create_water_source(water_source_name, water_source_pin)
    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor, water_source_name)

    await api_client.set_water_tank_max_volume(water_tank_name, 20)

    water_tank = await api_client.get_water_tank(water_tank_name)

    assert water_tank['active'] == True
    assert water_tank['filling'] == False

    await api_client.fill_water_tank(water_tank_name, True)

    water_tank = await api_client.get_water_tank(water_tank_name)

    assert water_tank['active'] == True
    assert water_tank['filling'] == True
    
    await api_client.set_water_tank_active(water_tank_name, False)

    water_tank = await api_client.get_water_tank(water_tank_name)

    assert water_tank['active'] == False
    assert water_tank['filling'] == False

    with pytest.raises(APIInvalidRequest) as exc_info: 
        await api_client.fill_water_tank(water_tank_name, True)

    response = exc_info.value.response
    assert response.message == 'Cannot fill a deactivated water tank'

    await api_client.fill_water_tank(water_tank_name, True, True)

    water_tank = await api_client.get_water_tank(water_tank_name)

    assert water_tank['active'] == True
    assert water_tank['filling'] == True


async def test_create_empty_water_tank(api_client: APIClient):
    """Platform should respond with en error when trying to create a water tank without a name"""
    water_tank_name, pressure_sensor, volume_factor, pressure_factor = '', 1, 1.5, 2.5

    with pytest.raises(APIInvalidRequest) as exc_info:
        await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor)

    response = exc_info.value.response
    assert response.exception_type is APIInvalidRequest
    assert response.message == 'Cannot create a resource with an empty name'
