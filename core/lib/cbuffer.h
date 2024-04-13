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

#ifndef __CBUF_H__
#define __CBUF_H__

#include <stddef.h>
#include <stdint.h>

typedef int cbuffer_status_t;

#define CBUFFER_SUCCESS (cbuffer_status_t)0
#define CBUFFER_ERR (cbuffer_status_t)1
#define CBUFFER_OVERFLOW (cbuffer_status_t)2
#define CBUFFER_UNDERFLOW (cbuffer_status_t)3

typedef struct cbuffer_t {
  void *buffer;     // buffer data
  size_t size;      // buffer size
  size_t elem_size; // element size
  size_t head;      // head position idx
  size_t tail;      // tail position idx
} cbuffer_t;

void cbuffer_init(cbuffer_t *cbuffer, void *buffer, size_t elem_size,
                  size_t size);
cbuffer_status_t cbuffer_write(cbuffer_t *cbuffer, const void *data);
void *cbuffer_get(cbuffer_t *cbuffer);

#endif // __CBUF_H__