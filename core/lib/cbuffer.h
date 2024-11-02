/**
 * @file cbuffer.h
 * @author ztnel (christian911@sympatico.ca)
 * @brief A lightweight circular buffer API
 * @version 0.1
 * @date 2022-10
 *
 * @copyright Copyright © 2022 Christian Sargusingh
 *
 */

#ifndef __CBUFFER_H__
#define __CBUFFER_H__

#include <stddef.h>

typedef int cbuffer_status_t;

#define CBUFFER_SUCCESS (cbuffer_status_t)0
#define CBUFFER_ERR (cbuffer_status_t)1
#define CBUFFER_EMPTY (cbuffer_status_t)2
#define CBUFFER_OVERFLOW (cbuffer_status_t)3
#define CBUFFER_UNDERFLOW (cbuffer_status_t)4

struct cbuffer_handle {
  void *buffer;     // buffer data
  size_t size;      // buffer size
  size_t elem_size; // element size
  size_t head;      // head position idx
  size_t tail;      // tail position idx
};

void cbuffer_init(struct cbuffer_handle *cbuffer, void *buffer, size_t elem_size, size_t size);
cbuffer_status_t cbuffer_push(struct cbuffer_handle *cbuffer, const void *data);
cbuffer_status_t cbuffer_pop(struct cbuffer_handle *cbuffer, void *data);

#endif // __CBUFFER_H__
