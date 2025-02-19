/**
 * @file uhci.c
 * @brief Micro Host Controller Interface Server
 * @version 0.1
 * @date 2025-02
 *
 * @copyright Copyright © 2025 dronectl
 *
 */

#include "pb_decode.h"
#include "pb_encode.h"
#include "uassert.h"
#include "logger.h"
#include "uhci.h"
#include "raptor/v1/uhci.pb.h"

#include <lwip/inet.h>
#include <lwip/sockets.h>
#include <FreeRTOS.h>
#include <task.h>

#define UHCI_MAX_CLIENTS 1

static struct uhci_context ctx = {0};

/**
 * @brief Decode bytes to UHCI request.
 *
 * @param[in] buffer Encoded bytes
 * @param[in] buffer_len buffer length in bytes
 * @param[out] request Protobuf request struct
 * @return true if decode successful, false otherwise
 */
static bool decode_uhci_request(const uint8_t *buffer, const size_t buffer_len, raptor_v1_UHCIBaseRequest *request);

/**
 * @brief Encode bytes to UHCI response.
 *
 * @param[out] buffer Buffer to store encoded bytes
 * @param[in] buffer_len Buffer length in bytes
 * @param[in] response Protobuf response struct
 * @param[out] encode_bytes Number of bytes encoded
 * @return true if encode successful, false otherwise
 */
static bool encode_uhci_response(uint8_t *buffer, const size_t buffer_len, const raptor_v1_UHCIBaseResponse *response, size_t *encoded_bytes);

/**
 * @brief Process User request and build response payload
 *
 * @param[in] req User request payload
 * @param[in,out] resp User response payload
 */
static void process_user_request(const raptor_v1_UserRequest *req, raptor_v1_UserResponse *resp);

/**
 * @brief Process elevated permissions request and build response payload. Perform neccessary authentication.
 *
 * @param[in] req Root request payload
 * @param[in,out] resp Root response payload
 */
static void process_root_request(const raptor_v1_RootRequest *req, raptor_v1_RootResponse *resp);

/**
 * @brief Process UHCI protocol request and build response payload
 *
 * @param[in] req UHCI request payload
 * @param[in,out] resp UHCI response payload
 */
static void process_uhci_request(const raptor_v1_UHCIProtocolRequest *req, raptor_v1_UHCIProtocolResponse *resp);

/**
 * @brief Handle UDP discovery service event.
 * @todo This function should implement ddos protection rate limit.
 *
 * @param[in] udp_socket UDP socket file descriptor
 */
static void handle_udp_discovery_service_event(const int udp_socket);

/**
 * @brief Handle new TCP client session.
 *
 * @param[in] tcp_socket TCP socket file descriptor
 */
static void handle_tcp_base_request(const int tcp_socket);

static bool decode_uhci_request(const uint8_t *buffer, const size_t buffer_len, raptor_v1_UHCIBaseRequest *request) {
  uassert(buffer != NULL);
  uassert(request != NULL);
  pb_istream_t stream = pb_istream_from_buffer(buffer, buffer_len);
  return pb_decode(&stream, raptor_v1_UHCIBaseRequest_fields, request);
}

static bool encode_uhci_response(uint8_t *buffer, const size_t buffer_len, const raptor_v1_UHCIBaseResponse *response, size_t *encoded_bytes) {
  uassert(buffer != NULL);
  uassert(response != NULL);
  uassert(encoded_bytes != NULL);
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, buffer_len);
  const bool status = pb_encode(&stream, raptor_v1_UHCIBaseRequest_fields, response);
  *encoded_bytes = stream.bytes_written;
  return status;
}

static void process_root_request(const raptor_v1_RootRequest *req, raptor_v1_RootResponse *resp)
{
  (void)req;
  (void)resp;
}

static void process_user_request(const raptor_v1_UserRequest *req, raptor_v1_UserResponse *resp)
{
  (void)req;
  (void)resp;
}

static void process_uhci_request(const raptor_v1_UHCIProtocolRequest *req, raptor_v1_UHCIProtocolResponse *resp)
{
  (void)req;
  (void)resp;
}

static void handle_udp_discovery_service_event(const int udp_socket) {
  uint8_t buffer[256];
  size_t encoded_bytes = 0;
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  raptor_v1_UHCIBaseRequest req = raptor_v1_UHCIBaseRequest_init_zero;
  raptor_v1_UHCIBaseResponse resp = raptor_v1_UHCIBaseResponse_init_zero;

  const ssize_t buflen = recvfrom(udp_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &client_addr_len);
  info("Received discovery packet from %s\n", inet_ntoa(client_addr.sin_addr));

  if (buflen <= 0) {
    error("Failed to receive UDP packet\n");
    return;
  }

  if (decode_uhci_request(buffer, buflen, &req)) {
    switch (req.which_request_mux) {
      case raptor_v1_UHCIBaseRequest_uhci_tag:
        process_uhci_request(&req.request_mux.uhci, &resp.response_mux.uhci);
        break;
      default:
        resp.status = raptor_v1_UHCIStatus_UHCI_STATUS_NOT_FOUND;
        break;
    }
  } else {
    resp.status = raptor_v1_UHCIStatus_UHCI_STATUS_DECODE_ERR;
  }

  memset(buffer, 0, sizeof(buffer));
  if (encode_uhci_response(buffer, sizeof(buffer), &resp, &encoded_bytes)) {
    if (sendto(udp_socket, buffer, encoded_bytes, 0, (struct sockaddr *)&client_addr, client_addr_len) < 0) {
      error("Failed to send response to client\n");
    }
  }
}

