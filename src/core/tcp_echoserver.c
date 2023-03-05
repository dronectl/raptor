#include "tcp_echoserver.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"

void tcp_server_task(void *pv_params) {
  int sock, newconn, size;
  struct sockaddr_in address, remotehost;

  /* create a TCP socket */
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    return;
  }

  /* bind to port 80 at any interface */
  address.sin_family = AF_INET;
  address.sin_port = htons(7);
  address.sin_addr.s_addr = INADDR_ANY;

  if (bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
    return;
  }

  /* listen for incoming connections (TCP listen backlog = 5) */
  listen(sock, 5);

  size = sizeof(remotehost);

  while (1) {
    newconn = accept(sock, (struct sockaddr *)&remotehost, (socklen_t *)&size);
    if (newconn < 0) {
      // Error in accepting connection
      continue;
    }
    char buffer[1024];
    ssize_t bytes_received = recv(newconn, buffer, sizeof(buffer), 0);
    if (bytes_received < 0) {
      // Error in receiving data
      close(newconn);
      continue;
    } else if (bytes_received == 0) {
      // Client disconnected
      close(newconn);
      continue;
    }
    ssize_t bytes_sent = send(newconn, buffer, bytes_received, 0);
    if (bytes_sent < 0) {
      // Error in sending data
      close(newconn);
      continue;
    }
    // Close the connection
    close(newconn);
  }
}
