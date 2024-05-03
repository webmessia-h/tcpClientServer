#include "../../include/server.hpp"
#include "../../include/network.hpp"
#include "../../include/platform.hpp"
#include <cerrno>
#include <cstring>
#include <string>
#include <sys/socket.h>

Server::Server(const char *ip, const int port) : port{port}, ip{ip} {}

// Receive file list
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

// Request upload of a file
bool Server::request_upload(const char *filename) {
  if (!Network::send_data(communication_sockfd, filename,
                          strlen(filename) + 1)) {
    std::cerr << "Error: Failed to provide client with a filename "
              << strerror(errno) << std::endl;
    return false;
  };
  if (!change_port()) {
    std::cerr << "Error: Failed to change port ";
    return false;
  }
  std::cout << "Requested upload of a " << filename << " on a port "
            << ntohs(server_addr.sin_port) << std::endl;
  return true;
};

// Change port
bool Server::change_port() {

  if (transfer_sockfd > 0) {
    Network::close_socket(transfer_sockfd);
  }

  transfer_sockfd = Network::create_socket(PF_INET, SOCK_STREAM, 0);

  int yes = 1;
  if (setsockopt(transfer_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) <
      0) {
    std::cerr << "Error setting SO_REUSEADDR" << std::strerror(errno)
              << std::endl;
    Network::close_socket(transfer_sockfd);
    return false;
  }

  if (port < 8400) {
    port = 8400;
  } else {
    port += 5;
  }

  Network::bind_to_port(port, transfer_sockfd, server_addr);

  Network::listen_client(transfer_sockfd);

  // Send new port to client
  if (!Network::send_data(communication_sockfd, &port, sizeof(port))) {
    return false;
    std::cerr << "Error: Failed to notify client about new port"
              << strerror(errno) << std::endl;
  }
  Network::accept_connection(transfer_sockfd, transfer_sockfd);
  std::cout << "Port changed to: " << ntohs(server_addr.sin_port) << std::endl;
  return true;
}

// Receive the file
bool Server::receive_file(int transfer_sockfd) {
  // Messages for client
  const char ACK[4] = "ACK";    // continue transfer
  const char CHPORT[4] = "CHP"; // change port
  // Get size of file
  size_t file_size;
  Network::receive_data(transfer_sockfd, &file_size, sizeof(file_size));
  char *buffer = new char[file_size];
  std::cout << "Size of file: " << file_size / (1024 * 1024) << "MB"
            << std::endl;
  // Specify the location and filename
  char filename[255] = {0};
  std::cout << "Choose a name and optionally location for a file: ";
  std::cin >> filename;
  // For dynamic port change and progress bar
  int percentage = 0;
  size_t total_bytes_received = 0;
  // Receive the file contents
  while (total_bytes_received < file_size) {
    // Calculate percentage
    percentage = static_cast<int>(static_cast<double>(total_bytes_received) *
                                  100.0 / file_size);
    // Send ACK for start of the upload
    Network::send_data(communication_sockfd, ACK, sizeof(ACK));
    // To determine loop conditions and size of received package (1KB)
    size_t remaining_bytes = file_size - total_bytes_received;
    size_t chunk_size = remaining_bytes > 1024 ? 1024 : remaining_bytes;
    // Actually receive chunk of file
    size_t bytes_received = Network::receive_data(
        transfer_sockfd, buffer + total_bytes_received, chunk_size);
    total_bytes_received += bytes_received;
    if (bytes_received == 0) {
      std::cout << "End of file tranfer" << std::endl;
      break;
      return false;
    }
    // Check if it's time to change port
    int new_percentage = static_cast<int>(
        static_cast<double>(total_bytes_received) / file_size * 100.0);
    if (new_percentage != percentage) {
      percentage = new_percentage;
      if (percentage % 10 == 0 && percentage != 0) {
        Network::send_data(communication_sockfd, CHPORT, sizeof(CHPORT));
        change_port();
      }
      // Print the progress bar
      int num_steps = 25;
      int completed_steps =
          static_cast<int>(static_cast<double>(percentage) / 100.0 * num_steps);
      std::cout << "\rTransfer progress: [" << std::string(completed_steps, '=')
                << std::string(num_steps - completed_steps, ' ') << "] "
                << percentage << "%" << std::flush;
    }
  }

  // Write file from buffer
  std::cout << std::endl;
  write_file(filename, buffer, file_size);
  ::cleanup_handler(buffer);
  return true;
};

Server::~Server() {
  Network::close_socket(server_sockfd);
  Network::close_socket(communication_sockfd);
  Network::close_socket(transfer_sockfd);
};
