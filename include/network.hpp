#ifndef NETWORK_H
#define NETWORK_H
#include <sys/types.h> // For socket types
// Function prototypes for network communication
// namespace network {

int create_socket(int domain, int type, int protocol);

size_t send_data(int socket_fd, const void *data, size_t size);

size_t receive_data(int socket_fd, void *buffer, size_t size);

void close_socket(int socket_fd);
#ifdef _WIN32
#include <winsock2.h> // Windows
#define SOCKET int
#else

#include <arpa/inet.h>
#include <netinet/in.h>

#endif

#endif // NETWORK_H
//}
