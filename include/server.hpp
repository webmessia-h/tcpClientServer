// server
#pragma once
#include "network.hpp"
#include "platform.hpp"
class Server {
public:
  Server(const char *ip, const int port);
  ~Server();
  // Receive and print listed folder
  bool receive_file_list();
  // Request desired file from the client
  bool request_upload(const char *filename);
  // Handle port change
  int change_port();
  //  Receive file
  bool receive_file(int communication_sockfd);

  int server_sockfd, port, communication_sockfd, transfer_sockfd;
  struct sockaddr_in server_addr;
  const char *ip;
};
