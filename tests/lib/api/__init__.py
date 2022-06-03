import sys
import struct
import asyncio
import logging
import pathlib
import itertools
import importlib
import collections

from typing import Dict

try:
    from protobuf.out.python.api_pb2 import Request, Response
except ImportError:
    # This module was done just for testing on Pytest and it cannot be import outside.
    # FIXME: Handling for GUI lib
    module_path = pathlib.Path(__file__).parent.parent.parent.parent / 'protobuf' / 'out' / 'python' / 'api_pb2.py'
    module_name = 'api_pb2'
    spec = importlib.util.spec_from_file_location(module_name, module_path)
    api_pb2 = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = api_pb2 
    spec.loader.exec_module(api_pb2)

    from api_pb2 import Request, Response


from .models import OperationMode, IOType, IOSource
from .response import APIResponse, APIErrorResponse
from .exceptions import APIException
from .volatile_queue import VolatileQueue

try:
    from ...test_protobuf.test_pb2 import _TestRequest, _TestResponse
except ImportError:
    # This module was done just for testing on Pytest and it cannot be import outside.
    # FIXME: Handling for GUI lib
    module_path = pathlib.Path(__file__).parent.parent.parent / 'test_protobuf' / 'test_pb2.py'
    module_name = 'test_pb2'
    spec = importlib.util.spec_from_file_location(module_name, module_path)
    test_pb2 = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = test_pb2 
    spec.loader.exec_module(test_pb2)

    from test_pb2 import _TestRequest, _TestResponse


PACKET_FORMAT = '<BH'
LOGGER = logging.getLogger(__name__)


FutureResponse = collections.namedtuple('FutureResponse', ['future', 'response_type'])


