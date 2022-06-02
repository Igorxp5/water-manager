import asyncio

import pytest


from .lib.api import APIClient
from .lib.api.models import OperationMode
from .lib.api.exceptions import APIRuntimeError


async def test_auto_fill_until_max_volume(api_client: APIClient):
    """Platform should stop to fill a water tank when it reaches at its max volume"""
    clock_offset = 0

    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1, 1
    water_source_name, water_source_pin = 'Compesa water source', 15

    await api_client.create_water_source(water_source_name, water_source_pin)
    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor, water_source_name)

    await api_client.set_water_tank_minimum_volume(water_tank_name, 10)
    await api_client.set_water_tank_max_volume(water_tank_name, 20)

    water_tank = await api_client.get_water_tank(water_tank_name)

    assert not water_tank['filling']
    
    await api_client.set_operation_mode(OperationMode.AUTO)

    clock_offset += 65 * 1000

    await api_client.set_clock_offset(clock_offset)

    water_tank = await api_client.get_water_tank(water_tank_name)

    assert water_tank['filling']

    await api_client.set_io_value(pressure_sensor, 21)

    water_tank = await api_client.get_water_tank(water_tank_name)

    clock_offset += 65 * 1000

    await api_client.set_clock_offset(clock_offset)

    water_tank = await api_client.get_water_tank(water_tank_name)

    assert not water_tank['filling']



async def test_auto_fill_under_min_volume(api_client: APIClient):
    """Platform should fill a water tank when it under min volume"""
    clock_offset = 0

    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1, 1
    water_source_name, water_source_pin = 'Compesa water source', 15

    await api_client.create_water_source(water_source_name, water_source_pin)
    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor, water_source_name)

    await api_client.set_water_tank_minimum_volume(water_tank_name, 10)
    await api_client.set_water_tank_max_volume(water_tank_name, 20)

    water_tank = await api_client.get_water_tank(water_tank_name)

    assert not water_tank['filling']
    
    await api_client.set_operation_mode(OperationMode.AUTO)

    clock_offset += 65 * 1000

    await api_client.set_clock_offset(clock_offset)

    water_tank = await api_client.get_water_tank(water_tank_name)

    assert water_tank['filling']


async def test_stop_fill_second_water_tank_at_first_min_volume(api_client: APIClient):
    """Platform should stop to fill a water tank if its water tank source reaches its min volume"""
    clock_offset = 0

    water_tank_name_1, pressure_sensor_1, volume_factor_1, pressure_factor_1 = 'Bottom tank', 1, 1, 1
    water_source_name_1, water_source_pin_1 = 'Compesa water source', 15

    await api_client.create_water_source(water_source_name_1, water_source_pin_1)
    await api_client.create_water_tank(water_tank_name_1, pressure_sensor_1, volume_factor_1, pressure_factor_1, water_source_name_1)
    
    water_tank_name_2, pressure_sensor_2, volume_factor_2, pressure_factor_2 = 'Upper tank', 2, 1, 1
    water_source_name_2, water_source_pin_2 = 'Water pump', 16

    await api_client.create_water_source(water_source_name_2, water_source_pin_2, water_tank_name_1)
    await api_client.create_water_tank(water_tank_name_2, pressure_sensor_2, volume_factor_2, pressure_factor_2, water_source_name_2)

    for water_tank_name in [water_tank_name_1, water_tank_name_2]:
        await api_client.set_water_tank_minimum_volume(water_tank_name, 10)
        await api_client.set_water_tank_max_volume(water_tank_name, 20)
    
    await api_client.set_io_value(pressure_sensor_1, 20)
    await api_client.set_io_value(pressure_sensor_2, 10)

    await api_client.set_operation_mode(OperationMode.AUTO)

    clock_offset += 65 * 1000

    await api_client.set_clock_offset(clock_offset)

    assert not (await api_client.get_water_tank(water_tank_name_1))['filling']
    assert (await api_client.get_water_tank(water_tank_name_2))['filling']

    clock_offset += 65 * 1000

    await api_client.set_clock_offset(clock_offset)

    await api_client.set_io_value(pressure_sensor_1, 10)
    await api_client.set_io_value(pressure_sensor_2, 15)

    assert (await api_client.get_water_tank(water_tank_name_1))['filling']
    assert not (await api_client.get_water_tank(water_tank_name_2))['filling']



