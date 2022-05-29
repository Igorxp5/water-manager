import pytest

from .lib import utils
from .lib.api import APIException

from .test_protobuf.test_pb2 import _TestRequest, _TestResponse


async def test_create_io(api_client):
    """
    Platform should be able to test IO
    Platform should return an error when it receives CreateIO request to an already set pin
    """
    await api_client.create_io(pin=1)

    with pytest.raises(APIException) as exc_info:
        await api_client.create_io(pin=1)

    response = exc_info.value.response
    assert response.error is APIException
    assert response.message == 'TestIO already set with that pin'


async def test_set_and_get_io(api_client):
    """Platform should be able to set/get values for TestIO"""
    pin = 2
    value = 20

    await api_client.create_io(pin=pin)

    await api_client.set_io_value(pin=pin, value=value)

    assert await api_client.get_io_value(pin=pin) == value


async def test_set_undefined_pin(api_client):
    """
    Platform should be able to answer with an error when setting undefined pin 
    """
    with pytest.raises(APIException) as exc_info:
        await api_client.set_io_value(pin=3, value=20)

    response = exc_info.value.response
    assert response.error is APIException
    assert response.message == 'TestIO with that pin does not exist'


async def test_get_undefined_pin(api_client):
    """Platform should be able to answer with an error when getting undefined pin"""
    with pytest.raises(APIException) as exc_info:
        await api_client.get_io_value(pin=4)

    response = exc_info.value.response
    assert response.error is APIException
    assert response.message == 'TestIO with that pin does not exist'


async def test_clear_all_test_ios(api_client):
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
    assert response.error is APIException
    assert response.message == 'TestIO with that pin does not exist'