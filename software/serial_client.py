# -*- coding: utf-8 -*-
"""
Filename: serial_client.py
Author: Bastien Sagetat
Description:
    SerialClient class implementation.
    Create an instance of the class in order to use it as a singleton.

License: MIT License
"""

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

serial_client = SerialClient()
