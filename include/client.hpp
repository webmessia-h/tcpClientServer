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
// Get and set new port
bool get_new_port(int socket_fd);
// Send listed folder
void send_file_list(const char *folder_path, int socket_fd);
// Receive specified filename from the server
void handle_request(const char *filename, int socket_fd);
// Send file with dynamic port change
void send_file(int socket_fd, const file_info &FILE);
#endif
