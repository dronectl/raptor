/**
 * @file server.c
 * @brief SCPI server
 * @version 0.1
 * @date 2024-06
 *
 * @copyright Copyright © 2024 dronectl
 *
 */

#include "uassert.h"
#include "logger.h"
#include "scpi/ieee.h"
#include "scpi/lexer.h"
#include "scpi/parser.h"
#include "scpi/common.h"
#include "scpi/commands.h"

#include <lwip/inet.h>
#include <lwip/sockets.h>
#include <FreeRTOS.h>
#include <task.h>

#define STACK_SIZE 4000

__attribute__((section(".ram_d3"))) static StackType_t scpi_task_stk[STACK_SIZE];
__attribute__((section(".ram_d3"))) static StaticTask_t scpi_task_buffer;
static TaskHandle_t scpi_task_handle;

static void handle_scpi_request(const struct scpi_handle *shandle) {
  char buffer[SCPI_MAX_RESPONSE_LEN] = {0};
  struct lexer_handle lhandle = {0};
  struct parser_handle phandle = {0};
  lexer_init(&lhandle);
  parser_init(&phandle);
  lexer_run(&lhandle, shandle->buffer, shandle->buflen);
  if (lhandle.status & LEXER_STAT_ERR) {
    error("Lexer failed with: 0x%X\n", lhandle.err);
    return;
  }
  parser_run(&phandle, &lhandle);
  for (int i = 0; i < phandle.cmdidx + 1; i++) {
    info("S:0x%X|Eti:%d|Eci:%d|Es:0x%X|H:%d|A:%d\n", phandle.commands[i].spec, phandle.error.tidx, phandle.error.cidx, phandle.error.code, phandle.commands[i].hidx, phandle.commands[i].aidx);
    int index = commands_search_index(phandle.commands[i].headers, phandle.commands[i].hidx);
    if (index < 0) {
      warning("command endpoint not found\n");
    }
    if (phandle.commands[i].spec & PARSER_CMD_SPEC_QUERY) {
      commands_process_query(index, phandle.commands[i].aidx, phandle.commands[i].args, &buffer[i], sizeof(buffer));
    } else if (phandle.commands[i].spec & PARSER_CMD_SPEC_SET) {
      commands_process_write(index, phandle.commands[i].aidx, phandle.commands[i].args);
    }
  }
  write(shandle->clfd, buffer, strlen(buffer));
}

static void handle_client_session(int client_fd) {
  struct scpi_handle shandle;
  shandle.clfd = client_fd;
  for (;;) {
    memset(shandle.buffer, '\0', sizeof(shandle.buffer));
    shandle.buflen = read(client_fd, shandle.buffer, sizeof(shandle.buffer));
    if (shandle.buflen <= 0) {
      break;
    }
    if (strncmp(shandle.buffer, "\n", 1) == 0) {
      continue;
    }
    info("handling inbound request: %s", shandle.buffer);
    handle_scpi_request(&shandle);
  }
}

static void scpi_task(__attribute__((unused)) void *argument) {
  int sock, size, client_fd;
  struct sockaddr_in address, remotehost;
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    goto error;
  }
  address.sin_family = AF_INET;
  address.sin_port = htons(IEEE_SCPI_PORT);
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
    handle_client_session(client_fd);
    info("fd (%d) closing session\n", client_fd);
    close(client_fd);
  }
error:
  critical("scpi socket init failed with %i\n", errno);
  vTaskDelete(NULL);
}

void scpi_init(void) {
  scpi_error_init();
  scpi_task_handle = xTaskCreateStatic(scpi_task, "scpi_task", STACK_SIZE, NULL, tskIDLE_PRIORITY + 10, scpi_task_stk, &scpi_task_buffer);
  uassert(scpi_task_handle == NULL);
}
