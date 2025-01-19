/**
 * @file grpc.c
 * @brief grpc server
 * @version 0.1
 * @date 2024-06
 *
 * @copyright Copyright © 2024 dronectl
 *
 */

#include "pb_decode.h"
#include "pb_encode.h"
#include "uassert.h"
#include "logger.h"
#include "grpc.h"
#include "raptor/v1/commands.pb.h"

#include <lwip/inet.h>
#include <lwip/sockets.h>
#include <FreeRTOS.h>
#include <task.h>

const char *firmware_version_str = "1.0.0";
const char *hardware_version_str = "0.1.0";

#define STACK_SIZE 4000

__attribute__((section(".ram_d3"))) static StackType_t grpc_srv_task_stk[STACK_SIZE];
__attribute__((section(".ram_d3"))) static StaticTask_t grpc_srv_task_buffer;
static TaskHandle_t grpc_srv_task_handle;

static bool decode_command_request(uint8_t *data, const size_t len, raptor_v1_CommandRequest *request) {
  pb_istream_t stream = pb_istream_from_buffer(data, len);
  return pb_decode(&stream, raptor_v1_CommandRequest_fields, request);
}

static bool encode_command_response(uint8_t *buffer, size_t buffer_size, raptor_v1_CommandResponse *response, size_t *encoded_size) {
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, buffer_size);
  if (!pb_encode(&stream, raptor_v1_CommandResponse_fields, response)) {
    return false;
  }
  *encoded_size = stream.bytes_written;
  return true;
}

static bool encode_string_callback(pb_ostream_t *stream, const pb_field_iter_t *field, void *const *arg) {
  const char *str = (const char *)(*arg);
  if (!pb_encode_tag_for_field(stream, field)) {
    return false; // Failed to encode tag
  }
  return pb_encode_string(stream, (uint8_t *)str, strlen(str));
}

static int process_get_version(const raptor_v1_GetVersionRequest *req, raptor_v1_GetVersionResponse *resp) {
  resp->firmware_version.funcs.encode = &encode_string_callback;
  resp->firmware_version.arg = (void *)firmware_version_str;
  resp->hardware_version.funcs.encode = &encode_string_callback;
  resp->hardware_version.arg = (void *)hardware_version_str;
  return raptor_v1_CommandStatus_COMMAND_STATUS_OK;
}

static void process_grpc_command(const raptor_v1_CommandRequest *req, raptor_v1_CommandResponse *resp) {
  switch (req->which_request_mux) {
    case raptor_v1_CommandRequest_get_version_tag:
      // to get version handling
      resp->status = process_get_version(&req->request_mux.get_version, &resp->response_mux.get_version);
      break;
    default:
      warning("Unknown request mux value.");
      resp->status = raptor_v1_CommandStatus_COMMAND_STATUS_GEN_ERR;
      break;
  }
}

static void process_request(const int client_fd) {
  uint8_t rx_buffer[256];
  uint8_t tx_buffer[256];
  raptor_v1_CommandRequest req = {0};
  raptor_v1_CommandResponse resp = {
      .status = raptor_v1_CommandStatus_COMMAND_STATUS_UNSPECIFIED};
  size_t buflen = read(client_fd, rx_buffer, sizeof(rx_buffer));
  if (decode_command_request(rx_buffer, buflen, &req)) {
    process_grpc_command(&req, &resp);
  } else {
    resp.status = raptor_v1_CommandStatus_COMMAND_STATUS_GEN_ERR;
  }
  size_t encoded_bytes;
  if (encode_command_response(tx_buffer, sizeof(tx_buffer), &resp, &encoded_bytes)) {
    write(client_fd, tx_buffer, encoded_bytes);
  }
}

static void grpc_srv_task(__attribute__((unused)) void *argument) {
  int sock, size, client_fd;
  struct sockaddr_in address, remotehost;
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    goto error;
  }
  address.sin_family = AF_INET;
  address.sin_port = htons(50051);
  address.sin_addr.s_addr = INADDR_ANY;
  if (bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
    goto error;
  }
  if (listen(sock, 5) < 0) {
    goto error;
  }
  while (1) {
    client_fd = accept(sock, (struct sockaddr *)&remotehost, (socklen_t *)&size);
    if (client_fd < 0) {
      taskYIELD();
      continue;
    }
    info("fd (%d) accepting connection\n", client_fd);
    process_request(client_fd);
    info("fd (%d) closing session\n", client_fd);
    close(client_fd);
  }
error:
  critical("grpc server socket init failed with %i\n", errno);
  vTaskDelete(NULL);
}

void grpc_init(void) {
  grpc_srv_task_handle = xTaskCreateStatic(grpc_srv_task, "grpc", STACK_SIZE, NULL, tskIDLE_PRIORITY + 10, grpc_srv_task_stk, &grpc_srv_task_buffer);
  uassert(grpc_srv_task_handle != NULL);
}
