import os
import sys
import time
import serial
import pytest
import subprocess

from _pytest.runner import runtestprotocol
from serial.tools.list_ports import comports

PROJECT_ROOT = os.path.join(os.path.dirname(__file__), '..')
READ_TIMEOUT = 5

os.environ['PYTHONUNBUFFERED'] = '1'
sys.path.append(PROJECT_ROOT)  # Add project to PYTHONPATH

class ArduinoNotFound(RuntimeError):
    pass


class ArduinoConnection:
    def __init__(self, port=None, baudrate=9600):
        try:
            self.port = port or ArduinoConnection.available_comports()[0]
            self.baudrate = baudrate
        except IndexError:
            raise ArduinoNotFound

    def __enter__(self, *args, **kwargs):
        self.conn = serial.Serial(port=self.port, baudrate=self.baudrate, timeout=READ_TIMEOUT)
        if not self.conn.is_open:
            print('Openning connection to Arduino...')
            self.conn.open()
        time.sleep(3)
        return self.conn
    
    def __exit__(self, *args, **kwargs):
        self.conn.close()

    @staticmethod
    def available_comports():
        return [com.name for com in comports()]


def pytest_runtest_protocol(item, nextitem):
    reports = runtestprotocol(item, nextitem=nextitem)
    for report in reports:
        if report.when == 'call':
            print('\n%s --- %s' % (item.name, report.outcome))
    return True


@pytest.fixture(scope='session')
def arduino():
    process = subprocess.Popen(['pio', 'run', '-t', 'upload', '-e', 'test'], cwd=PROJECT_ROOT)
    process.wait()
    
    assert process.returncode == 0, 'Failed to upload test environment to Arduino'

    with ArduinoConnection() as arduino_connection:
        yield arduino_connection
