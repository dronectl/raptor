/**
 * @file utf8.h
 * @brief UTF8 and ASCII string helpers
 * @version 0.1
 * @date 2024-06
 *
 * @copyright Copyright © 2024 dronectl
 *
 */

#ifndef __UTF8_H__
#define __UTF8_H__

#include <stdbool.h>

bool utf8_is_numeric(const char c);
bool utf8_is_alpha(const char c);
bool utf8_is_uppercase(const char c);
bool utf8_is_lowercase(const char c);
char utf8_uppercase_to_lowercase(char c);
char utf8_lowercase_to_uppercase(char c);

#endif // __UTF8_H__
