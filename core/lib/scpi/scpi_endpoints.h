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

#include "parser.h"
#include "scpi_server.h"
#include "scpi/scpi_constants.h"

void scpi_endpoint_process_query(const int index, struct parser_cmd *cmd, char *buffer, int *len);
void scpi_endpoint_process_write(const int index, const struct parser_cmd *cmd);
void scpi_endpoint_search(const struct parser_cmd *pcmd);

#endif // __SCPI_ENDPOINTS_H__
