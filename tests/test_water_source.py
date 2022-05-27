import time

from .lib import utils

from protobuf.out.python.api_pb2 import Request, Response


async def test_create_water_source(api_client):
    """Platform should be able to create a water source"""
    await api_client.create_water_source('Registro da Compesa', 15)
    
    water_sources = await api_client.get_water_source_list()
    
    assert water_sources == ['Registro da Compesa']

# TODO: Max length of water source name

# TODO: Turn off water sources when the program crashes

# TODO: Test memory deallocation when removing a water source

# TODO: Cannot delete a water source with a water tank referencing it
