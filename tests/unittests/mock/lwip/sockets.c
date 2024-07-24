#include "sockets.h"

int socket(int domain, int type, int protocol) {
  return 1;
}
int listen(int s, int backlog) {
  return 1;
}
int bind(int s, struct sockaddr *name, socklen_t namelen) {
  return 1;
}
ssize_t write(int s, const void *data, size_t size) {
  return 1;
}
int accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
  return 1;
}
int close(int s) {
  return 1;
}