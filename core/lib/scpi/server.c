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
#include "scpi/lexer.h"
#include "scpi/parser.h"
#include "scpi/common.h"
#include "scpi/commands.h"

#include <lwip/inet.h>
#include <lwip/sockets.h>
#include <cmsis_os2.h>

uint64_t scpi_thread_stk[1];
__attribute__((section(".bss.os.thread.cb"))) osThreadId_t scpi_handle;
const osThreadAttr_t scpi_attr = {
    .name = "scpi_task",
    .attr_bits = osThreadJoinable,
    .cb_mem = &scpi_handle,
    .cb_size = sizeof(scpi_handle),
    .stack_mem = &scpi_thread_stk[0],
    .stack_size = sizeof(scpi_thread_stk),
    .priority = osPriorityNormal1,
    .tz_module = 0,
};

static void handle_scpi_request(const struct scpi_handle *shandle) {
  char buffer[SCPI_MAX_RESPONSE_LEN] = {0};
  struct lexer_handle lhandle = {0};
  struct parser_handle phandle = {0};
  lexer_init(&lhandle);
  lexer_run(&lhandle, shandle->buffer, shandle->buflen);
  if (lhandle.status & LEXER_STAT_ERR) {
    error("Lexer failed with: 0x%X\n", lhandle.err);
    return;
  }
  info("[");
  for (int i = 0; i < lhandle.tidx + 1; i++) {
    info(" '%s' ", lhandle.tokens[i].token);
  }
  info("]\n");
  info("Number of tokens: %u\n", lhandle.tidx + 1);
  parser_run(&phandle, &lhandle);
  for (int i = 0; i < phandle.cmdidx + 1; i++) {
    info("S:0x%X|Eti:%d|Eci:%d|Es:0x%X|H:%d|A:%d\n", phandle.commands[i].spec, phandle.error.tidx, phandle.error.cidx, phandle.error, phandle.commands[i].hidx + 1, phandle.commands[i].aidx + 1);
    int index = commands_search_index(phandle.commands[i].headers, phandle.commands[i].hidx);
    if (index < 0) {
      error("command index %d endpoint not found\n", i);
      continue;
    }
    if (phandle.commands[i].spec & PARSER_CMD_SPEC_QUERY) {
      commands_process_query(index, phandle.commands[i].aidx + 1, phandle.commands[i].args, buffer, sizeof(buffer));
    } else if (phandle.commands[i].spec & PARSER_CMD_SPEC_SET) {
      commands_process_write(index, phandle.commands[i].aidx + 1, phandle.commands[i].args);
    }
    write(shandle->clfd, buffer, sizeof(buffer));
  }
}

static void handle_client_session(int client_fd) {
  struct scpi_handle shandle = {0};
  shandle.clfd = client_fd;
  for (;;) {
    memset(shandle.buffer, '\0', SCPI_MAX_INPUT_BUFFER_LEN);
    shandle.buflen = read(client_fd, shandle.buffer, sizeof(shandle.buffer));
    info("buflen: %d\n", shandle.buflen);
    if (strncmp(shandle.buffer, "\n", 1) == 0) {
      continue;
    }
    if (shandle.buflen <= 0) {
      warning("read 0 bytes from client handler\n");
      break;
    }
    info("handling inbound request: %s\n", shandle.buffer);
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
  critical("scpi socket init failed with %i\n", errno);
  osThreadExit();
}

system_status_t scpi_init(void) {
  scpi_handle = osThreadNew(scpi_main, NULL, &scpi_attr);
  if (scpi_handle == NULL) {
    return SYSTEM_MOD_FAIL;
  }
  return SYSTEM_OK;
}
