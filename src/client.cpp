#include "../include/client.hpp"
#include "../include/network.hpp"
#include "../include/platform.hpp"
#include <iostream>
#include <netinet/in.h>

int client_sockfd;
struct sockaddr_in server_addr;
sockaddr_in *p_serv_addr = &server_addr;

// Initialize CLIENT and send connection request
bool connect_to_server(const char *host, int port) {
  // Create socket
  client_sockfd = create_socket(AF_INET, SOCK_STREAM, 0);
  // Get server's ip and port number
  p_serv_addr->sin_family = AF_INET;
  p_serv_addr->sin_port = htons(port);
  p_serv_addr->sin_addr.s_addr = inet_addr(host);

  // Try to connect to server
  if (connect(client_sockfd, (struct sockaddr *)p_serv_addr,
              sizeof(*p_serv_addr)) < 0) {
    // Failed to connect
    std::cerr << "Error: Client failed to connect to:"
              << inet_ntoa(p_serv_addr->sin_addr) << " :" << port
              << strerror(errno) << std::endl;
    close_socket(client_sockfd);
    return false;
  }
  std::cout << "Client connected to: " << inet_ntoa(p_serv_addr->sin_addr)
            << " on port: " << port << std::endl;
  return true;
}
// Connect to new port
bool connect_to_new_port(sockaddr_in *p_serv_addr) {
  // Try to connect to server
  int yes = 1;
  if (setsockopt(client_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) <
      0) {
    std::cerr << "Error setting SO_REUSEADDR" << std::endl;
    close_socket(client_sockfd);
    return false;
  }
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
}

// Send listed directory
void send_file_list(const char *folder_path, int socket_fd) {
  std::string file_info = {0};
  std::vector<std::pair<std::string, long long>> files =
      // Function to make a pair of filename and its size
      list_directory(folder_path);
  // Actually send list
  for (const auto &file : files) {
    file_info += file.first + " - " +
                 std::to_string((file.second) / (1024 * 1024)) + "MB\n";
  }
  send_data(socket_fd, file_info.c_str(), file_info.size());
}

// get and set new port
bool get_new_port(int socket_fd) {
  in_port_t port = 0;
  receive_data(socket_fd, &port, sizeof(port));
  p_serv_addr->sin_port = port;
  std::cout << "new port: " << ntohs(port) << std::endl;
  if (!connect_to_new_port(p_serv_addr))
    return false;
  return true;
}

// send file with dynamic port change
void send_file(int socket_fd, const file_info &FILE) {
  // send file size
  send_data(socket_fd, &FILE.file_size, sizeof(FILE.file_size));

  // send file in 1MB chunks
  size_t total_bytes_sent = 0;
  const size_t CHUNK_SIZE = 1024 * 1024 * 50; // 1MB chunk size
  while (total_bytes_sent < FILE.file_size) {
    size_t remaining_bytes = FILE.file_size - total_bytes_sent;
    size_t chunk_size =
        (remaining_bytes > CHUNK_SIZE) ? CHUNK_SIZE : remaining_bytes;

    size_t bytes_sent =
        send_data(socket_fd, FILE.buffer + total_bytes_sent, chunk_size);
    total_bytes_sent += bytes_sent;

    // Check for port change
    if (!get_new_port(socket_fd))
      return;
  }

  delete[] FILE.buffer;
}
void handle_request(int socket_fd) {
  // get filename
  char filename[1024] = {0};
  receive_data(socket_fd, filename, sizeof(filename));
  // std::cout << filename << std::endl;

  // get new port
  get_new_port(socket_fd);

  // write file into buffer
  file_info FILE = read_file(filename);
  if (FILE.buffer == nullptr) {
    std::cerr << "Error: Failed to deliver file to client " << strerror(errno)
              << std::endl;
    return;
  }

  send_file(socket_fd, FILE);
  // delete[] FILE.buffer;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <server_ip> <port_number>"
              << std::endl;
    return 1;
  }

  const char *host = argv[1];
  int port = std::stoi(argv[2]);
  if (!connect_to_server(host, port))
    return -1;
  char folder[512] = {0};
  std::cout << "Specify folder to list: ";
  std::cin >> folder;

  send_file_list(folder, client_sockfd);
  handle_request(client_sockfd);
  return 0;
}
