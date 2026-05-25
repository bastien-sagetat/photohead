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


async def error(websocket: ServerConnection, message: str):
    """
    Send an error message.

    """
    event: dict[str, str] = {
        "type": "error",
        "message": message,
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
            pass
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
