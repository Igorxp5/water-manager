import time
import utils

from test_protobuf.test_pb2 import _TestRequest, _TestResponse


def test_create_io(arduino):
    """
    Platform should be able to test IO
    Platform should return an error when it receives CreateIO request to an already set pin
    """
    request = _TestRequest()
    request.id = utils.generate_request_id()
    request.createIO.pin = 1

    payload = request.SerializeToString()
    utils.send_test_message(arduino, payload)
    response = utils.read_response(arduino)

    assert isinstance(response, _TestResponse)
    assert response.error == False
    assert response.id == request.id

    # Second CreateIO
    request.id = utils.generate_request_id()

    payload = request.SerializeToString()
    utils.send_test_message(arduino, payload)

    response = utils.read_response(arduino)

    assert isinstance(response, _TestResponse)
    assert response.error == True
    assert response.id == request.id
    assert response.message.stringValue == 'TestIO already set with that pin'

def test_set_and_get_io(arduino):
    """
    Platform should be able to set/get values for TestIO
    """
    # CreateIO
    request = _TestRequest()
    request.id = utils.generate_request_id()
    request.createIO.pin = 2

    payload = request.SerializeToString()
    utils.send_test_message(arduino, payload)
    response = utils.read_response(arduino)
    assert isinstance(response, _TestResponse)
    assert response.error == False
    assert response.id == request.id

    # SetIOValue
    request = _TestRequest()
    request.id = utils.generate_request_id()
    request.setIOValue.pin = 2
    request.setIOValue.value = 20

    payload = request.SerializeToString()
    utils.send_test_message(arduino, payload)
    response = utils.read_response(arduino)

    assert isinstance(response, _TestResponse)
    assert response.error == False
    assert response.id == request.id
    
    # GetIOValue
    request = _TestRequest()
    request.id = utils.generate_request_id()
    request.getIOValue.pin = 2

    payload = request.SerializeToString()
    utils.send_test_message(arduino, payload)
    response = utils.read_response(arduino)

    assert isinstance(response, _TestResponse)
    assert response.error == False
    assert response.id == request.id
    assert response.message.intValue == 20

def test_set_undefined_pin(arduino):
    """
    Platform should be able to answer with an error when setting undefined pin 
    """
    request = _TestRequest()
    request.id = utils.generate_request_id()
    request.setIOValue.pin = 3
    request.setIOValue.value = 20

    payload = request.SerializeToString()
    utils.send_test_message(arduino, payload)
    response = utils.read_response(arduino)

    assert isinstance(response, _TestResponse)
    assert response.error == True
    assert response.id == request.id
    assert response.message.stringValue == 'TestIO with that pin does not exist'

def test_get_undefined_pin(arduino):
    """
    Platform should be able to answer with an error when getting undefined pin 
    """
    request = _TestRequest()
    request.id = utils.generate_request_id()
    request.getIOValue.pin = 4

    payload = request.SerializeToString()
    utils.send_test_message(arduino, payload)
    response = utils.read_response(arduino)

    assert isinstance(response, _TestResponse)
    assert response.error == True
    assert response.id == request.id
    assert response.message.stringValue == 'TestIO with that pin does not exist'

def test_clear_all_test_ios(arduino):
    """
    Platform should be able to delete all set TestIO's
    """
    utils.set_pin_value(arduino, 5, 5)

    assert utils.get_pin_value(arduino, 5) == 5

    # ClearIOs
    request = _TestRequest()
    request.id = utils.generate_request_id()
    request.clearIOs.SetInParent()

    payload = request.SerializeToString()
    utils.send_test_message(arduino, payload)
    response = utils.read_response(arduino)

    assert isinstance(response, _TestResponse)
    assert response.error == False
    assert response.id == request.id

    # GetIOValue
    request = _TestRequest()
    request.id = utils.generate_request_id()
    request.getIOValue.pin = 5

    payload = request.SerializeToString()
    utils.send_test_message(arduino, payload)
    response = utils.read_response(arduino)

    assert isinstance(response, _TestResponse)
    assert response.error == True
    assert response.id == request.id
    assert response.message.stringValue == 'TestIO with that pin does not exist'
