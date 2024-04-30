#include "../../include/client.hpp"
#include "../../include/network.hpp"
#include "../../include/platform.hpp"
Client::Client(const char *ip, int port)
    : ip(std::move(ip)), port(std::move(port)) {}

void Client::create_client() {
  if (client_sockfd != 0) {
    close_socket(client_sockfd);
  }
  client_sockfd = socket(PF_INET, SOCK_STREAM, 0);
  if (client_sockfd < 0) {
    std::cerr << "Error: Failed to create client socket " << strerror(errno)
              << std::endl;
  }
}

bool Client::connect_to_server() {
  // Create socket
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
  std::cout << "Client connected to: " << inet_ntoa(server_addr.sin_addr)
            << " on port: " << port << std::endl;
  char msg[27] = {0};
  int bytes_received = receive_data(client_sockfd, &msg, sizeof(msg));
  std::cout.write(msg, bytes_received);
  std::cout << "\n";
  return true;
}
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
    size_t bytes_sent = send_data(client_sockfd, FILE.buffer + total_bytes_sent,
                                  remaining_bytes);
    total_bytes_sent += bytes_sent;
    // Check for port change
    if (!get_new_port())
      return;
  }
  // delete[] FILE.buffer;
}
bool Client::handle_request() {
  // get filename
  char filename[1024] = {0};
  Network::receive_data(client_sockfd, filename, sizeof(filename));
  // get new port
  Client::get_new_port();
  // write file into buffer
  file_info FILE = read_file(filename);
  if (FILE.buffer == nullptr) {
    std::cerr << "Error: Failed to deliver file to client " << strerror(errno)
              << std::endl;
    return false;
  }
  return true;
}

Client::~Client() { close_socket(client_sockfd); }

/*int main(int argc, char *argv[]) {
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
}*/
