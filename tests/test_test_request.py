import pytest

from .lib.api import APIClient
from .lib.api.models import IOSource, IOType
from .lib.api.exceptions import APIException


async def test_create_io(api_client: APIClient):
    """
    Platform should be able to test IO
    Platform should return an error when it receives CreateIO request to an already set pin
    """
    await api_client.create_io(pin=1)

    with pytest.raises(APIException) as exc_info:
        await api_client.create_io(pin=1)

    response = exc_info.value.response
    assert response.exception_type is APIException
    assert response.message == 'TestIO already set with that pin'


async def test_set_and_get_io(api_client: APIClient):
    """Platform should be able to set/get values for TestIO"""
    pin = 2
    value = 20

    await api_client.create_io(pin=pin)

    await api_client.set_io_value(pin=pin, value=value)

    assert await api_client.get_io_value(pin=pin) == value


async def test_set_undefined_pin(api_client: APIClient):
    """
    Platform should be able to answer with an error when setting undefined pin 
    """
    with pytest.raises(APIException) as exc_info:
        await api_client.set_io_value(pin=3, value=20)

    response = exc_info.value.response
    assert response.exception_type is APIException
    assert response.message == 'TestIO with that pin does not exist'


async def test_get_undefined_pin(api_client: APIClient):
    """Platform should be able to answer with an error when getting undefined pin"""
    with pytest.raises(APIException) as exc_info:
        await api_client.get_io_value(pin=4)

    response = exc_info.value.response
    assert response.exception_type is APIException
    assert response.message == 'TestIO with that pin does not exist'


async def test_clear_all_test_ios(api_client: APIClient):
    """Platform should be able to delete all set TestIO's"""
    pin = 5
    value = 5

    await api_client.create_io(pin=pin)
    await api_client.set_io_value(pin=pin, value=value)

    assert await api_client.get_io_value(pin=pin) == 5
    
    await api_client.clear_io()

    with pytest.raises(APIException) as exc_info:
        await api_client.get_io_value(pin=pin)

    response = exc_info.value.response
    assert response.exception_type is APIException
    assert response.message == 'TestIO with that pin does not exist'


async def test_advance_and_get_millis(api_client: APIClient):
    """Platform should be able to mock time for testing"""
    current_time = await api_client.get_millis()

    assert current_time != 0

    offset = 1 * 60 * 60 * 1000 # 1 hour

    await api_client.set_clock_offset(offset)

    assert await api_client.get_millis() >= current_time + offset


async def test_mock_long_overflow(api_client: APIClient):
    """Platform shuold be able to mock a long overflow on millis"""
    MAX_UINT32 = 4294967295

    current_time = await api_client.get_millis()

    offset = MAX_UINT32 - current_time

    await api_client.set_clock_offset(offset)

    assert await api_client.get_millis() < 60 * 1000  # 60 seconds


@pytest.mark.xfail(reason='Need to connect A01 pin')
async def test_set_io_source(api_client: APIClient):
    """Platform should be able to switch between physical and virtual I/O"""
    await api_client.create_io(pin=1, type_=IOType.ANALOGIC)
    
    assert await api_client.get_io_value(pin=1) == 0
    
    await api_client.set_io_source(IOSource.PHYSICAL)

    assert await api_client.get_io_value(pin=1) != 0

    await api_client.set_io_source(IOSource.VIRTUAL)
    
    assert await api_client.get_io_value(pin=1) == 0
