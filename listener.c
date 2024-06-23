#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wait.h>

#define PORT "4090"
#define MAXBUFSIZE 100

void *getaddr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main() {
  int status;
  struct addrinfo hints, *servinfo, *curr;
  int sockfd, their_fd;
  int yes = 1;
  struct sockaddr_storage their_addr;
  socklen_t their_addr_len;
  char s[INET6_ADDRSTRLEN];
  char buf[MAXBUFSIZE];
  int num_bytes;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_DGRAM;
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

  printf("Listening on port: %s\n", PORT);
  
  their_addr_len = sizeof(their_addr);

  if ((num_bytes = recvfrom(sockfd, buf, MAXBUFSIZE - 1, 0,
                            (struct sockaddr *)&their_addr, &their_addr_len)) ==
      -1) {
    perror("recvfrom: ");
  }
  
  buf[num_bytes] = '\0';

  inet_ntop(their_addr.ss_family, getaddr((struct sockaddr *)&their_addr), s,
            sizeof(s));

  printf("received connection from: %s\n", s);
  printf("Message: %s\n", buf);

  close(sockfd);

  return 0;
}
