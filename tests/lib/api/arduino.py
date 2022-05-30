import asyncio

from serial.tools.list_ports import comports
from serial_asyncio import open_serial_connection


class ArduinoNotFound(RuntimeError):
    pass


class ArduinoConnection:
    def __init__(self, port: int = None, baudrate: int = 9600):
        try:
            self.port = port or ArduinoConnection.available_comports()[0]
        except IndexError:
            raise ArduinoNotFound
        self.baudrate = baudrate
        self.transport = None

    async def open(self, loop=None):
        if not self.transport:
            self.transport = await open_serial_connection(url=self.port, baudrate=self.baudrate, loop=loop)
            await asyncio.sleep(3)
        reader, writer = self.transport
        return reader, writer

    async def close(self):
        if self.transport and not self.transport[1].is_closing():
            self.transport[1].close()
            await self.transport[1].wait_closed()
        self.transport = None

    @staticmethod
    def available_comports():
        return [com.name for com in comports()]
