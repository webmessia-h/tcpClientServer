#include "../../include/network.hpp"
#include "../../include//client.hpp"
#include "../../include/server.hpp"
#include <netinet/in.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <cerrno>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

#endif
int Network::create_socket(int domain, int type, int protocol) {
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

// Create server socket
bool Network::create_server(int &server_sockfd, struct sockaddr_in &server_addr,
                            const char *ip, int port) {
  if (server_sockfd > 0) {
    Network::close_socket(server_sockfd);
  }
  /*int yes = 1;
  if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) <
      0) {
    std::cerr << "Error setting SO_REUSEADDR" << std::endl;
    Network::close_socket(server_sockfd);
    return false;
  }*/

  server_sockfd = Network::create_socket(PF_INET, SOCK_STREAM, 0);
  if (server_sockfd < 0) {
    std::cerr << "Error: Failed to create server socket " << strerror(errno)
              << std::endl;
    Network::close_socket(server_sockfd);
    return false;
  }
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ip);
  server_addr.sin_port = htons(port);
  return true;
}
bool Network::bind_to_port(int port, int &server_sockfd,
                           struct sockaddr_in &server_addr) {
  // close and reopen socket
  server_addr.sin_port = htons(port);
  std::cout << "Server started on port: " << port << std::endl;
  if (bind(server_sockfd, reinterpret_cast<struct sockaddr *>(&server_addr),
           sizeof(server_addr)) < 0) {
    std::cerr << "Error: Failed to bind " << strerror(errno) << std::endl;
    Network::close_socket(server_sockfd);
    return false;
  }
  return true;
}

// Listen for incoming connections
bool Network::listen_client(int &server_sockfd) {
  if (listen(server_sockfd, 1) < 0) {
    std::cerr << "Error: Failed to listen " << strerror(errno) << std::endl;
    Network::close_socket(server_sockfd);
    return false;
  }
  return true;
}

// Accept incoming connections
int Network::accept_connection(int &server_sockfd, int &communication_sockfd) {
  // Create communication socket
  if (communication_sockfd > 0) {
    Network::close_socket(communication_sockfd);
  }
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  communication_sockfd =
      accept(server_sockfd, reinterpret_cast<struct sockaddr *>(&client_addr),
             &client_addr_len);
  if (communication_sockfd < 0) {
    std::cerr << "Error: Connection was not accepted " << strerror(errno)
              << std::endl;
    return -1;
  }
  const char *msg = "Connection was established";
  std::cout << "Connection was established" << std::endl;
  send_data(communication_sockfd, msg, strlen(msg));
  return communication_sockfd;
}

// Create client
void Network::create_client(int &client_sockfd) {
  if (client_sockfd != 0) {
    close_socket(client_sockfd);
  }
  client_sockfd = socket(PF_INET, SOCK_STREAM, 0);
  if (client_sockfd < 0) {
    std::cerr << "Error: Failed to create client socket " << strerror(errno)
              << std::endl;
  }
}

bool Network::connect_to_server(int &client_sockfd,
                                struct sockaddr_in &server_addr, const char *ip,
                                int port) {
  // Create socket
  if (client_sockfd > 0) {
    Network::close_socket(client_sockfd);
  }
  client_sockfd = Network::create_socket(AF_INET, SOCK_STREAM, 0);
  // Get server's ip and port number
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = inet_addr(ip);
  // Try to connect to server
  if (connect(client_sockfd, reinterpret_cast<sockaddr *>(&server_addr),
              sizeof(server_addr)) < 0) {
    // Failed to connect
    std::cerr << "Error: Client failed to connect to:"
              << inet_ntoa(server_addr.sin_addr) << " on port: " << port
              << strerror(errno) << std::endl;
    close_socket(client_sockfd);
    return false;
  }
  std::cout << "Client attempted to connect to: "
            << inet_ntoa(server_addr.sin_addr) << " on port: " << port
            << std::endl;
  char msg[27] = {0};
  int bytes_received = receive_data(client_sockfd, &msg, sizeof(msg));
  std::cout.write(msg, bytes_received);
  std::cout << "\n";
  return true;
}

/*bool Network::create_connection(const char *ip, int port, class Server
&server, class Client &client) { server.start_server(); server.bind_to_port();
  server.listen_client();
  client.connect_to_server();
  if (server.accept_connection() > 0) {
    std::cout << "Connection established" << std::endl;
    return true;
  } else {
    return false;
    std::cerr << "Error: Failed to establish connection" << strerror(errno)
              << std::endl;
  }
}*/

size_t Network::send_data(int socket_fd, const void *data, size_t size) {
  ssize_t bytes_sent = send(socket_fd, data, size, 0);
  if (bytes_sent < 0) {
    // Error sending data
    std::cerr << "Error: No bytes sent." << std::endl;
    return -1;
  }
  // std::cout << "Bytes sent: " << bytes_sent << std::endl;
  return bytes_sent;
}

size_t Network::receive_data(int socket_fd, void *buffer, size_t size) {
  ssize_t bytes_received = recv(socket_fd, buffer, size, 0);
  if (bytes_received < 0) {
    // Error sending data
    std::cerr << "Error: No bytes received." << std::endl;
    return -1;
  }
  // std::cout << "Bytes received: " << bytes_received << std::endl;
  return bytes_received;
}

void Network::close_socket(int socket_fd) {
#ifdef _WIN32
  closesocket(socket_fd);
  WSACleanup();
#else
  close(socket_fd);
  std::cout << "Socket was closed" << std::endl;
#endif
}
