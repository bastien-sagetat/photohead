# -*- coding: utf-8 -*-
"""
Filename: serial_client.py
Author: Bastien Sagetat
Description:
    SerialClient class implementation.
    Create an instance of the class in order to use it as a singleton.

License: MIT License
"""

import asyncio
import serialx
from pydantic import BaseModel

class SerialDevice(BaseModel):
    device: str
    name: str
    vid: int
    pid: int
    serial_number: str


class SerialClient:
    def __init__(self):
        self.device = None
        self.baudrate = None
        self.timeout = None

        self.reader = None
        self.writer = None
        # TODO: self.connected = True/False

    async def connect(self, device: str, baudrate: int=115200, timeout: float=0.1):
        self.device = device
        self.baudrate = baudrate
        self.timeout = timeout

        # TODO: raise custom exception if already connected
        self.reader, self.writer = await serialx.open_serial_connection(self.device, baudrate=115200)

    async def send(self, command: str) -> str:
        # TODO: raise custom exception if not connected

        try:
            # Send the command and wait a bit for the response
            self.writer.write(command)
            await self.writer.drain()
            response: bytes = await asyncio.wait_for(self.reader.readuntil(b'\n'), timeout=self.timeout)
            return response.decode("utf-8", errors='replace').strip()

        except asyncio.TimeoutError:
            try:
                # Clear the input buffer in case there are any remaining bytes
                self.writer.transport.serial.reset_input_buffer()
            except Exception:
                pass
            raise

    async def disconnect(self):
        # TODO: raise custom exception if not connected
        if self.writer:
            try:
                self.writer.close()
                await self.writer.wait_closed()
            except Exception:
                pass

        self.device = None
        self.baudrate = None
        self.timeout = None

        self.reader = None
        self.writer = None

    @staticmethod
    async def get_devices() -> list[SerialDevice]:
        devices: list[SerialDevice] = [
            await SerialDevice(
                device=device.device,
                name=device.resolved_device,
                vid=device.vid,
                pid=device.pid,
                serial_number=device.serial_number,
            ) for device in serialx.list_serial_ports() if (
                device.vid is not None and device.pid is not None and device.serial_number is not None
                )
        ]
        return devices


serial_client = SerialClient()
