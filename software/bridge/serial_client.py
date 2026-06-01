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
from asyncio.streams import StreamReader
import serialx
from serialx.async_serial import SerialStreamWriter
from serialx.common import BaseSerialTransport, SerialPortInfo


class SerialPortError(Exception):
    pass


class SerialDeviceNotFoundError(SerialPortError):
    pass


class SerialDeviceAlreadyOpenError(SerialPortError):
    pass


class SerialDeviceNotConnectedError(SerialPortError):
    pass


class SerialClient:
    def __init__(self):
        self.device:SerialPortInfo | None = None
        self.baudrate:int | None = None
        self.timeout:float | None = None

        self.reader: StreamReader | None = None
        self.writer: SerialStreamWriter[BaseSerialTransport] | None = None

    async def connect(self, device_name: str, baudrate: int=115200, timeout: float=0.1):
        if self.writer is not None:
            raise SerialDeviceAlreadyOpenError(f"Serial device is already in use by the current process: {device_name}")

        device = next((d for d in await SerialClient.get_devices()
                       if device_name == d.device or device_name == d.resolved_device), None)

        if device is None:
            raise SerialDeviceNotFoundError(f"Serial device not found: {device_name}")

        try:
            self.reader, self.writer = await serialx.open_serial_connection(url=device_name, baudrate=baudrate)

        except OSError as e:
            raise SerialDeviceAlreadyOpenError(f"Serial device is already in use by another process: {device_name}") from e

        self.device = device
        self.baudrate = baudrate
        self.timeout = timeout


    async def send(self, command: str) -> str:
        if self.writer is None or self.reader is None:
            raise SerialDeviceNotConnectedError("Serial device not connected")

        try:
            # Send the command and wait a bit for the response
            self.writer.write((command + '\n').encode())
            await self.writer.drain()
            response: bytes = await asyncio.wait_for(self.reader.readuntil(b'\n'), timeout=self.timeout)
            return response.decode("utf-8", errors='replace').strip()

        except OSError as e:
            await self.disconnect()
            raise SerialDeviceNotConnectedError("Serial device got disconnected") from e

        except asyncio.TimeoutError:
            try:
                # Clear the input buffer in case there are any remaining bytes
                self.writer.transport.serial.reset_input_buffer()
            except Exception:
                pass
            raise

    async def disconnect(self):
        if self.writer is None:
            return

        try:
            self.writer.close()
            await self.writer.wait_closed()

        except OSError:
            pass

        finally:
            self.device = None
            self.baudrate = None
            self.timeout = None

            self.reader = None
            self.writer = None

    @staticmethod
    async def get_devices() -> list[SerialPortInfo]:
        return [device for device in await serialx.async_list_serial_ports()
                if (device.vid is not None and device.pid is not None and device.serial_number is not None)]


serial_client = SerialClient()
