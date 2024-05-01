#include "../../include/server.hpp"
#include "../../include/network.hpp"
#include "../../include/platform.hpp"
#include <cerrno>
#include <iomanip>

Server::Server(const char *ip, const int port) : port{port}, ip{ip} {}

bool Server::receive_file_list() {
  char buffer[1024] = {0};
  int bytes_received =
      Network::receive_data(communication_sockfd, buffer, sizeof(buffer));
  if (bytes_received == 0) {
    std::cerr << "Error: Client failed to provide listed directory"
              << std::endl;
    return false;
  }
  std::cout.write(buffer, bytes_received);
  std::cout << "\n";
  return true;
}

bool Server::request_upload(const char *filename) {
  // std::cout << "request_upload called" << std::endl;
  if (!Network::send_data(communication_sockfd, filename,
                          strlen(filename) + 1)) {
    std::cerr << "Error: Failed to provide client with a filename "
              << strerror(errno) << std::endl;
    return false;
  };
  /*if (!change_port()) {
    std::cerr << "Error: Failed to change port ";
    return false;
  }*/
  std::cout << "Requested upload of a " << filename << " on a port "
            << ntohs(server_addr.sin_port) << std::endl;
  return true;
};

// Change port
bool Server::change_port() {
  int newPort = 0;
  if (server_addr.sin_port != htons(8400)) {
    newPort = 8400;
  } else {
    newPort += htons(5);
  }
  // Notify the client about new port CHPORT
  if (!Network::send_data(communication_sockfd, &newPort, sizeof(newPort))) {
    return false;
    std::cerr << "Error: Failed to notify client about new port"
              << strerror(errno) << std::endl;
  }
  port = newPort;
  std::cout << "Port changed to: " << port << std::endl;
  Network::close_socket(server_sockfd);
  Network::close_socket(communication_sockfd);
  Server::~Server();
  return true;
}

// Receive the file
bool Server::receive_file(int communication_sockfd) {
  size_t file_size;
  Network::receive_data(communication_sockfd, &file_size, sizeof(file_size));
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
    size_t bytes_received = Network::receive_data(
        communication_sockfd, buffer + total_bytes_received, file_size);
    total_bytes_received += bytes_received;
    if (bytes_received == 0) {
      std::cout << "End of file tranfer" << std::endl;
      break;
      return 0;
    }
    // Calculate percentage
    int new_percentage = static_cast<int>(
        static_cast<double>(total_bytes_received) / file_size * 100.0);
    if (new_percentage != percentage && new_percentage % 10 == 0) {
      percentage = new_percentage;

      // Print the progress bar
      int num_steps = 50;
      int completed_steps =
          static_cast<int>(static_cast<double>(percentage) / 100.0 * num_steps);
      std::cout << "\rTransfer progress: [" << std::string(completed_steps, '=')
                << std::string(num_steps - completed_steps, ' ') << "] "
                << percentage << "%" << std::flush;
    }

  } // Change of a port
  /*if (percentage % 10 == 0 && percentage != 0) {
  change_port(p_serv_addr, server_sockfd);
}*/
  // Write file from buffer
  std::cout << std::endl;
  write_file(filename, buffer, file_size);
  ::cleanup_handler(buffer);
  return true;
};

Server::~Server() {
  Network::close_socket(server_sockfd);
  Network::close_socket(communication_sockfd);
}
