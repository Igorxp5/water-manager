import os
import sys
import pytest
import logging
import subprocess

from _pytest.runner import runtestprotocol

PROJECT_ROOT = os.path.join(os.path.dirname(__file__), '..')
PIO_EXECUTABLE = os.environ.get('PIO_EXECUTABLE', 'pio')
ARDUINO_PORT = os.environ.get('ARDUINO_PORT')
LOGGER = logging.getLogger(__name__)

os.environ['PYTHONUNBUFFERED'] = '1'
sys.path.append(PROJECT_ROOT)  # Add project to PYTHONPATH

from .lib.api import APIClient
from .lib.api.arduino import ArduinoConnection


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

@pytest.fixture(autouse=True)
async def check_memory_leak(api_client):
    await api_client.reset()
    await api_client.clear_io()
    free_memory = await api_client.get_free_memory()
    LOGGER.debug(f'Free memory before starting the test: {free_memory} bytes!')
    yield
    free_memory = await api_client.get_free_memory()
    LOGGER.debug(f'Free memory after finishing the test: {free_memory} bytes!')
    await api_client.reset()
    await api_client.clear_io()


@pytest.fixture(scope='session')
def arduino_connection():
    yield ArduinoConnection(port=ARDUINO_PORT)


@pytest.fixture
def api_client(arduino_connection, event_loop):
    client = APIClient(arduino_connection, event_loop=event_loop)
    yield client
    client.close()
