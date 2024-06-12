/**
 * @file server.c
 * @brief SCPI server
 * @version 0.1
 * @date 2024-06
 *
 * @copyright Copyright © 2024 dronectl
 *
 */

#include "logger.h"
#include "lexer.h"
#include "parser.h"
#include "scpi_server.h"
#include "scpi_endpoints.h"
#include "scpi_constants.h"

#include <lwip/inet.h>
#include <lwip/sockets.h>
#include <cmsis_os2.h>

const osThreadAttr_t health_attr = {
    .name = "scpi_task",
    .priority = osPriorityNormal1,
};
static osThreadId_t scpi_handle;

static void handle_scpi_request(const struct scpi_handle *shandle) {
  char buffer[SCPI_MAX_RESPONSE_LEN] = {0};
  struct lexer_handle lhandle = {0};
  struct parser_handle phandle = {0};
  lexer(&lhandle, shandle->buffer, shandle->buflen);
  if (lhandle.status & LEXER_STAT_ERR) {
    error("Lexer failed with: 0x%X\n", lhandle.err);
    return;
  }
  info("[");
  for (int i = 0; i < lhandle.tidx + 1; i++) {
    info(" '%s' ", lhandle.tokens[i].lexeme);
  }
  info("]\n");
  info("Number of tokens: %u\n", lhandle.tidx + 1);
  parser(&phandle, &lhandle);
  for (int i = 0; i < phandle.cmdidx + 1; i++) {
    info("S:0x%X|Eti:%d|Eci:%d|Es:0x%X|H:%d|A:%d\n", phandle.commands[i].spec, phandle.error.tidx, phandle.error.cidx, phandle.error, phandle.commands[i].hidx + 1, phandle.commands[i].aidx + 1);
    int index = scpi_endpoint_search(&phandle.commands[i]);
    if (index < 0) {
      error("command index %d endpoint not found\n", i);
      continue;
    }
    if (phandle.commands[i].spec & PARSER_CMD_SPEC_QUERY) {
      size_t slen = 0;
      scpi_endpoint_process_query(index, &phandle.commands[i], buffer, &slen);
      if (slen < SCPI_MAX_RESPONSE_LEN) {
        buffer[slen] = '\n';
      } else {
        error("Output overflow protection");
      }
    } else if (phandle.commands[i].spec & PARSER_CMD_SPEC_SET) {
      scpi_endpoint_process_write(index, &phandle.commands[i]);
    }
    // append \n delimiter
    write(shandle->clfd, buffer, sizeof(buffer));
  }
}

static void handle_client_session(int client_fd) {
  struct scpi_handle shandle = {0};
  shandle.clfd = client_fd;
  for (;;) {
    memset(shandle.buffer, '\0', SCPI_MAX_INPUT_BUFFER_LEN);
    shandle.buflen = read(client_fd, shandle.buffer, sizeof(shandle.buffer));
    if (strncmp(shandle.buffer, "\n", SCPI_MAX_INPUT_BUFFER_LEN) == 0) {
      continue;
    }
    if (shandle.buflen == 0) {
      warning("read 0 bytes from client handler");
      break;
    }
    info("handling inbound request: %s", shandle.buffer);
    handle_scpi_request(&shandle);
  }
}

static __NO_RETURN void scpi_main(__attribute__((unused)) void *argument) {
  int sock, size, client_fd;
  struct sockaddr_in address, remotehost;
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    goto error;
  }
  address.sin_family = AF_INET;
  address.sin_port = htons(SCPI_PORT);
  address.sin_addr.s_addr = INADDR_ANY;
  if (bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
    goto error;
  }
  listen(sock, 5);
  while (1) {
    client_fd = accept(sock, (struct sockaddr *)&remotehost, (socklen_t *)&size);
    if (client_fd < 0) {
      osDelay(100);
      continue;
    }
    handle_client_session(client_fd);
  }
error:
  critical("scpi socket init failed with %i", errno);
  osThreadExit();
}
