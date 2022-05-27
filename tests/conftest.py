import os
import sys
import pytest
import logging
import subprocess

from _pytest.runner import runtestprotocol

PROJECT_ROOT = os.path.join(os.path.dirname(__file__), '..')
PIO_EXECUTABLE = os.environ.get('PIO_EXECUTABLE', 'pio')
LOGGER = logging.getLogger(__name__)

os.environ['PYTHONUNBUFFERED'] = '1'
sys.path.append(PROJECT_ROOT)  # Add project to PYTHONPATH

from .lib import utils
from .lib.api import ArduinoConnection, APIClient


def pytest_runtest_protocol(item, nextitem):
    reports = runtestprotocol(item, nextitem=nextitem)
    for report in reports:
        if report.when == 'call':
            print('\n%s --- %s' % (item.name, report.outcome))
    return True


@pytest.fixture(scope='session', autouse=True)
def upload_test_environment():
    process = subprocess.Popen([PIO_EXECUTABLE, 'run', '-t', 'upload', '-e', 'test'], cwd=PROJECT_ROOT)
    process.wait()

    assert process.returncode == 0, 'Failed to upload test environment to Arduino'

@pytest.fixture(autouse=True)
async def check_memory_leak(api_client):
    before_free_memory = await api_client.get_memory_free()
    LOGGER.debug(f'Free memory before starting the test: {before_free_memory} bytes!')
    yield
    await api_client.reset()
    after_free_memory = await api_client.get_memory_free()
    LOGGER.debug(f'Free memory after finishing the test: {after_free_memory} bytes!')
    if before_free_memory > after_free_memory:
        LOGGER.warning(f'Possible memory leak found: {before_free_memory} bytes -> {after_free_memory} bytes!')


@pytest.fixture(scope='session')
def arduino_connection():
    yield ArduinoConnection()


@pytest.fixture
def api_client(arduino_connection, event_loop):
    client = APIClient(arduino_connection, event_loop=event_loop)
    yield client
    client.close()
