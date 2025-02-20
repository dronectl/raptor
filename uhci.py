"""
µHCI Driver for Raptor
======================
Modified: 2025-01



"""

import logging
import json
import socket
import coloredlogs
from google.protobuf.json_format import MessageToJson

from raptor.v1.commands_pb2 import * 
from raptor.v1.uhci_pb2 import *
from raptor.v1.device_pb2 import *

coloredlogs.install(logging.DEBUG)
_logger = logging.getLogger(__name__)

class UHCIClient:

    BUFFER_SIZE = 1024
    DISCOVERY_TIMEOUT_S = 2.0 # two second discovery timeout

    def __init__(self, ip_address: str, uuid: int) -> None:
        self._logger = logging.getLogger(__name__)
        self.ip_address = ip_address
        self.addr = f'{self.ip_address}:{UHCI_PORT_COMMAND}'
        self.uuid = uuid
        self.host = socket.gethostname()
        self._logger.info("Instantiated %s", str(self))

    def __repr__(self) -> str:
        return f"UHCI {self.uuid} {self.addr}"

    def send_command(self, request: UHCIBaseRequest) -> UHCIBaseResponse:
        serialized_data = request.SerializeToString()
        self._logger.info("%s -> %s\t%s", self.host, self.addr, MessageToJson(request, indent=None, preserving_proto_field_name=True))
        with socket.create_connection((self.ip_address, UHCI_PORT_COMMAND)) as sock:
            sock.sendall(serialized_data)
            response_data = sock.recv(self.BUFFER_SIZE)  # Adjust buffer size if necessary
            response = UHCIBaseResponse()
            response.ParseFromString(response_data)
        self._logger.info("%s <- %s\t%s", self.host, self.addr, MessageToJson(response, indent=None, preserving_proto_field_name=True))
        return response

    @staticmethod
    def get_hostname() -> str:
        return socket.gethostname()

    @staticmethod
    def discover(broadcast_ip: str = '255.255.255.255') -> list[tuple[str, DeviceMetadata]]:
        devices = []
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP) as sock:
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
            request = UHCIBaseRequest(
                uhci=UHCIProtocolRequest(
                    discovery=UHCIDiscoveryRequest()
                )
            )
            host = UHCIClient.get_hostname()
            serialized_request = request.SerializeToString()
            _logger.info("%s -> %s\t%s", host,  broadcast_ip, MessageToJson(request, indent=None, preserving_proto_field_name=True))
            sock.sendto(serialized_request, (broadcast_ip, UHCI_PORT_DISCOVERY))
            # response to discovery must be made within 2-second window
            sock.settimeout(UHCIClient.DISCOVERY_TIMEOUT_S)
            try:
                while True:
                    data, addr = sock.recvfrom(UHCIClient.BUFFER_SIZE)
                    ip_addr, port = addr
                    response = UHCIBaseResponse()
                    response.ParseFromString(data)
                    _logger.info("%s <- %s\t%s", host, f'{ip_addr}:{port}', MessageToJson(response, indent=None, preserving_proto_field_name=True))
                    devices.append((ip_addr, response.uhci.discovery.device))
            except socket.timeout:
                pass
        return devices


if __name__ == "__main__":
    ip_addr, meta = UHCIClient.discover()[0]
    raptor = UHCIClient(ip_addr, meta.uuid)
    req = UHCIBaseRequest(
        user=UserRequest(
            device_metadata=DeviceMetadataRequest()
        )
    )
    resp = raptor.send_command(req)
    assert resp.status == UHCI_STATUS_OK
