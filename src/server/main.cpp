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

  while (true) {
    srv->start_server();
    srv->bind_to_port();
    srv->listen_client();
    srv->accept_connection();
    srv->receive_file_list();
    std::cout << "Choose file to upload: ";
    std::cin >> filename;
    srv->request_upload(filename);
    srv->receive_file();
  }
  delete srv;
  return 0;
}
