#pragma once

#include <stdint.h>
#include <stdio.h>

struct in_addr {
  uint32_t s_addr;
};

typedef uint32_t socklen_t;
typedef uint8_t sa_family_t;
typedef uint16_t in_port_t;

struct sockaddr_in {
  uint8_t sin_len;
  sa_family_t sin_family;
  in_port_t sin_port;
  struct in_addr sin_addr;
  char sin_zero[8];
};
struct sockaddr {
  uint8_t sa_len;
  sa_family_t sa_family;
  char sa_data[14];
};

#define SOCK_STREAM 1

extern int errno;

uint16_t htons(uint16_t n);

int socket(int domain, int type, int protocol);
int listen(int s, int backlog);
int bind(int s, struct sockaddr *name, socklen_t namelen);
int accept(int s, struct sockaddr *addr, socklen_t *addrlen);
ssize_t write(int s, const void *data, size_t size);
int close(int s);