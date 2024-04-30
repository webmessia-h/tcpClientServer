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
  char folder[512] = {0};

  Client *cl = new Client(ip, port);

  while (true) {
    cl->create_client();
    cl->connect_to_server();
    std::cout << "Specify full path to folder to list: ";
    std::cin >> folder;
    cl->send_file_list(folder);

    // cl->send_list();
  }
}
