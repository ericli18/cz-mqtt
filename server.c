#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT "8080"

int main() {
  int status;
  struct addrinfo hints, *servinfo, *curr;
  int sockfd, their_fd;
  int yes = 1;
  struct sockaddr their_addr;
  socklen_t their_addr_len;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((status = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }

  for (curr = servinfo; curr != NULL; curr = curr->ai_next) {
    if ((sockfd = socket(curr->ai_family, curr->ai_socktype,
                         curr->ai_protocol)) == -1) {
      perror("socket: ");
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
      perror("critical setsockopt error: ");
      exit(1);
    }

    if (bind(sockfd, curr->ai_addr, curr->ai_addrlen) == -1) {
      perror("bind: ");
      close(sockfd);
      continue;
    }

    break;
  }

  freeaddrinfo(servinfo);

  if (curr == NULL) {
    fprintf(stderr, "bind: socket failed to bind");
    exit(1);
  }

  if (listen(sockfd, 1) == -1) {
    perror("listen: ");
    exit(1);
  }

  their_addr_len = sizeof(their_addr);

  if ((their_fd = accept(sockfd, &their_addr, &their_addr_len)) == -1) {
    perror("accept: ");
  }

  if (send(their_fd, "Hello!", 6, 0) == -1) {
    perror("send: ");
  }

  close(their_fd);
  close(sockfd);

  return 0;
}
