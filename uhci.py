"""
µHCI Driver for Raptor
======================
Modified: 2025-01


"""

import os
import logging
import json
import socket
import coloredlogs
from google.protobuf.json_format import MessageToJson
from dataclasses import dataclass
from commands_pb2 import COMMAND_STATUS_OK, CommandRequest, CommandResponse, GetVersionRequest, DiscoveryRequest, DiscoveryResponse


@dataclass
class RaptorDevice:
    uuid: int
    hardware_version: str
    firmware_version: str
    ip_address: str


class RaptorUHCI:

    UHCI_PORT = 50051  # use a non-wellknown port
    UHCI_DISCOVERY_PORT = 8000  # use a non-wellknown port
    BUFFER_SIZE = 1024
    DISCOVERY_TIMEOUT_S = 2.0 # two second discovery timeout

    def __init__(self, ip_address: str, uuid: int) -> None:
        self._logger = logging.getLogger(__name__)
        self.ip_address = ip_address
        self.addr = f'{self.ip_address}:{self.UHCI_PORT}'
        self.uuid = uuid
        self.host = socket.gethostname()
        self._logger.info("Instantiated %s", str(self))

    def __repr__(self) -> str:
        return f"UHCI {self.uuid} {self.addr}"

    def send_command(self, command: CommandRequest) -> CommandResponse:
        serialized_data = command.SerializeToString()
        self._logger.info("%s -> %s\t%s", self.host, self, json.dumps(MessageToJson(command)))
        with socket.create_connection((self.ip_address, self.UHCI_PORT)) as sock:
            sock.sendall(serialized_data)
            response_data = sock.recv(self.BUFFER_SIZE)  # Adjust buffer size if necessary
            response = CommandResponse()
            response.ParseFromString(response_data)
        self._logger.info("%s <- %s\t%s", json.dumps(MessageToJson(command)))
        return response

    @staticmethod
    def discover(broadcast_ip: str = '255.255.255.255') -> list[RaptorDevice]:
        devices = []
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP) as sock:
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
            request = DiscoveryRequest()
            serialized_request = request.SerializeToString()
            sock.sendto(serialized_request, (broadcast_ip, RaptorUHCI.UHCI_DISCOVERY_PORT))
            # response to discovery must be made within 2-second window
            sock.settimeout(RaptorUHCI.DISCOVERY_TIMEOUT_S)
            try:
                while True:
                    data, addr = sock.recvfrom(RaptorUHCI.BUFFER_SIZE)
                    response = DiscoveryResponse()
                    response.ParseFromString(data)
                    devices.append(
                        RaptorDevice(
                            ip_address=addr,
                            uuid=response.uuid,
                            hardware_version=response.hardware_version,
                            firmware_version=response.firmware_version
                        )
                    )
            except socket.timeout:
                pass
        return devices


if __name__ == "__main__":
    coloredlogs.install(logging.DEBUG)
    device = RaptorUHCI.discover()[0]
    raptor = RaptorUHCI(device.ip_address, device.uuid)
    req = CommandRequest(get_version=GetVersionRequest())
    resp = raptor.send_command(req)
    assert resp.status == COMMAND_STATUS_OK
