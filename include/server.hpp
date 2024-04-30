// server
#pragma once
#include "network.hpp"
class Server : Network {
public:
  Server(const char *ip, const int port);
  ~Server();
  bool start_server();
  bool bind_to_port();
  bool listen_client();
  int accept_connection();
  // Receive and print listed folder
  bool receive_file_list();
  // Request desired file from the client
  bool request_upload(const char *filename);
  // Handle port change
  bool change_port();
  //  Receive file
  bool receive_file();

private:
  int server_sockfd, port, communication_sockfd;
  struct sockaddr_in server_addr;
  const char *ip;
};
