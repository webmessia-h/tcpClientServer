// client
#pragma once
#include "network.hpp"
#include "platform.hpp"
class Client : Network {
public:
  Client(const char *ip, int port);
  ~Client();
  void create_client();
  bool connect_to_server();
  bool get_new_port();
  void send_file_list(const char *folder_path);
  bool parse_message(std::string message, int &intPort);
  bool handle_request();
  void send_file(const file_info &FILE);

private:
  const char *ip;
  int port, client_sockfd = 0;
  struct sockaddr_in server_addr;
};
