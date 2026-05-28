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
from serial_client import serial_client

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

async def connect(websocket: ServerConnection):
    """
    Connect to serial port

    """
    serial_client.connect()

async def disconnect(websocket: ServerConnection):
    """
    Disconnect from serial port

    """
    serial_client.disconnect()
    
async def send(websocket: ServerConnection):
    """
    Send command to serial

    """
    try:
        serial_client.send()
    except timeout as e:

        
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
            else if event_type == "connect":
                await connect(websocket)
            else if event_type == "disconnect":
                await disconnect(websocket)
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
