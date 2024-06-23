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

  if ((status = getaddrinfo("localhost", PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }

  for (curr = servinfo; curr != NULL; curr = curr->ai_next) {
    if ((sockfd = socket(curr->ai_family, curr->ai_socktype,
                         curr->ai_protocol)) == -1) {
      perror("socket: ");
      continue;
    }

    break;
  }


  if (curr == NULL) {
    fprintf(stderr, "bind: socket failed to bind");
    exit(1);
  }

  if(sendto(sockfd, "Hello world!", 12, 0, curr->ai_addr, curr->ai_addrlen) == -1)
  {
    perror("sendto: ");
  }
  freeaddrinfo(servinfo);
  
  close(sockfd);

  return 0;
}
