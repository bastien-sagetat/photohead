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
from websockets.exceptions import ConnectionClosed
import signal
from serial.tools import list_ports
import serialx

class SerialClient:
    def __init__(self, port, baudrate=115200, timeout=0.1):
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout

        self.reader = None
        self.writer = None

    async def connect(self):
        self.reader, self.writer = await serialx.open_serial_connection(self.port, baudrate=115200)
    
    async def send(self, command: str) -> str:
        try:
            self.writer.write(req.frame)
            await self.writer.drain()
            response: bytes = await asyncio.wait_for(self.reader.readuntil(b'\n'), timeout=self.timeout)
            return response.decode("utf-8", errors='replace').strip()
    
        except asyncio.TimeoutError:
            try:
                self.writer.transport.serial.reset_input_buffer()
            except Exception:
                pass
            raise
            
    async def disconnect(self):
        if self.writer:
            try:
                self.writer.close()
                await self.writer.wait_closed()
            except Exception:
                pass

        self.reader = None
        self.writer = None

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
    try:
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
    except ConnectionClosed:
        print("Client disconnected")

    finally:
        print("Cleanup")
        # TODO: stop stepper & disconnect serial

async def main():
    async with serve(handler, "", 8080) as server:
        loop = asyncio.get_running_loop()
        loop.add_signal_handler(signal.SIGTERM, server.close)
        loop.add_signal_handler(signal.SIGINT, server.close)
        await server.wait_closed()


if __name__ == "__main__":
    asyncio.run(main())
