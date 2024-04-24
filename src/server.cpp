#include "../include/server.hpp"
#include "../include/network.hpp"
#include "../include/platform.hpp"
#include <iostream>
#include <netinet/in.h>

int server_sockfd;        // server's socket
int communication_sockfd; // communication socket
struct sockaddr_in server_addr;
sockaddr_in *p_serv_addr = &server_addr;
char buffer[4096] = {0};

// Initialize server, bind it, listen for connections
bool listen_on_port(const char *host, int port, sockaddr_in *p_serv_addr) {
  // Create server socket
  server_sockfd = create_socket(AF_INET, SOCK_STREAM, 0);
  // Initialize server
  p_serv_addr->sin_family = AF_INET;
  p_serv_addr->sin_port = htons(port);
  p_serv_addr->sin_addr.s_addr = inet_addr(host);
  // Bind server socket
  if (bind(server_sockfd, (struct sockaddr *)p_serv_addr,
           sizeof(*p_serv_addr)) < 0) {
    std::cerr << "Error: Failed to bind " << strerror(errno) << std::endl;
    close_socket(server_sockfd);
    return false;
  }
  // Listen for incoming connections
  if (listen(server_sockfd, 1) < 0) {
    std::cerr << "Error: Failed to listen " << strerror(errno) << std::endl;
    close_socket(server_sockfd);
    return false;
  }
  // Server started successfully
  std::cout << "Server started on port " << port << " and adress " << host
            << std::endl;
  return true;
}

// Listen on new port
bool listen_on_new_port(sockaddr_in *p_serv_addr) {
  // setsockopt to allow binding to already-in-use adress
  int yes = 1;
  if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) <
      0) {
    std::cerr << "Error setting SO_REUSEADDR" << std::endl;
    close_socket(server_sockfd);
    return false;
  }
  close_socket(server_sockfd);
  // Bind server socket to new port
  server_sockfd = create_socket(AF_INET, SOCK_STREAM, 0);
  if (bind(server_sockfd, (struct sockaddr *)p_serv_addr,
           sizeof(*p_serv_addr)) < 0) {
    std::cerr << "Error: Failed to bind " << strerror(errno) << std::endl;
    close_socket(server_sockfd);
    return false;
  }
  // Listen for incoming connections
  if (listen(server_sockfd, 1) < 0) {
    std::cerr << "Error: Failed to listen " << strerror(errno) << std::endl;
    close_socket(server_sockfd);
    return false;
  }
  return true;
}

// Accept incoming connections
int accept_connection(int listening_sockfd) {
  // Create communication socket
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  communication_sockfd = accept(
      listening_sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
  if (communication_sockfd < 0) {
    // Failed to accept connection
    std::cerr << "Error: Connection was not accepted " << strerror(errno)
              << std::endl;
    return -1;
  }
  std::cout << "Connection was accepted." << std::endl;
  return communication_sockfd;
}

// Change prot
bool change_port(sockaddr_in *p_serv_addr, int server_sockfd) {
  if (p_serv_addr->sin_port != htons(8400)) {
    p_serv_addr->sin_port = htons(8400);
  } else {
    p_serv_addr->sin_port += htons(5);
  }
  // Notify the client about new port
  if (!send_data(communication_sockfd, &p_serv_addr->sin_port,
                 sizeof(p_serv_addr->sin_port))) {
    return false;
  }
  // Set up the new port
  if (!listen_on_new_port(p_serv_addr)) {
    return false;
  }
  // Accept connection on new port
  if (accept_connection(server_sockfd) < 0) {
    return false;
  }
  std::cout << "Port changed to: " << ntohs(p_serv_addr->sin_port) << std::endl;
  return true;
}

// Receive the file
bool receive_file(int communication_sockfd) {
  size_t file_size;
  receive_data(communication_sockfd, &file_size, sizeof(file_size));
  char *buffer = new char[file_size];
  // Size of file
  std::cout << "Size of file: " << file_size / (1024 * 1024) << "MB"
            << std::endl;
  // Specify the location and filename
  char filename[255] = {0};
  std::cout << "Choose a name and optionally location for a file: ";
  std::cin >> filename;
  int percentage = 0;
  size_t total_bytes_received = 0;
  // Receive the file contents
  while (total_bytes_received < file_size) {
    // Receive a chunk of data
    size_t bytes_received =
        receive_data(communication_sockfd, buffer, file_size);
    total_bytes_received += bytes_received;
    // Calculte percentage
    percentage = static_cast<int>(static_cast<double>(total_bytes_received) /
                                  file_size * 100.0);
    // Progress bar
    std::cout << "Transfer progress: " << percentage << "%" << std::endl;
    // Change of a port
    if (percentage % 10 == 0 && percentage != 0) {
      change_port(p_serv_addr, server_sockfd);
    }
  }
  // Write file from buffer
  write_file(filename, buffer, file_size);
  return true;
};

// Receive listed folder
bool receive_file_list(int socket, char *buffer, size_t buffer_size) {
  int bytes_received = receive_data(socket, buffer, buffer_size);
  std::cout.write(buffer, bytes_received);
  std::cout << "\n";
  return true;
}

// Request upload of a file
bool request_upload(int socket, const char *filename, sockaddr_in *p_serv_addr,
                    int server_sockfd) {
  // std::cout << "request_upload called" << std::endl;
  if (!send_data(socket, filename, strlen(filename) + 1)) {
    std::cerr << "Error: Failed to provide client with a filename "
              << strerror(errno) << std::endl;
    return false;
  };
  if (!change_port(p_serv_addr, server_sockfd)) {
    std::cerr << "Error: Failed to change port ";
    return false;
  }
  std::cout << "Requested upload of a " << filename << " on a port "
            << ntohs(p_serv_addr->sin_port) << std::endl;
  return true;
};

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << "<server_ip> <port_number>"
              << std::endl;
    return 1;
  }

  const char *host = argv[1];
  int port = std::stoi(argv[2]);

  listen_on_port(host, port, p_serv_addr);
  accept_connection(server_sockfd);
  receive_file_list(communication_sockfd, buffer, sizeof(buffer));

  char filename[255] = {0};
  std::cout << "choose file to upload" << std::endl;
  std::cin >> filename;

  request_upload(communication_sockfd, filename, p_serv_addr, server_sockfd);
  receive_file(communication_sockfd);
  return 0;
}
