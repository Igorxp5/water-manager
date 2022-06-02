import asyncio
import logging

import pytest

from .lib.api import APIClient

LOGGER = logging.getLogger(__name__)


async def test_create_max_items(api_client: APIClient):
    """
    Plataform should have enough memory to create the max of 
    water sources/water tanks.
    """
    volume_factor, pressure_factor = 1.5, 2.5
    expected_water_sources = [(f'Water source {i}', i) for i in range(1, 11)]
    expected_water_tanks = [(f'Water tank {i}', i) for i in range(1, 11)]

    for name, pin in expected_water_sources:
        free_memory = await api_client.get_free_memory()
        LOGGER.debug(f'Create a water source: {name}...')
        LOGGER.debug(f'Current free memory: {free_memory} bytes')
        await api_client.create_water_source(name, pin)
        assert free_memory > 500
        await asyncio.sleep(0.1)
    
    for name, pressure_sensor in expected_water_tanks:
        free_memory = await api_client.get_free_memory()
        LOGGER.debug(f'Create a water tank: {name}...')
        LOGGER.debug(f'Current free memory: {free_memory} bytes')
        await api_client.create_water_tank(name, pressure_sensor, volume_factor, pressure_factor)
        assert free_memory > 500
        await asyncio.sleep(0.1)

    water_tanks = await api_client.get_water_tank_list()
    
    assert water_tanks == [name for name, _ in expected_water_tanks]

    water_sources = await api_client.get_water_source_list()
    
    assert water_sources == [name for name, _ in expected_water_sources]


async def test_create_infinite_ios_by_creating_water_sources_and_tanks(api_client: APIClient):
    """
    Platform should deallocate IOInterface instances when there are no water source
    or water tank using it
    """
    pressure_sensor, volume_factor, pressure_factor = 1, 1.5, 2.5

    pins = [p for p in range(1, 101)]

    free_memory = await api_client.get_free_memory()

    for _ in range(6):
        water_sources = []
        water_tanks = []

        for i, pin in enumerate(pins):
            if i % 2:
                water_source = f'Water source {pin}'
                await api_client.create_water_source(water_source, pin)
                water_sources.append(water_source)
            else:
                water_tank = f'Water Tank {pin}'
                await api_client.create_water_tank(water_tank, pressure_sensor, volume_factor, pressure_factor)
                water_tanks.append(water_tank)

            if len(water_sources) == 10:
                for water_source_name in water_sources[:2]:
                    await api_client.remove_water_source(water_source_name)
                    water_sources.remove(water_source_name)
            
            if len(water_tanks) == 10:
                for water_tank_name in water_tanks[:2]:
                    await api_client.remove_water_tank(water_tank_name)
                    water_tanks.remove(water_tank_name)

            await asyncio.sleep(0.05)

        for water_source_name in water_sources:
            await api_client.remove_water_source(water_source_name)
        
        for water_tank_name in water_tanks:
            await api_client.remove_water_tank(water_tank_name)

    assert abs(await api_client.get_free_memory() - free_memory) <= 2**4


async def test_memory_leak_by_restting_many_times(api_client: APIClient):
    """Platform shuold deallocate everything, to avoid a memory leak"""
    start_free_memory = await api_client.get_free_memory()
    free_memory = start_free_memory

    total = 30
    for i in range(total):
        iteration = i + 1
        free_memory = await api_client.get_free_memory()
        LOGGER.info(f'Starting loop {iteration}/{total}... Current free memory: {free_memory} bytes')

        volume_factor, pressure_factor = 1.5, 2.5
        expected_water_sources = [(f'Water source {i}', i) for i in range(1, 11)]
        expected_water_tanks = [(f'Water tank {i}', i) for i in range(1, 11)]

        for name, pin in expected_water_sources:
            await api_client.create_water_source(name, pin)
            await asyncio.sleep(0.3)
        
        for name, pressure_sensor in expected_water_tanks:
            await api_client.create_water_tank(name, pressure_sensor, volume_factor, pressure_factor)
            await asyncio.sleep(0.3)

        water_tanks = await api_client.get_water_tank_list()
        
        assert water_tanks == [name for name, _ in expected_water_tanks]

        water_sources = await api_client.get_water_source_list()
        
        assert water_sources == [name for name, _ in expected_water_sources]

        await asyncio.sleep(0.3)

        LOGGER.info(f'Resetting...')
        await api_client.reset()

    end_free_memory = await api_client.get_free_memory()

    assert end_free_memory + 100 >= start_free_memory
