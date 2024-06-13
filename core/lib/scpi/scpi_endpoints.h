/**
 * @file scpi_endpoints.h
 * @brief SCPI command callbacks
 * @version 0.1
 * @date 2024-06
 *
 * @copyright Copyright © 2024 dronectl
 *
 */

#ifndef __SCPI_ENDPOINTS_H__
#define __SCPI_ENDPOINTS_H__

#include "scpi.h"
#include <stddef.h>

void scpi_endpoint_process_query(const int index, const uint8_t argc, const struct scpi_token argv[], char *buffer, const size_t size);
void scpi_endpoint_process_write(const int index, const uint8_t argc, const struct scpi_token argv[]);
int scpi_endpoint_search_index(const struct scpi_token sts[], const uint8_t len);

#endif // __SCPI_ENDPOINTS_H__
