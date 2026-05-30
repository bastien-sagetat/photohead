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
from websockets.asyncio.server import serve
from websockets.asyncio.server import ServerConnection
from websockets.exceptions import ConnectionClosed
import signal
from serial.tools import list_ports
from serial_client import serial_client
from typing import Union, Literal, Annotated
from pydantic import BaseModel, Field, TypeAdapter, ValidationError

class SerialDevice(BaseModel):
    device: str
    name: str
    description: str
    hwid: str
    vid: int
    pid: int
    serial_number: str

class ScanOkResponse(BaseModel):
    type: Literal["scan"] = "scan"
    status: Literal["ok"] = "ok"
    devices: list[SerialDevice]


class ConnectOkResponse(BaseModel):
    type: Literal["connect"] = "connect"
    status: Literal["ok"] = "ok"


class DisconnectOkResponse(BaseModel):
    type: Literal["disconnect"] = "disconnect"
    status: Literal["ok"] = "ok"


class ErrorResponse(BaseModel):
    type: Literal["scan", "connect", "disconnect",]
    status: Literal["error"] = "error"
    reason: str


class RequestBase(BaseModel):
    async def process(self):
        raise NotImplementedError


class ScanRequest(RequestBase):
    type: Literal["scan"]

    async def process(self) -> str:

        # TODO: Move to serial_client.py and use serialx to get list of devices
        devices: list[SerialDevice] = [
            SerialDevice(
                device=port.device,
                name=port.name,
                description=port.description,
                hwid=port.hwid,
                vid=port.vid,
                pid=port.pid,
                serial_number=port.serial_number,
            ) for port in list_ports.comports() if (port.vid is not None and port.pid is not None and port.serial_number is not None)
        ]

        response = ScanOkResponse(devices=devices)
        return response.model_dump_json()


class ConnectRequest(RequestBase):
    type: Literal["connect"]
    device: str

    async def process(self)-> str:
        await serial_client.connect(device=self.device)

        response = ConnectOkResponse()
        return response.model_dump_json()


class DisconnectRequest(RequestBase):
    type: Literal["disconnect"]

    async def process(self)-> str:
        await serial_client.disconnect()

        response = DisconnectOkResponse()
        return response.model_dump_json()

Request = Annotated[
    Union[
        ScanRequest,
        ConnectRequest,
        DisconnectRequest
    ],
    Field(discriminator="type")
]

adapter: TypeAdapter = TypeAdapter(Request)

active_client = None

async def handler(websocket: ServerConnection):
    """
    Handle a connection.

    """
    global active_client

    if active_client is not None:
        await websocket.close(code=1008, reason="Only one client allowed")
        return

    active_client = websocket
    print("Client connected")

    try:
        async for message in websocket:
            try:
                request = adapter.validate_json(message)

                response = await request.process()
                await websocket.send(response)

            # Invalid JSON OR invalid schema
            except ValidationError as e:
                print(e)
                # TODO: send error response ?

            except Exception as e:
                print(e)
                # TODO: send error response ?

    except ConnectionClosed:
        print(f"Connection closed with code={e.code}")
        print(f"Reason={e.reason}")

    finally:
        if active_client is websocket:
            active_client = None
        print("Client disconnected")
        # TODO: stop stepper & disconnect serial

async def main():
    async with serve(handler, "", 8080) as server:
        loop = asyncio.get_running_loop()
        loop.add_signal_handler(signal.SIGTERM, server.close)
        loop.add_signal_handler(signal.SIGINT, server.close)
        await server.wait_closed()


if __name__ == "__main__":
    asyncio.run(main())
