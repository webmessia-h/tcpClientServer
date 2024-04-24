#include "../include/network.hpp"
#ifdef _WIN32
#include <winsock2.h>
#else
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

#endif
int create_socket(int domain, int type, int protocol) {
#ifdef _WIN32
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    // Failed to initialize Winsock
    return -1;
  }
#endif

  int sockfd = socket(domain, type, protocol);
  if (sockfd < 0) {
    std::cerr << "Error: Failed to create socket" << std::endl;
#ifdef _WIN32
    WSACleanup();
#endif
  }
  return sockfd;
}

size_t send_data(int socket_fd, const void *data, size_t size) {
  ssize_t bytes_sent = send(socket_fd, data, size, 0);
  if (bytes_sent < 0) {
    // Error sending data
    std::cerr << "Error: No bytes sent." << std::endl;
    return -1;
  }
  // std::cout << "Bytes sent: " << bytes_sent << std::endl;
  return bytes_sent;
}

size_t receive_data(int socket_fd, void *buffer, size_t size) {
  ssize_t bytes_received = recv(socket_fd, buffer, size, 0);
  if (bytes_received < 0) {
    // Error sending data
    std::cerr << "Error: No bytes received." << std::endl;
    return -1;
  }
  // std::cout << "Bytes received: " << bytes_received << std::endl;
  return bytes_received;
}

void close_socket(int socket_fd) {
#ifdef _WIN32
  closesocket(socket_fd);
  WSACleanup();
#else
  close(socket_fd);
  std::cout << "Socket was closed" << std::endl;
#endif
}
