#ifndef SERVER_H
#define SERVER_H
#include "network.hpp"
#include <netinet/in.h>
extern int server_sockfd, connected_client_socket;
extern char buffer[4096];
// Function prototypes for server-side actions
// Listen for connections
bool listen_on_port(const char *host, int port, sockaddr_in *p_serv_addr);
// Bind server to different port but same address
bool listen_on_new_port(sockaddr_in *p_serv_addr);
// Accept connection
int accept_connection(int listening_socket_fd);
// Receive and print listed folder
bool receive_file_list(int socket_fd, char *buffer, size_t buffer_size);
// Request desired file from the client
bool request_upload(int socket, const char *filename, sockaddr_in *p_serv_addr,
                    int server_sockfd);
// Handle port change
bool change_port(sockaddr_in *p_serv_addr, int server_sockfd);
//  Receive file
bool receive_file(int client_socket);
#endif
