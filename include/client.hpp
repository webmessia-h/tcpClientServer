// client
#pragma once
#include "network.hpp"
#include "platform.hpp"
class Client {
public:
  Client(const char *ip, int port);
  ~Client();
  friend class Network;
  int get_new_port();
  void send_file_list(const char *folder_path);
  bool handle_request();
  void send_file(const file_info &FILE);

  const char *ip;
  int port, client_sockfd, transfer_sockfd;
  struct sockaddr_in server_addr;
};
