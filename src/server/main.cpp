#include "../../include/server.hpp"
#include <iostream>
int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << "<server_ip> <port_number>"
              << std::endl;
    return 1;
  }

  const char *ip = argv[1];
  int port = std::stoi(argv[2]);
  char filename[255] = {0};

  Server *srv = new Server(ip, port);

  Network::create_server(srv->server_sockfd, srv->server_addr, srv->ip,
                         srv->port);
  Network::bind_to_port(srv->port, srv->server_sockfd, srv->server_addr);
  Network::listen_client(srv->server_sockfd);
  Network::accept_connection(srv->server_sockfd, srv->communication_sockfd);

  if (srv->receive_file_list() != 0) {
    std::cout << "Choose file to upload: ";
    std::cin >> filename;
    srv->request_upload(filename);
    srv->receive_file(srv->transfer_sockfd);
  }
  // srv->receive_file();
  srv->~Server();
  delete srv;
  return 0;
}
