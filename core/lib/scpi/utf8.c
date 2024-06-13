#include "scpi/utf8.h"

bool utf8_is_numeric(const char c) {
  if (c < UTF8_OFFSET || c > UTF8_MAX) {
    return false;
  }
  return utf8_lut[(uint8_t)c - UTF8_OFFSET] & UTF8_NUMERIC;
}

bool utf8_is_alpha(const char c) {
  if (c < UTF8_OFFSET || c > UTF8_MAX) {
    return false;
  }
  return utf8_lut[(uint8_t)c - UTF8_OFFSET] & UTF8_ALPHA;
}

bool utf8_is_uppercase(const char c) {
  if (c < UTF8_OFFSET || c > UTF8_MAX) {
    return false;
  }
  return utf8_lut[(uint8_t)c - UTF8_OFFSET] & UTF8_LOWERCASE;
}

bool utf8_is_lowercase(const char c) {
  if (c < UTF8_OFFSET || c > UTF8_MAX) {
    return false;
  }
  return utf8_lut[(uint8_t)c - UTF8_OFFSET] & UTF8_LOWERCASE;
}

char utf8_uppercase_to_lowercase(char c) {
  if (!utf8_is_uppercase(c)) {
    return c;
  }
  return c - UTF8_CAPS_OFFSET;
}

char utf8_lowercase_to_uppercase(char c) {
  if (!utf8_is_lowercase(c)) {
    return c;
  }
  return c + UTF8_CAPS_OFFSET;
}