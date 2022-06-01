import asyncio

import pytest

from .lib.api import APIClient

MAX_UINT32 = 4294967295


@pytest.mark.xfail
@pytest.mark.parametrize('clock_offset', [0, MAX_UINT32])
async def test_auto_fill_until_max_volume(api_client: APIClient, clock_offset: int):
    """Platform should stop to fill a water tank when it reaches at its max volume"""
    raise NotImplementedError


@pytest.mark.xfail
@pytest.mark.parametrize('clock_offset', [0, MAX_UINT32])
async def test_auto_fill_under_min_volume(api_client: APIClient, clock_offset: int):
    """Platform should fill a water tank when it under min volume"""
    raise NotImplementedError


@pytest.mark.xfail
@pytest.mark.parametrize('clock_offset', [0, MAX_UINT32])
async def test_stop_fill_second_water_tank_at_first_min_volume(api_client: APIClient, clock_offset: int):
    """Platform should stop to fill a water tank if its water tank source reaches its min volume"""
    raise NotImplementedError


@pytest.mark.xfail
@pytest.mark.parametrize('clock_offset', [0, MAX_UINT32])
async def test_stop_fill_third_water_tank_at_first_min_volume(api_client: APIClient, clock_offset: int):
    """Platform should stop to fill a water tank if any of water tank source reaches its min volume"""
    raise NotImplementedError


@pytest.mark.xfail
@pytest.mark.parametrize('clock_offset', [0, MAX_UINT32])
async def test_error_water_tank_is_not_filling(api_client: APIClient, clock_offset: int):
    """Platform should send an RuntimeError message when a water tank should be filling and it's not"""
    raise NotImplementedError


@pytest.mark.xfail
@pytest.mark.parametrize('clock_offset', [0, MAX_UINT32])
async def test_error_water_tank_has_stopped_to_filling(api_client: APIClient, clock_offset: int):
    """
    Platform should send an RuntimeError message when a water tank stops to filling 
    before reaches its max volume
    """
    raise NotImplementedError
