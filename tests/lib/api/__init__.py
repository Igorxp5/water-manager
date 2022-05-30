import struct
import asyncio
import logging
import itertools
import collections

from typing import Dict

from protobuf.out.python.api_pb2 import Request, Response

from .models import OperationMode, IOType
from .response import APIResponse
from .volatile_queue import VolatileQueue
from ...test_protobuf.test_pb2 import _TestRequest, _TestResponse

PACKET_FORMAT = '<BH'
LOGGER = logging.getLogger(__name__)


FutureResponse = collections.namedtuple('FutureResponse', ['future', 'response_type'])


class APIClient:
    REQUEST_ID_ITERATOR = itertools.cycle(range(1, 65535))
    DEFAULT_REQUEST_TIMEOUT = 7
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

    def __del__(self):
        self.close()

    def create_water_source(self, name: str, pin: int, water_tank_name: str = None):
        return self.send_request('createWaterSource', name=name, pin=pin, waterTankName=water_tank_name)

    def remove_water_source(self, name: str):
        return self.send_request('removeWaterSource', waterSourceName=name)

    def get_water_source_list(self):
        return self.send_request('getWaterSourceList', response_type=list)
    
    def get_water_source(self, name: str):
        return self.send_request('getWaterSource', waterSourceName=name)

    def set_operation_mode(self, mode: OperationMode):
        return self.send_request('setMode', mode=mode.value)
    
    def get_operation_mode(self):
        def _operation_mode_factory(value=0):
            return OperationMode(value) if value else OperationMode.MANUAL
        return self.send_request('getMode', response_type=_operation_mode_factory)

    def create_io(self, pin: int, type_: IOType=IOType.DIGITAL):
        return self.send_request('createIO', pin=pin, type=type_.value, request_class=_TestRequest)

    def set_io_value(self, pin: int, value: int):
        return self.send_request('setIOValue', pin=pin, value=value, request_class=_TestRequest)

    def get_io_value(self, pin: int):
        return self.send_request('getIOValue', pin=pin, request_class=_TestRequest, response_type=int)

    def clear_io(self):
        return self.send_request('clearIOs', request_class=_TestRequest)

    def get_free_memory(self):
        return self.send_request('freeMemory', request_class=_TestRequest, response_type=int)

    def reset(self):
        return self.send_request('resetAPI', request_class=_TestRequest)
    
    def set_timeout(self, timeout):
        self._timeout = timeout

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

    async def get_error_response(self):
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
                    if not response.error:
                        message = response.message
                        if response_type:
                            message = response_type(message) if message else response_type()
                        future.set_result(message)
                    else:
                        exc = response.error(response.message, response)
                        future.set_exception(exc)
                elif response.error is None:
                    LOGGER.warning('Got Response without mapped request!')
                    LOGGER.warning(f'Response message: {response.message}')
                    LOGGER.warning(f'Response exception: {response.error.__class__.__name__}')
                else:
                    await self._unmapped_error_responses.put(response)

    async def _request_timeout_routine(self, request_id, timeout):
        future, _ = self._future_responses[request_id]
        try:
            await asyncio.wait_for(future, timeout)
        except asyncio.TimeoutError:
            future.cancel()
        except asyncio.CancelledError:
            pass
        if request_id in self._future_responses:
            del self._future_responses[request_id]

    async def send_request(self, command, request_id=None, response_type=None, **params):
        request = self.create_request(command=command, request_id=request_id, **params)
        payload = self.build_request_wrapper(request)
        future = await self.send_payload(payload, request.id, response_type=response_type)
        return await future

    async def send_payload(self, payload, request_id=None, response_type=None):
        _, writer = await self._open_stream_task
        writer.write(payload)
        await writer.drain()
        future = asyncio.Future()
        self._future_responses[request_id] = FutureResponse(future, response_type)
        if self._timeout:
            timeout_routine = self._event_loop.create_task(self._request_timeout_routine(request_id, self._timeout))
            self._timeout_tasks.append(timeout_routine)
        return future

    @staticmethod
    def build_request_wrapper(request):
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
