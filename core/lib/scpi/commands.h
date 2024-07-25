/**
 * @file scpi_endpoints.h
 * @brief SCPI command callbacks
 * @version 0.1
 * @date 2024-06
 *
 * @copyright Copyright © 2024 dronectl
 *
 */

#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include "scpi/common.h"
#include <stddef.h>

void commands_process_query(const int index, const uint8_t argc, const struct scpi_token argv[], char *buffer, const size_t size);
void commands_process_write(const int index, const uint8_t argc, const struct scpi_token argv[]);
int commands_search_index(const struct scpi_token sts[], const uint8_t len);

#endif // __COMMANDS_H__
