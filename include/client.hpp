#ifndef CLIENT_H
#define CLIENT_H
#include "network.hpp"
#include "platform.hpp"
#include <netinet/in.h>
// Function prototypes for client-side action
extern int client_sockfd;
// Connect to server
bool connect_to_server(const char *host, int port);
// Overloaded to handle connection on same server but different port
bool connect_to_new_port(sockaddr_in *p_serv_addr);
// Send listed folder
void send_file_list(const char *folder_path, int socket_fd);
// Receive specified filename from the server
void handle_request(const char *filename, int socket_fd);
#endif