static void handle_tcp_base_request(const int tcp_socket) {
  uint8_t buffer[256];
  size_t encoded_bytes = 0;
  int client_fd = -1;
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  raptor_v1_UHCIBaseRequest req = raptor_v1_UHCIBaseRequest_init_zero;
  raptor_v1_UHCIBaseResponse resp = raptor_v1_UHCIBaseResponse_init_zero;

  client_fd = accept(tcp_socket, (struct sockaddr *)&client_addr, &client_addr_len);
  if (client_fd < 0) {
    error("Failed to accept client connection\n");
    return;
  }
  info("Accepted client connection from %s\n", inet_ntoa(client_addr.sin_addr));

  const ssize_t buflen = read(client_fd, buffer, sizeof(buffer));

  if (decode_uhci_request(buffer, buflen, &req)) {
    switch (req.which_request_mux) {
      case raptor_v1_UHCIBaseRequest_uhci_tag:
        process_uhci_request(&req.request_mux.uhci, &resp.response_mux.uhci);
        break;
      case raptor_v1_UHCIBaseRequest_root_tag:
        process_root_request(&req.request_mux.root, &resp.response_mux.root);
        break;
      case raptor_v1_UHCIBaseRequest_user_tag:
        process_user_request(&req.request_mux.user, &resp.response_mux.user);
        break;
      default:
        resp.status = raptor_v1_UHCIStatus_UHCI_STATUS_NOT_FOUND;
        break;
    }
  } else {
    resp.status = raptor_v1_UHCIStatus_UHCI_STATUS_DECODE_ERR;
  }

  memset(buffer, 0, sizeof(buffer));
  if (encode_uhci_response(buffer, sizeof(buffer), &resp, &encoded_bytes)) {
    if (write(client_fd, buffer, encoded_bytes) < 0) {
      // TODO: handle write error
      error("Failed to write response to client\n");
    }
  }
  close(client_fd);
}

static void uhci_srv_task(void*  __attribute__((unused)) argument) {
  fd_set read_fds;
  int udp_socket, tcp_socket, result;
  struct sockaddr_in udp_addr, tcp_addr;
  
  // initialize and bind udp socket
  udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
  memset(&udp_addr, 0, sizeof(udp_addr));
  udp_addr.sin_family = AF_INET;
  udp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  udp_addr.sin_port = htons(raptor_v1_UHCIPort_UHCI_PORT_DISCOVERY);
  result = bind(udp_socket, (struct sockaddr *)&udp_addr, sizeof(udp_addr));
  uassert(result >= 0);
  info("UHCI discovery service bound to UDP port %d\n", raptor_v1_UHCIPort_UHCI_PORT_DISCOVERY);

  // initialize and bind tcp socket
  tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
  memset(&tcp_addr, 0, sizeof(tcp_addr));
  tcp_addr.sin_family = AF_INET;
  tcp_addr.sin_port = htons(raptor_v1_UHCIPort_UHCI_PORT_COMMAND);
  tcp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  result = bind(tcp_socket, (struct sockaddr *)&tcp_addr, sizeof(tcp_addr));
  uassert(result >= 0);
  info("UHCI command server bound to TCP port %d\n", raptor_v1_UHCIPort_UHCI_PORT_COMMAND);

  // listen on tcp socket
  result = listen(tcp_socket, UHCI_MAX_CLIENTS);
  uassert(result >= 0);

  while (1) {
    // initialize file descriptor set
    FD_ZERO(&read_fds);
    FD_SET(udp_socket, &read_fds);
    FD_SET(tcp_socket, &read_fds);
    const int max_fd = (udp_socket > tcp_socket) ? udp_socket : tcp_socket;

    // wait for activity on either UDP or TCP socket
    select(max_fd + 1, &read_fds, NULL, NULL, NULL);

    if (FD_ISSET(udp_socket, &read_fds)) {
      handle_udp_discovery_service_event(udp_socket);
    }

    if (FD_ISSET(tcp_socket, &read_fds)) {
      handle_tcp_base_request(tcp_socket);
    }
  }
  vTaskDelete(NULL);
}

void uhci_start(const struct system_task_context *task_ctx) {
  uassert(task_ctx != NULL);
  BaseType_t ret = xTaskCreate(uhci_srv_task, task_ctx->name, task_ctx->stack_size, NULL, task_ctx->priority, &ctx.task_handle);
  uassert(ret == pdPASS);
}
