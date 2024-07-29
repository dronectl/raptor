import time
import os
import asyncio
import serial
import logging
import influxdb_client
from influxdb_client import Point
from asyncio import Queue, QueueEmpty, QueueFull
from dataclasses import dataclass
from influxdb_client.client.write_api import SYNCHRONOUS

logging.basicConfig(level=logging.INFO)
_logger = logging.getLogger(__name__)

@dataclass
class Telemetry:
    timestamp:int # ns
    voltage:float

class Config:
    INFLUX_BUCKET = os.environ['INFLUX_BUCKET']
    INFLUX_ORG = os.environ['INFLUX_ORG']
    INFLUX_TOKEN = os.environ['INFLUX_TOKEN']
    INFLUX_URL = os.environ['INFLUX_URL']
    PORT= os.environ['PORT']
    BAUDRATE=int(os.environ['BAUDRATE'])

async def producer(dqueue: Queue, port:str, baudrate:int) -> None:
    ser = serial.Serial(port=port, baudrate=baudrate, timeout=1)
    t = time.time_ns()
    qf_stat = False
    while True:
        if ser.in_waiting > 0:
            data = ser.readline().decode('utf-8').strip()
            try:
                voltages = list(map(float, data.split(',')))
            except BaseException:
                continue
            _logger.info("%s", voltages)
            try:
                for voltage in voltages:
                    t+= 1 * 1000 * 1000
                    await dqueue.put(Telemetry(t, voltage))
                qf_stat = False
            except QueueFull:
                if not qf_stat:
                    qf_stat = True 
                    _logger.warning("Queue Full!")
                pass
            await asyncio.sleep(0)

async def consumer(dqueue: Queue) -> None:
    points: list[Point] = []
    while True:
        telemetry = await dqueue.get()
        points.append(Point("adc-dma").field("voltage", telemetry.voltage).time(telemetry.timestamp))
        if len(points) >= 50:
            #write_api.write(bucket=Config.INFLUX_BUCKET, org=Config.INFLUX_ORG, record=points, write_precision="ns")
            points.clear()
        dqueue.task_done()

async def main():
    dqueue = Queue[Telemetry](maxsize=1000)
    await asyncio.gather(consumer(dqueue), producer(dqueue, Config.PORT, Config.BAUDRATE))

if __name__ == "__main__":
    _logger.info("INFLUX ORG: %s", Config.INFLUX_ORG)
    _logger.info("INFLUX BUCKET: %s", Config.INFLUX_BUCKET)
    _logger.info("INFLUX URL: %s", Config.INFLUX_URL)
    client = influxdb_client.InfluxDBClient(url=Config.INFLUX_URL, token=Config.INFLUX_TOKEN, org=Config.INFLUX_ORG)
    write_api = client.write_api(write_options=SYNCHRONOUS)
    asyncio.run(main())