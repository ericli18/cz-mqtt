#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT "3490"
#define MAX_CONNECTIONS 10
#define MAX_BUFFER_SIZE 256

int create_listener_socket() {
  int listener_socket, getaddrinfo_status;
  struct addrinfo hints, *server_info, *current_addr;
  int reuse_addr_option = 1;

  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;
  hints.ai_flags = AI_PASSIVE;

  if ((getaddrinfo_status =
           getaddrinfo("localhost", PORT, &hints, &server_info)) != 0) {
    return -1;
  }

  for (current_addr = server_info; current_addr != NULL;
       current_addr = current_addr->ai_next) {
    if ((listener_socket =
             socket(current_addr->ai_family, current_addr->ai_socktype,
                    current_addr->ai_protocol)) == -1) {
      continue;
    }

    if (setsockopt(listener_socket, SOL_SOCKET, SO_REUSEADDR,
                   &reuse_addr_option, sizeof(reuse_addr_option)) == -1) {
      return -1;
    }

    if (bind(listener_socket, current_addr->ai_addr,
             current_addr->ai_addrlen) == -1) {
      close(listener_socket);
      continue;
    }

    break;
  }

  if (current_addr == NULL) {
    return -1;
  }

  freeaddrinfo(server_info);

  if (listen(listener_socket, MAX_CONNECTIONS) == -1) {
    return -1;
  }

  return listener_socket;
}

void *get_addr_name(struct sockaddr *address) {
  if (address->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)address)->sin_addr);
  } else {
    return &(((struct sockaddr_in6 *)address)->sin6_addr);
  }
}

int add_to_poll_fds(struct pollfd **pfds, int new_fd, int *fd_count,
                    int *fd_size) {
  if (*fd_count == *fd_size) {
    *fd_size *= 2;
    struct pollfd *temp = realloc(*pfds, sizeof(struct pollfd) * (*fd_size));
    if (temp == NULL) {
      return -1; // Reallocation failed
    }
    *pfds = temp;
  }

  (*pfds)[*fd_count].fd = new_fd;
  (*pfds)[*fd_count].events = POLLIN;
  (*fd_count)++;

  return 0; // Success
}

void remove_from_pollfds(struct pollfd **pfds, int index, int *fd_count) {
  pfds[index] = pfds[*fd_count - 1];
  (*fd_count)--;
}