class APIClient:
    REQUEST_ID_ITERATOR = itertools.cycle(range(1, 65535))
    DEFAULT_REQUEST_TIMEOUT = 7
    FUTURE_ALLOCATE_TIMEOUT = 100
    REQUEST_MESSAGE_TYPES = {Request: 1, _TestRequest: 2} 

    def __init__(self, arduino_connection, event_loop: asyncio.ProactorEventLoop = None, timeout=DEFAULT_REQUEST_TIMEOUT):
        self._arduino_connection = arduino_connection
        self._future_responses: Dict[FutureResponse] = dict()
        self._event_loop = event_loop or asyncio.get_event_loop()

        self._read_responses_task = self._event_loop.create_task(self._read_responses_routine())
        self._open_stream_task = self._event_loop.create_task(self._arduino_connection.open())

        self._timeout = timeout
        self._timeout_tasks = []

        self._unmapped_error_responses = VolatileQueue()

        self._clock_offset = 0

    def __del__(self):
        self.close()

    def create_water_source(self, name: str, pin: int, water_tank_name: str = None, return_exceptions=False):
        return self.send_request('createWaterSource', name=name, pin=pin, waterTankName=water_tank_name, return_exceptions=return_exceptions)

    def get_water_source(self, name: str, return_exceptions=False) -> dict:
        return self.send_request('getWaterSource', waterSourceName=name, return_exceptions=return_exceptions)

    def set_water_source_state(self, name: str, enabled: bool, force: bool=False, return_exceptions=False):
        return self.send_request('setWaterSourceState', waterSourceName=name, state=enabled, force=force, return_exceptions=return_exceptions)

    def set_water_source_active(self, name: str, active: bool, return_exceptions=False):
        return self.send_request('setWaterSourceActive', waterSourceName=name, active=active, return_exceptions=return_exceptions)

    def remove_water_source(self, name: str, return_exceptions=False):
        return self.send_request('removeWaterSource', waterSourceName=name, return_exceptions=return_exceptions)

    def get_water_source_list(self, return_exceptions=False) -> list:
        return self.send_request('getWaterSourceList', response_type=list, return_exceptions=return_exceptions)

    def create_water_tank(self, name: str, pressure_sensor_pin: int, volume_factor: float, pressure_factor: float, water_source_name: str = None, return_exceptions=False):
        return self.send_request('createWaterTank', name=name, pressureSensorPin=pressure_sensor_pin, volumeFactor=volume_factor,
                                 pressureFactor=pressure_factor, waterSourceName=water_source_name, return_exceptions=return_exceptions)

    def remove_water_tank(self, name: str, return_exceptions=False):
        return self.send_request('removeWaterTank', waterTankName=name, return_exceptions=return_exceptions)
    
    def set_water_tank_minimum_volume(self, name: str, value: float, return_exceptions=False):
        return self.send_request('setWaterTankMinimumVolume', waterTankName=name, value=value, return_exceptions=return_exceptions)
    
    def set_water_tank_max_volume(self, name: str, value: float, return_exceptions=False):
        return self.send_request('setWaterTankMaxVolume', waterTankName=name, value=value, return_exceptions=return_exceptions)
    
    def set_water_tank_zero_volume_pressure(self, name: str, value: float, return_exceptions=False):
        return self.send_request('setWaterTankZeroVolume', waterTankName=name, value=value, return_exceptions=return_exceptions)

    def set_water_tank_volume_factor(self, name: str, value: float, return_exceptions=False):
        return self.send_request('setWaterTankVolumeFactor', waterTankName=name, value=value, return_exceptions=return_exceptions)
    
    def set_water_tank_pressure_factor(self, name: str, value: float, return_exceptions=False):
        return self.send_request('setWaterTankPressureFactor', waterTankName=name, value=value, return_exceptions=return_exceptions)

    def set_water_tank_pressure_changing_value(self, name: str, value: float, return_exceptions=False):
        return self.send_request('setWaterTankPressureChangingValue', waterTankName=name, value=value, return_exceptions=return_exceptions)

    def get_water_tank_list(self, return_exceptions=False) -> list:
        return self.send_request('getWaterTankList', response_type=list, return_exceptions=return_exceptions)
    
    def get_water_tank(self, name: str, return_exceptions=False) -> dict:
        return self.send_request('getWaterTank', waterTankName=name, return_exceptions=return_exceptions)

    def fill_water_tank(self, name: str, enabled: bool, force: bool=False, return_exceptions=False):
        return self.send_request('fillWaterTank', waterTankName=name, enabled=enabled, force=force, return_exceptions=return_exceptions)

    def set_water_tank_active(self, name: str, active: bool, return_exceptions=False):
        return self.send_request('setWaterTankActive', waterTankName=name, active=active, return_exceptions=return_exceptions)

    def set_operation_mode(self, mode: OperationMode, return_exceptions=False):
        return self.send_request('setMode', mode=mode.value, return_exceptions=return_exceptions)
    
    def get_operation_mode(self, return_exceptions=False) -> OperationMode:
        def _operation_mode_factory(value=0):
            return OperationMode(value) if value else OperationMode.MANUAL
        return self.send_request('getMode', response_type=_operation_mode_factory, return_exceptions=return_exceptions)

    def reset(self, return_exceptions=False):
        return self.send_request('reset', return_exceptions=return_exceptions)

    def create_io(self, pin: int, type_: IOType=IOType.DIGITAL, return_exceptions=False):
        return self.send_request('createIO', pin=pin, type=type_.value, request_class=_TestRequest, return_exceptions=return_exceptions)

    def set_io_value(self, pin: int, value: int, return_exceptions=False):
        return self.send_request('setIOValue', pin=pin, value=value, request_class=_TestRequest, return_exceptions=return_exceptions)

    def get_io_value(self, pin: int, return_exceptions=False) -> int:
        return self.send_request('getIOValue', pin=pin, request_class=_TestRequest, response_type=int, return_exceptions=return_exceptions)
    
    def set_io_source(self, source: IOSource, return_exceptions=False):
        return self.send_request('setIOSource', source=source, request_class=_TestRequest, return_exceptions=return_exceptions)

    def clear_io(self, return_exceptions=False):
        return self.send_request('clearIOs', request_class=_TestRequest, return_exceptions=return_exceptions)

    def set_clock_offset(self, value: int, return_exceptions=False):
        return self.send_request('setClockOffset', value=value, request_class=_TestRequest, return_exceptions=return_exceptions)
    
    def get_millis(self, return_exceptions=False) -> int:
        return self.send_request('getMillis', request_class=_TestRequest, response_type=int, return_exceptions=return_exceptions)

    def get_free_memory(self, return_exceptions=False) -> int:
        return self.send_request('freeMemory', request_class=_TestRequest, response_type=int, return_exceptions=return_exceptions)

    def reset_clock(self, return_exceptions=False):
        self._clock_offset = 0
        return self.send_request('resetClock', request_class=_TestRequest, return_exceptions=return_exceptions)

    def set_timeout(self, timeout):
        self._timeout = timeout

    async def advance_clock(self, seconds: int):
        self._clock_offset += seconds * 1000
        await self.set_clock_offset(self._clock_offset)

    def close(self):
        tasks = itertools.chain(self._timeout_tasks, (self._read_responses_task,))
        for task in tasks:
            if not task.done():
                task.cancel()
        for future, _ in self._future_responses.values():
            if not future.done():
                future.cancel()
        if not self._event_loop.is_closed():
            self._event_loop.run_until_complete(self._arduino_connection.close())

    async def get_error_response(self) -> APIResponse:
        response = await self._unmapped_error_responses.get()
        self._unmapped_error_responses.task_done()
        return response

    async def _read_responses_routine(self):
        while True:
            raw_response = await self.read_response()
            if raw_response:
                response = APIResponse.parse(raw_response)
                future_response = self._future_responses.get(response.id)
                if future_response is not None:
                    future, response_type = future_response
                    if not isinstance(response, APIErrorResponse):
                        message = response.message
                        if response_type:
                            message = response_type(message) if message else response_type()
                        future.set_result(message)
                    else:
                        exc = response.exception_type(response.message, response.arg, response)
                        future.set_exception(exc)
                elif not isinstance(response, APIErrorResponse):
                    LOGGER.warning('Got Response without mapped request!')
                    LOGGER.warning(f'Response message: {response.message}')
                else:
                    await self._unmapped_error_responses.put(response)

    async def _request_timeout_routine(self, request_id, timeout):
        future, _ = self._future_responses[request_id]
        try:
            await asyncio.wait_for(asyncio.shield(future), timeout)
        except asyncio.TimeoutError:
            if request_id in self._future_responses:
                del self._future_responses[request_id]
        except asyncio.CancelledError:
            pass
        except APIException:
            pass

    async def send_request(self, command, request_id=None, response_type=None, return_exceptions=False, **params):
        request = self.create_request(command=command, request_id=request_id, **params)
        payload = self.build_request_wrapper(request)
        future = await self.send_payload(payload, request.id, response_type=response_type)
        gather = asyncio.gather(future, return_exceptions=return_exceptions)
        try:
            results = await asyncio.wait_for(gather, timeout=self._timeout)
            return results[0]
        except asyncio.TimeoutError:
            if request_id in self._future_responses:
                del self._future_responses[request_id]
            raise

    async def send_payload(self, payload, request_id=None, response_type=None) -> asyncio.Future:
        _, writer = await self._open_stream_task
        writer.write(payload)
        await writer.drain()
        future = asyncio.Future()
        self._future_responses[request_id] = FutureResponse(future, response_type)
        timeout_routine = self._event_loop.create_task(self._request_timeout_routine(request_id, self.FUTURE_ALLOCATE_TIMEOUT))
        self._timeout_tasks.append(timeout_routine)
        return future

    @staticmethod
    def build_request_wrapper(request) -> bytes:
        message = request
        if isinstance(request, Request) or isinstance(request, _TestRequest):
            message = request.SerializeToString()
        message_type = APIClient.REQUEST_MESSAGE_TYPES.get(request.__class__, 1)
        return struct.pack(PACKET_FORMAT, message_type, len(message)) + message

    @staticmethod
    def create_request(command, request_class=Request, request_id=None, **params):
        request = request_class()
        request.id = request_id or next(APIClient.REQUEST_ID_ITERATOR)
        request_command = getattr(request, command)
        request_command.SetInParent()
        for key, value in params.items():
            # filter optional params (None value)
            if value is not None:
                setattr(request_command, key, value)
        return request
    
    async def read_response(self):
        reader, _ = await self._open_stream_task
        message_type = struct.unpack('B', await reader.readexactly(1))
        message_type = message_type[0]

        if message_type == 3:
            data = await APIClient.stream_read_until(reader)
            LOGGER.debug(data.decode())
            return

        message_length = struct.unpack('<H', await reader.readexactly(2))[0]
        raw = await reader.readexactly(message_length)

        response = None
        if message_type == 1:
            response = Response()
            response.ParseFromString(raw)
        elif message_type == 2:
            response = _TestResponse()
            response.ParseFromString(raw)
        return response
    
    @staticmethod
    async def stream_read_until(stream_reader, seperator=b'\n'):
        separator_buffer = b'\00' * len(seperator)
        buffer = b''
        try:
            while separator_buffer != seperator:
                byte = await stream_reader.readexactly(1)
                buffer += byte
                separator_buffer = separator_buffer[1:] + byte
        except asyncio.asyncio.IncompleteReadError:
            pass
        return buffer
