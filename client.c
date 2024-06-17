#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT "8080"

int main() {
  int status;
  struct addrinfo hints, *servinfo, *curr;
  int sockfd;
  int yes = 1;
  struct sockaddr their_addr;
  socklen_t their_addr_len;

  if ((status = getaddrinfo("localhost", PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
  }

  for (curr = servinfo; curr != NULL; curr = curr->ai_next) {
    if ((sockfd = socket(curr->ai_family, curr->ai_socktype,
                         curr->ai_protocol)) == -1) {
      perror("socket: ");
      continue;
    }

    if (connect(sockfd, curr->ai_addr, curr->ai_addrlen) == -1) {
      perror("connect: ");
      close(sockfd);
      continue;
    }

    break;
  }

  freeaddrinfo(servinfo);

  if (curr == NULL) {
    fprintf(stderr, "connect: socket failed to connect");
    exit(1);
  }

  char message[100];
  int message_size = 100;

  if (recv(sockfd, message, message_size, 0) == -1) {
    perror("recv: ");
  }

  printf("%s\n", message);

  close(sockfd);

  return 0;
}
