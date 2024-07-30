#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT "3490"
#define MAX_CONNECTIONS 10
#define MAX_BUFFER_SIZE 256

// Function prototypes
int create_listener_socket();
void *get_addr_name(struct sockaddr *address);
int add_to_poll_fds(struct pollfd **pfds, int new_fd, int *fd_count, int *fd_size);
void remove_from_pollfds(struct pollfd **pfds, int index, int *fd_count);

#endif // SERVER_UTILS_H