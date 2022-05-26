import os
import sys
import time
import serial
import pytest
import logging
import subprocess

from _pytest.runner import runtestprotocol
from serial.tools.list_ports import comports

PROJECT_ROOT = os.path.join(os.path.dirname(__file__), '..')
PIO_EXECUTABLE = os.environ.get('PIO_EXECUTABLE', 'pio')
READ_TIMEOUT = 5
LOGGER = logging.getLogger(__name__)

os.environ['PYTHONUNBUFFERED'] = '1'
sys.path.append(PROJECT_ROOT)  # Add project to PYTHONPATH

import utils


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
def upload_test_environment():
    process = subprocess.Popen([PIO_EXECUTABLE, 'run', '-t', 'upload', '-e', 'test'], cwd=PROJECT_ROOT)
    process.wait()

    assert process.returncode == 0, 'Failed to upload test environment to Arduino'


@pytest.fixture(scope='session')
def arduino_connection():
    with ArduinoConnection() as arduino_connection:
        yield arduino_connection


@pytest.fixture
def arduino(upload_test_environment, arduino_connection):
    before_free_memory = utils.current_free_memory(arduino_connection)
    LOGGER.info(f'Free memory before starting the test: {before_free_memory} bytes!')
    yield arduino_connection
    utils.reset_api(arduino_connection)
    after_free_memory = utils.current_free_memory(arduino_connection)
    LOGGER.info(f'Free memory before starting the test: {after_free_memory} bytes!')
    if after_free_memory > before_free_memory:
        LOGGER.warning(f'Possible memory leak found: {before_free_memory} bytes -> {after_free_memory} bytes!')
