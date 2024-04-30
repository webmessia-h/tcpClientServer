#include "../../include/server.hpp"
#include "../../include/network.hpp"
#include "../../include/platform.hpp"
#include <cerrno>

Server::Server(const char *ip, const int port) : port{port}, ip{ip} {}

bool Server::start_server() {
  if (server_sockfd > 0) {
    Network::close_socket(server_sockfd);
  }
  int yes = 1;
  if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) <
      0) {
    std::cerr << "Error setting SO_REUSEADDR" << std::endl;
    Network::close_socket(server_sockfd);
    return false;
  }

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
bool Server::bind_to_port() {
  server_addr.sin_port = htons(port);
  std::cout << port << std::endl;
  if (bind(server_sockfd, reinterpret_cast<struct sockaddr *>(&server_addr),
           sizeof(server_addr)) < 0) {
    std::cerr << "Error: Failed to bind " << strerror(errno) << std::endl;
    Network::close_socket(server_sockfd);
    return false;
  }
  return true;
}

bool Server::listen_client() {
  if (listen(Server::server_sockfd, 1) < 0) {
    std::cerr << "Error: Failed to listen " << strerror(errno) << std::endl;
    Network::close_socket(Server::server_sockfd);
    return false;
  }
  return true;
}

// Accept incoming connections
int Server::accept_connection() {
  // Create communication socket
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  int communication_sockfd =
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

bool Server::receive_file_list() {
  char buffer[1024] = {0};
  int bytes_received =
      receive_data(communication_sockfd, buffer, sizeof(buffer));
  std::cout.write(buffer, bytes_received);
  std::cout << "\n";
  return true;
}

bool Server::request_upload(const char *filename) {
  // std::cout << "request_upload called" << std::endl;
  if (!send_data(communication_sockfd, filename, strlen(filename) + 1)) {
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
  int newPort;
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
  Server::start_server();
  return true;
}

Server::~Server() {
  close_socket(server_sockfd);
  close_socket(communication_sockfd);
}
/*
  // Receive the file
  bool receive_file(int communication_sockfd, int server_sockfd) {
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
    delete[] buffer;
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

    // listen_on_port(host, port, p_serv_addr);
    // accept_connection(server_sockfd);
    //  receive_file_list(communication_sockfd, buffer, sizeof(buffer));

    char filename[255] = {0};
    std::cout << "choose file to upload" << std::endl;
    std::cin >> filename;

    // request_upload(communication_sockfd, filename, p_serv_addr,
  server_sockfd);
    // receive_file(communication_sockfd);
    return 0;
  }
  */
