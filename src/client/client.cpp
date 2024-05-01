#include "../../include/client.hpp"
#include "../../include/network.hpp"
#include "../../include/platform.hpp"
#include <iomanip>
Client::Client(const char *ip, int port)
    : ip(std::move(ip)), port(std::move(port)) {}

/*
// Connect to new port
bool connect_to_new_port(sockaddr_in *p_serv_addr) {
  // reinitialize socket
  close_socket(client_sockfd);
  client_sockfd = create_socket(AF_INET, SOCK_STREAM, 0);
  if (connect(client_sockfd, (struct sockaddr *)p_serv_addr,
              sizeof(*p_serv_addr)) < 0) {
    // Failed to connect
    std::cerr << "Error: Client failed to connect to: "
              << inet_ntoa(p_serv_addr->sin_addr)
              << " on port: " << ntohs(p_serv_addr->sin_port) << strerror(errno)
              << std::endl;
    close_socket(client_sockfd);
    return false;
  }
  std::cout << "Client connected to: " << inet_ntoa(p_serv_addr->sin_addr)
            << " on port: " << ntohs(p_serv_addr->sin_port) << std::endl;
  return true;
}*/

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
bool Client::get_new_port() {
  in_port_t port = 0;
  Network::receive_data(client_sockfd, &port, sizeof(port));
  std::cout << "new port: " << ntohs(port) << std::endl;
  // if (!connect_to_new_port(p_serv_addr))
  //   return false;
  return true;
}

// send file with dynamic port change
void Client::send_file(const file_info &FILE) {
  // send file size
  Network::send_data(client_sockfd, &FILE.file_size, sizeof(FILE.file_size));

  // send file
  size_t total_bytes_sent = 0;
  while (total_bytes_sent < FILE.file_size) {
    size_t remaining_bytes = FILE.file_size - total_bytes_sent;
    size_t chunk_size = remaining_bytes > 1024 ? 1024 : remaining_bytes;
    size_t bytes_sent = Network::send_data(
        client_sockfd, FILE.buffer + total_bytes_sent, chunk_size);
    total_bytes_sent += bytes_sent;
  }
  cleanup_handler(FILE.buffer);
}
bool Client::handle_request() {
  // get filename
  char filename[1024] = {0};
  Network::receive_data(client_sockfd, filename, sizeof(filename));
  // get new port
  // Client::get_new_port();
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

Client::~Client() { Network::close_socket(client_sockfd); }
