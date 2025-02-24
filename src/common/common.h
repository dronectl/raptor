/**
 * @file common.h
 * @brief Collection of common functions
 * @version 0.1
 * @date 2025-01
 *
 * @copyright Copyright © 2025 dronectl
 *
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#define min(a, b) a > b ? b : a
#define max(a, b) a > b ? a : b
#define array_size(a) (size_t)(sizeof(a) / sizeof(a[0]))

#endif // __COMMON_H__
