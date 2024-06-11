/**
 * @file scpi_constants.h
 * @brief SCPI server buffer limits
 * @version 0.1
 * @date 2024-06
 *
 * @copyright Copyright © 2024 dronectl
 *
 */

#ifndef __SCPI_CONSTANTS_H__
#define __SCPI_CONSTANTS_H__

#define SCPI_MAX_INPUT_BUFFER_LEN 128
#define SCPI_MAX_RESPONSE_LEN 128
#define SCPI_MAX_CMD_ARGS 10
#define SCPI_MAX_CMD_HDRS 5
#define SCPI_MAX_COMMANDS 5
#define SCPI_MAX_TOKENS 30
#define SCPI_MAX_RET_LEN 10
#define SCPI_MAX_TOKEN_LEN 15

#endif // __SCPI_CONSTANTS_H__
