
import grpc
from commands_pb2_grpc import CommandServiceStub
from commands_pb2 import COMMAND_STATUS_OK, CommandRequest, CommandResponse, GetVersionRequest


class RaptorGrpc:
    def __init__(self, ip_addr: str, port: int) -> None:
        self.grpc_addr = f'{ip_addr}:{port}'
        self.channel = grpc.insecure_channel(
            self.grpc_addr,
            options=(('grpc.max_connection_age_ms', 3000),)
        )
        self.stub = CommandServiceStub(self.channel)

    def send_command(self, request: CommandRequest) -> CommandResponse:
        response = self.stub.SendCommand(request)
        return response


if __name__ == "__main__":
    raptor = RaptorGrpc(ip_addr='172.16.1.101', port=50051)
    req = CommandRequest(get_version=GetVersionRequest())
    resp = raptor.send_command(req)
    assert resp.status == COMMAND_STATUS_OK
