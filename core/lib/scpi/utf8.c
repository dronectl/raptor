#include "scpi/utf8.h"
#include <stdint.h>

#define UTF8_OFFSET (uint8_t)'0'
#define UTF8_MAX ('z' - UTF8_OFFSET)
#define UTF8_CAPS_OFFSET 0x20
#define UTF8_NUMERIC (1 << 0)
#define UTF8_ALPHA (1 << 1)
#define UTF8_UPPERCASE (1 << 2)
#define UTF8_LOWERCASE (1 << 3)

// clang-format off
static const uint8_t utf8_lut[] = {
    ['0' - UTF8_OFFSET] = UTF8_NUMERIC,
    ['1' - UTF8_OFFSET] = UTF8_NUMERIC,
    ['2' - UTF8_OFFSET] = UTF8_NUMERIC,
    ['3' - UTF8_OFFSET] = UTF8_NUMERIC,
    ['4' - UTF8_OFFSET] = UTF8_NUMERIC,
    ['5' - UTF8_OFFSET] = UTF8_NUMERIC,
    ['6' - UTF8_OFFSET] = UTF8_NUMERIC,
    ['7' - UTF8_OFFSET] = UTF8_NUMERIC,
    ['8' - UTF8_OFFSET] = UTF8_NUMERIC,
    ['9' - UTF8_OFFSET] = UTF8_NUMERIC,
    ['A' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_UPPERCASE,
    ['B' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_UPPERCASE,
    ['C' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_UPPERCASE,
    ['D' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_UPPERCASE,
    ['E' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_UPPERCASE,
    ['F' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_UPPERCASE,
    ['G' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_UPPERCASE,
    ['H' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_UPPERCASE,
    ['I' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_UPPERCASE,
    ['J' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_UPPERCASE,
    ['K' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_UPPERCASE,
    ['L' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_UPPERCASE,
    ['M' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_UPPERCASE,
    ['N' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_UPPERCASE,
    ['O' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_UPPERCASE,
    ['P' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_UPPERCASE,
    ['Q' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_UPPERCASE,
    ['R' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_UPPERCASE,
    ['S' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_UPPERCASE,
    ['T' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_UPPERCASE,
    ['U' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_UPPERCASE,
    ['V' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_UPPERCASE,
    ['W' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_UPPERCASE,
    ['X' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_UPPERCASE,
    ['Y' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_UPPERCASE,
    ['Z' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_UPPERCASE,
    ['a' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_LOWERCASE,
    ['b' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_LOWERCASE,
    ['c' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_LOWERCASE,
    ['d' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_LOWERCASE,
    ['e' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_LOWERCASE,
    ['f' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_LOWERCASE,
    ['g' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_LOWERCASE,
    ['h' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_LOWERCASE,
    ['i' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_LOWERCASE,
    ['j' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_LOWERCASE,
    ['k' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_LOWERCASE,
    ['l' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_LOWERCASE,
    ['m' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_LOWERCASE,
    ['n' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_LOWERCASE,
    ['o' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_LOWERCASE,
    ['p' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_LOWERCASE,
    ['q' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_LOWERCASE,
    ['r' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_LOWERCASE,
    ['s' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_LOWERCASE,
    ['t' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_LOWERCASE,
    ['u' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_LOWERCASE,
    ['v' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_LOWERCASE,
    ['w' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_LOWERCASE,
    ['x' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_LOWERCASE,
    ['y' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_LOWERCASE,
    ['z' - UTF8_OFFSET] = UTF8_ALPHA | UTF8_LOWERCASE,
};
// clang-format on

bool utf8_is_numeric(const char c) {
  int idx = (uint8_t)c - UTF8_OFFSET;
  if (idx < 0 || idx > UTF8_MAX) {
    return false;
  }
  return utf8_lut[idx] & UTF8_NUMERIC;
}

bool utf8_is_alpha(const char c) {
  int idx = (uint8_t)c - UTF8_OFFSET;
  if (idx < 0 || idx > UTF8_MAX) {
    return false;
  }
  return utf8_lut[idx] & UTF8_ALPHA;
}

bool utf8_is_uppercase(const char c) {
  int idx = (uint8_t)c - UTF8_OFFSET;
  if (idx < 0 || idx > UTF8_MAX) {
    return false;
  }
  return utf8_lut[idx] & UTF8_UPPERCASE;
}

bool utf8_is_lowercase(const char c) {
  int idx = (uint8_t)c - UTF8_OFFSET;
  if (idx < UTF8_OFFSET || idx > UTF8_MAX) {
    return false;
  }
  return utf8_lut[idx] & UTF8_LOWERCASE;
}

char utf8_uppercase_to_lowercase(char c) {
  if (utf8_is_lowercase(c)) {
    return c;
  }
  return c + UTF8_CAPS_OFFSET;
}

char utf8_lowercase_to_uppercase(char c) {
  if (utf8_is_uppercase(c)) {
    return c;
  }
  return c - UTF8_CAPS_OFFSET;
}
