#include "../../include/client.hpp"
#include "../../include/network.hpp"
#include "../../include/platform.hpp"
#include <linux/close_range.h>
#include <sys/socket.h>
Client::Client(const char *ip, int port)
    : ip(std::move(ip)), port(std::move(port)) {}

// Send listed directory
void Client::send_file_list(const char *folder_path) {
  std::string file_info = {0};
  std::vector<std::pair<std::string, long long>> files =
      // Function to make a pair of filename and its size
      list_directory(folder_path);
  if (files.empty() == 1) {
    return;
  }
  // Actually send list
  for (const auto &file : files) {
    file_info +=
        file.first + " - " +
        std::to_string(static_cast<float>(file.second) / (1024 * 1024)) +
        "MB\n";
  }
  Network::send_data(client_sockfd, file_info.c_str(), file_info.size());
}

// get and set new port
int Client::get_new_port() {
  if (transfer_sockfd > 0) {
    Network::close_socket(transfer_sockfd);
  }
  transfer_sockfd = Network::create_socket(PF_INET, SOCK_STREAM, 0);
  int port = 0;
  Network::receive_data(client_sockfd, &port, sizeof(port));
  // std::cout << "new port: " << (port) << std::endl;
  if (!Network::connect_to_server(transfer_sockfd, server_addr, ip, port)) {
    return false;
  }
  return transfer_sockfd;
}

// send file with dynamic port change
void Client::send_file(const file_info &FILE) {
  // send file size
  Network::send_data(transfer_sockfd, &FILE.file_size, sizeof(FILE.file_size));
  // send file
  size_t total_bytes_sent = 0;
  while (total_bytes_sent < FILE.file_size) {
    char msg[4] = {0};
    Network::receive_data(client_sockfd, msg, sizeof(msg));
    // std::cout << "\r" << msg << "\t" << std::flush;
    if (msg[0] == 'C') {
      transfer_sockfd = get_new_port();
      continue;
    } else if (msg[0] == 'A') {
      size_t remaining_bytes = FILE.file_size - total_bytes_sent;
      size_t chunk_size = remaining_bytes > 1024 ? 1024 : remaining_bytes;
      size_t bytes_sent = Network::send_data(
          transfer_sockfd, FILE.buffer + total_bytes_sent, chunk_size);
      total_bytes_sent += bytes_sent;
    }
  }
  cleanup_handler(FILE.buffer);
}

// handle upload request
bool Client::handle_request() {
  // get filename
  char filename[1024] = {0};
  Network::receive_data(client_sockfd, filename, sizeof(filename));
  Network::create_client(transfer_sockfd);
  // get new port
  Client::get_new_port();
  // write file into buffer
  file_info FILE = read_file(filename);
  if (FILE.buffer == nullptr) {
    std::cerr << "Error: Failed to deliver file to client " << strerror(errno)
              << std::endl;
    return false;
  }

  send_file(FILE);
  return true;
}

Client::~Client() {
  Network::close_socket(client_sockfd);
  Network::close_socket(transfer_sockfd);
}
