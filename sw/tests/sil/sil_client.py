import capnp
import asyncio
import socket
import argparse
import time
#import rotor_sensor_capnp
capnp.remove_import_hook()
rotor_sensor_capnp = capnp.load('tests/sil/rotor_sensor.capnp')

def parse_args():
    parser = argparse.ArgumentParser(
        usage="Connects to the Calculator server at the given address and does some RPCs"
    )
    parser.add_argument("host", help="HOST:PORT")

    return parser.parse_args()

async def main(connection):
    client = capnp.TwoPartyClient(connection)

    rotor_sensor = client.bootstrap().cast_as(rotor_sensor_capnp.RotorSensor)

    samples = []
    with open('resampled.txt', 'r') as f:
        lines = f.readlines()
        for line in lines:
            (timestamp, period) = line.split(";")
            samples.append((int(timestamp), int(period)))


    start = time.time()
    await rotor_sensor.initialize()

    #simulation_time = 120 * 1000  # 120 seconds in milliseconds

    last_timestamp = 0
    for sample in samples:
        timestamp, period = sample

        if timestamp > last_timestamp:
            await rotor_sensor.onTick(timestamp - last_timestamp)
            last_timestamp = timestamp

        await rotor_sensor.queueSpeedData(0, int(period / 2), int(period))

    # sample_index = 0
    # for i in range(simulation_time):
    #     await rotor_sensor.onTick(1)
    #     #await rotor_sensor.setPressure(101000)
    #     #altitude = (await rotor_sensor.getAltitude()).altitude
    #     #print(f"Altitude: {altitude}")

    #     if i == samples[sample_index][0]:
    #         period = samples[sample_index][1]
    #         await rotor_sensor.queueSpeedData(0, int(period / 2), int(period))
    #         sample_index += 1

    global_speed = (await rotor_sensor.getGlobalSpeed()).speed
    print(f"GlobalSpeed: {global_speed}")
    top_speed = (await rotor_sensor.getTopSpeed()).speed
    print(f"TopSpeed: {top_speed}")
    end = time.time()
    print(f"Time taken: {end - start} seconds")
    print(f"Simulation time: {simulation_time / 1000} seconds")

async def cmd_main(host):
    host, port = host.split(":")
    await main(await capnp.AsyncIoStream.create_connection(host=host, port=port))


if __name__ == "__main__":
    asyncio.run(capnp.run(cmd_main('localhost:6969')))