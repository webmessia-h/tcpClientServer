// network
#pragma once
#include <sys/types.h> // For socket types
class Network {
public:
  int create_socket(int domain, int type, int protocol);
  // Create connection between server and client
  bool create_connection(const char *ip, int port);
  // Send data with some logging
  size_t send_data(int socket_fd, const void *data, size_t size);
  // Receive data with some logging
  size_t receive_data(int socket_fd, void *buffer, size_t size);
  // Close socket with some logging
  void close_socket(int socket_fd);
  //
  bool cleanup_handler();
};
#ifdef _WIN32
#include <winsock2.h> // Windows
#define SOCKET int
#else

#include <arpa/inet.h>
#include <netinet/in.h>
#endif
