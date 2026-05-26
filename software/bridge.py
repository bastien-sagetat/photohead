#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Filename: bridge.py
Author: Bastien Sagetat
Description:
    Provide a network bridge (websocket, video streaming over RTSP)
    to access stepper motor controller (pyserial) and camera (gphoto2) remotly.
    GUI (Client) <-> Bridge (Server) <-> stepper controller / camera

License: MIT License
"""


import asyncio
import json
from websockets.asyncio.server import serve
from websockets.asyncio.server import ServerConnection
import signal
from serial.tools import list_ports
import serialx
from dataclasses import dataclass

@dataclass
class Request:
    frame: bytes
    future: asyncio.Future


class SerialClient:
    def __init__(self, port, baudrate=115200, timeout=0.1):
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout

        self.queue = asyncio.Queue()
        self.reader = None
        self.writer = None

        self.worker_task = None

    async def connect(self):
        self.reader, self.writer = await serialx.open_serial_connection(self.port, baudrate=115200)
        self.worker_task = asyncio.create_task(self.worker())

    async def worker(self):
        try:
            while True:
                req = await self.queue.get()

                try:
                    self.writer.write(req.frame)
                    await self.writer.drain()

                    response = await asyncio.wait_for(self.reader.readuntil(b'\n'), timeout=self.timeout)

                    if not req.future.done():
                        req.future.set_result(response)

                except asyncio.TimeoutError as e:
                    # flush input buffer to remove potential incomplete frame
                    try:
                        self.writer.transport.serial.reset_input_buffer()
                    except Exception:
                        pass

                    if not req.future.done():
                        req.future.set_exception(e)


                except Exception as e:
                    if not req.future.done():
                        req.future.set_exception(e)

                finally:
                    self.queue.task_done()

        except asyncio.CancelledError:
            raise

    async def close(self):
        # stop worker
        if self.worker_task:
            self.worker_task.cancel()
            try:
                await self.worker_task
            except asyncio.CancelledError:
                pass

        # close serial connection
        if self.writer:
            try:
                self.writer.close()
                await self.writer.wait_closed()
            except Exception:
                pass

        self.reader = None
        self.writer = None

    async def __aenter__(self):
        await self.connect()
        return self

    async def __aexit__(self, exc_type, exc, tb):
        await self.close()


# client = SerialClient()
# await client.connect()

# # use client.queue ...

# await client.close()

# OR

# async with SerialClient() as client:
#     # use client.queue ...
#     pass

async def error(websocket: ServerConnection, message: str):
    """
    Send an error message.

    """
    event: dict[str, str] = {
        "type": "error",
        "message": message,
    }

    await websocket.send(json.dumps(event))


async def scan(websocket: ServerConnection):
    """
    Send list of available serial ports.

    """
    ports: list[dict[str, str | int]] = [
        {
            "device": port.device,
            "name": port.name,
            "description": port.description,
            "hwid": port.hwid,
            "vid": port.vid,
            "pid": port.pid,
            "serial_number": port.serial_number,
        }
        for port in list_ports.comports() if (port.vid and port.pid and port.serial_number)
    ]

    event: dict[str, str | list] = {
        "type": "scan",
        "ports": ports,
    }

    await websocket.send(json.dumps(event))



async def handler(websocket: ServerConnection):
    """
    Handle a connection.

    """
    async for message in websocket:
        try:
            event = json.loads(message)
        except json.JSONDecodeError:
            await error(websocket, "Invalid JSON message")
            continue

        event_type = event.get("type")

        if event_type == "scan":
            await scan(websocket)
        else:
            await error(websocket, "Unknown message type")


async def main():
    async with serve(handler, "", 8080) as server:
        loop = asyncio.get_running_loop()
        loop.add_signal_handler(signal.SIGTERM, server.close)
        loop.add_signal_handler(signal.SIGINT, server.close)
        await server.wait_closed()


if __name__ == "__main__":
    asyncio.run(main())
