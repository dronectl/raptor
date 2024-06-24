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

__attribute__((section(".bss"))) static struct lexer_handle lhandle;
__attribute__((section(".bss"))) static struct parser_handle phandle;
__attribute__((section(".bss"))) static char buffer[SCPI_MAX_INPUT_BUFFER_LEN];

const osThreadAttr_t scpi_attr = {
    .name = "scpi_task",
    .priority = osPriorityNormal5,
};
static osThreadId_t scpi_handle;

static void handle_scpi_request(const struct scpi_handle *shandle) {
  lexer_init(&lhandle);
  parser_init(&phandle);
  lexer_run(&lhandle, shandle->buffer, shandle->buflen);
  if (lhandle.status & LEXER_STAT_ERR) {
    error("Lexer failed with: 0x%X\n", lhandle.err);
    return;
  }
  parser_run(&phandle, &lhandle);
  for (int i = 0; i < phandle.cmdidx + 1; i++) {
    info("S:0x%X|Eti:%d|Eci:%d|Es:0x%X|H:%d|A:%d\n", phandle.commands[i].spec, phandle.error.tidx, phandle.error.cidx, phandle.error, phandle.commands[i].hidx + 1, phandle.commands[i].aidx + 1);
    int index = commands_search_index(phandle.commands[i].headers, phandle.commands[i].hidx);
    if (index < 0) {
      error("command index %d endpoint not found\n", i);
      continue;
    }
    if (phandle.commands[i].spec & PARSER_CMD_SPEC_QUERY) {
      commands_process_query(index, phandle.commands[i].aidx + 1, phandle.commands[i].args, &buffer[0], sizeof(buffer));
    } else if (phandle.commands[i].spec & PARSER_CMD_SPEC_SET) {
      commands_process_write(index, phandle.commands[i].aidx + 1, phandle.commands[i].args);
    }
  }
  write(shandle->clfd, buffer, strlen(buffer));
}

static void handle_client_session(int client_fd) {
  struct scpi_handle shandle = {0};
  shandle.clfd = client_fd;
  for (;;) {
    memset(shandle.buffer, '\0', SCPI_MAX_INPUT_BUFFER_LEN);
    info("Waiting on read...\n");
    shandle.buflen = read(client_fd, shandle.buffer, sizeof(shandle.buffer));
    if (shandle.buflen <= 0) {
      break;
    }
    if (strncmp(shandle.buffer, "\n", 1) == 0) {
      continue;
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
  if (listen(sock, 5) < 0) {
    goto error;
  }
  while (1) {
    info("blocking on accept...\n");
    client_fd = accept(sock, (struct sockaddr *)&remotehost, (socklen_t *)&size);
    info("Accepted connection with fd: %d\n", client_fd);
    if (client_fd < 0) {
      osThreadYield();
      continue;
    }
    handle_client_session(client_fd);
    info("closing session\n");
    close(client_fd);
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
