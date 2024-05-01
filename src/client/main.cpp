#include "../../include/client.hpp"
#include <iostream>
int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << "<server_ip> <port_number>"
              << std::endl;
    return 1;
  }

  const char *ip = argv[1];
  int port = std::stoi(argv[2]);
  char folder_path[512] = {0};

  Client *cl = new Client(ip, port);

  Network::create_client(cl->client_sockfd);
  Network::connect_to_server(cl->client_sockfd, cl->server_addr, cl->ip,
                             cl->port);
  std::cout << "Specify folder path to list: ";
  std::cin >> folder_path;
  cl->send_file_list(folder_path);
  cl->handle_request();

  delete cl;
  return 0;
}
