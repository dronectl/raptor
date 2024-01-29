#include "tcp_echoserver.h"
#include "logger.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"

void tcp_server_task(void *pv_params) {
  int sock, newconn, size;
  struct sockaddr_in address, remotehost;

  info("Starting tcp echo server ...");
  /* create a TCP socket */
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    critical("tcp echo server failed to create socket");
    return;
  }

  /* bind to port 80 at any interface */
  address.sin_family = AF_INET;
  address.sin_port = htons(7);
  address.sin_addr.s_addr = INADDR_ANY;

  if (bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
    error("tcp echo server failed to bind to socket");
    return;
  }

  /* listen for incoming connections (TCP listen backlog = 5) */
  listen(sock, 5);

  size = sizeof(remotehost);

  while (1) {
    info("Accepting new connections on port: %i", 7);
    newconn = accept(sock, (struct sockaddr *)&remotehost, (socklen_t *)&size);
    if (newconn < 0) {
      warning("failed to accept new connection");
      // Error in accepting connection
      continue;
    }
    trace("Accepted new connection");
    char buffer[1024];
    ssize_t bytes_received = recv(newconn, buffer, sizeof(buffer), 0);
    info("Received data: %s", buffer);
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