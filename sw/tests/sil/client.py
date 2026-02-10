import capnp
import asyncio
import socket
import argparse
import time

capnp.remove_import_hook()

async def main(connection):
    rear_light_capnp = capnp.load('tests/sil/rear_light.capnp')
    client = capnp.TwoPartyClient(connection)
    rear_light = client.bootstrap().cast_as(rear_light_capnp.RearLight)

    await rear_light.initialize()

async def cmd_main(host):
    host, port = host.split(":")
    await main(await capnp.AsyncIoStream.create_connection(host=host, port=port))

if __name__ == "__main__":
    asyncio.run(capnp.run(cmd_main('localhost:8000')))
