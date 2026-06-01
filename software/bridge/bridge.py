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

import logging.config
import logging.handlers
import json
from pathlib import Path
import atexit
import argparse
import asyncio
from websockets.asyncio.server import serve
from websockets.asyncio.server import ServerConnection
from websockets.exceptions import ConnectionClosed
import signal
from serial_client import SerialClient, serial_client
from serial_client import SerialDeviceAlreadyOpenError, SerialDeviceNotConnectedError, SerialDeviceNotFoundError
from serialx.common import SerialPortInfo
from typing import Union, Literal, Annotated
from pydantic import BaseModel, Field, TypeAdapter, ValidationError
import traceback


logger = logging.getLogger("bridge")


class ResponseBase(BaseModel):
    pass


class ScanResponse(ResponseBase):
    type: Literal["scan"] = "scan"
    devices: list[SerialPortInfo]


class ConnectResponse(ResponseBase):
    type: Literal["connect"] = "connect"


class DisconnectResponse(ResponseBase):
    type: Literal["disconnect"] = "disconnect"


class CommandResponse(ResponseBase):
    type: Literal["command"] = "command"
    response: str


class ErrorResponse(ResponseBase):
    type: Literal["error"] = "error"
    error: Literal["bad_json", "cannot_connect"]
    reason: str


class RequestBase(BaseModel):
    async def process(self) -> str:
        raise NotImplementedError


class ScanRequest(RequestBase):
    type: Literal["scan"]

    async def process(self) -> str:

        devices: list = await SerialClient.get_devices()

        response = ScanResponse(devices=devices)
        return response.model_dump_json()


class ConnectRequest(RequestBase):
    type: Literal["connect"]
    device: str

    async def process(self)-> str:
        response: ResponseBase
        try:
            await serial_client.connect(device_name=self.device)

        except (SerialDeviceNotFoundError, SerialDeviceAlreadyOpenError) as e:
            response = ErrorResponse(error="cannot_connect", reason=str(e))

        else:
            response = ConnectResponse()

        return response.model_dump_json()


class DisconnectRequest(RequestBase):
    type: Literal["disconnect"]

    async def process(self)-> str:
        await serial_client.disconnect()

        response = DisconnectResponse()
        return response.model_dump_json()


class CommandRequest(RequestBase):
    type: Literal["command"]
    command: str

    async def process(self)-> str:
        response: ResponseBase
        try:
            serial_response = await serial_client.send(command=self.command)

        except SerialDeviceNotConnectedError as e:
            response = ErrorResponse(error="cannot_connect", reason=str(e))
        else:
            response = CommandResponse(response=serial_response)

        return response.model_dump_json()


Request = Annotated[
    Union[
        ScanRequest,
        ConnectRequest,
        DisconnectRequest,
        CommandRequest
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
    logger.info("Client connected")

    try:
        async for message in websocket:
            try:
                request = adapter.validate_json(message)

                response = await request.process()
                await websocket.send(response)

            except ValidationError as e:
                response = ErrorResponse(error="bad_json", reason="Invalid json format or invalid schema").model_dump_json()
                await websocket.send(response)

    except ConnectionClosed as e:
        logger.info(f"Connection closed with code={e.code}")
        logger.info(f"Reason={e.reason}")

    except Exception:
        logger.exception("Unexpected error")
        raise

    finally:
        await serial_client.disconnect()

        if active_client is websocket:
            active_client = None
        logger.info(f"Client disconnected")


def parse_args():
    description = ("Provide a network bridge (websocket)\n"
                   "to access stepper motor controller (serial communication) remotly.\n"
                   "Remote user interface (Client) <-> Bridge (Server) <-> stepper controller\n\n"
                   "License: MIT License\n"
                   )
    default_port = 8080

    parser = argparse.ArgumentParser(description=description)
    parser.add_argument("-p", "--port", type=int, nargs='?', const=default_port, default=default_port,
                        help=f"Port of the Websocket server (default is {default_port})")
    return parser.parse_args()


def setup_logging():
    config_file = Path("bridge_logging.json")
    with open(config_file) as f:
        config = json.load(f)
    logging.config.dictConfig(config)
    queue_handler = logging.getHandlerByName("queue_handler")

    # Check that that queue_handler is defined in the config file
    assert isinstance(queue_handler, logging.handlers.QueueHandler)
    assert queue_handler.listener is not None

    # Start the queue listener thread
    queue_handler.listener.start()
    atexit.register(queue_handler.listener.stop)


async def main():
    args = parse_args()
    setup_logging()

    async with serve(handler, "", args.port) as server:
        loop = asyncio.get_running_loop()
        loop.add_signal_handler(signal.SIGTERM, server.close)
        loop.add_signal_handler(signal.SIGINT, server.close)
        await server.wait_closed()


if __name__ == "__main__":
    asyncio.run(main())