@pytest.mark.xfail
async def test_stop_fill_third_water_tank_at_first_min_volume(api_client: APIClient):
    """Platform should stop to fill a water tank if any of water tank source reaches its min volume"""
    raise NotImplementedError



async def test_error_water_tank_is_not_filling(api_client: APIClient):
    """Platform should send an RuntimeError message when a water tank should be filling and it's not"""
    clock_offset = 0
    changing_interval = int(5 * 60 * 1000)  # 5 minutes

    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1, 1
    water_source_name, water_source_pin = 'Compesa water source', 15

    await api_client.create_water_source(water_source_name, water_source_pin)
    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor, water_source_name)

    await api_client.set_water_tank_minimum_volume(water_tank_name, 10)
    await api_client.set_water_tank_max_volume(water_tank_name, 20)

    water_tank = await api_client.get_water_tank(water_tank_name)

    assert not water_tank['filling']
    
    await api_client.set_operation_mode(OperationMode.AUTO)

    clock_offset += 65 * 1000

    await api_client.set_clock_offset(clock_offset)

    water_tank = await api_client.get_water_tank(water_tank_name)

    assert water_tank['filling']  # this means the water tank should be filling

    clock_offset += changing_interval

    await api_client.set_clock_offset(clock_offset)

    response = await asyncio.wait_for(api_client.get_error_response(), timeout=7)

    assert response.exception_type is APIRuntimeError
    assert response.message == 'The water tank is not filling'
    assert response.arg == water_tank_name


@pytest.mark.xfail
async def test_error_water_tank_has_stopped_to_filling(api_client: APIClient):
    """
    Platform should send an RuntimeError message when a water tank stops to filling 
    before reaches its max volume
    """
    raise NotImplementedError


async def test_avoid_multiples_turn_on_and_off_due_close_to_range_border(api_client: APIClient):
    """
    Platform should keep water source turned on/off for at least 1 minute
    to avoid multiples filling calls.
    """
    clock_offset = 0

    water_tank_name, pressure_sensor, volume_factor, pressure_factor = 'Bottom tank', 1, 1, 1
    water_source_name, water_source_pin = 'Compesa water source', 15

    await api_client.create_water_source(water_source_name, water_source_pin)
    await api_client.create_water_tank(water_tank_name, pressure_sensor, volume_factor, pressure_factor, water_source_name)

    await api_client.set_water_tank_minimum_volume(water_tank_name, 10)
    await api_client.set_water_tank_max_volume(water_tank_name, 20)
    
    await api_client.set_io_value(pressure_sensor, 10)

    water_tank = await api_client.get_water_tank(water_tank_name)

    assert not water_tank['filling']
    
    await api_client.set_operation_mode(OperationMode.AUTO)

    for _ in range(0, 50, 2):
        clock_offset += 2 * 1000

        await api_client.set_clock_offset(clock_offset)

        water_tank = await api_client.get_water_tank(water_tank_name)

        assert not water_tank['filling']
        
        await asyncio.sleep(0.1)
    
    clock_offset += 15 * 1000

    await api_client.set_clock_offset(clock_offset)

    water_tank = await api_client.get_water_tank(water_tank_name)

    assert water_tank['filling']

    await api_client.set_io_value(pressure_sensor, 20)

    for _ in range(0, 50, 2):
        clock_offset += 2 * 1000

        await api_client.set_clock_offset(clock_offset)

        water_tank = await api_client.get_water_tank(water_tank_name)

        assert water_tank['filling']
        
        await asyncio.sleep(0.1)
    
    clock_offset += 15 * 1000

    await api_client.set_clock_offset(clock_offset)

    water_tank = await api_client.get_water_tank(water_tank_name)

    assert not water_tank['filling']

    await api_client.set_io_value(pressure_sensor, 20)


@pytest.mark.xfail
async def test_interleaved_water_tanks_errors(api_client: APIClient):
    raise NotImplementedError


@pytest.mark.xfail
async def test_auto_stop_when_water_tank_is_not_filling(api_client: APIClient):
    raise NotImplementedError
