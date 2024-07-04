
#include <asm-generic/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT "3490"

int listener() {
  int sockfd, status;
  struct addrinfo hints, *res, *curr;
  int yes = 1;

  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags = AI_PASSIVE;

  if ((status = getaddrinfo("localhost", PORT, &hints, &res)) != 0) {
    return -1;
  }

  for (curr = res; curr != NULL; curr = curr->ai_next) {
    if ((sockfd = socket(curr->ai_family, curr->ai_socktype,
                         curr->ai_protocol)) == -1) {
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
      return -1;
    }

    if (bind(sockfd, curr->ai_addr, curr->ai_addrlen) == -1) {
      close(sockfd);
      continue;
    }
    break;
  }

  if (curr == NULL) {
    return -1;
  }

  freeaddrinfo(res);

  return sockfd;
}

int main() {
  printf("%d\n", listener());
  return 0;
}
