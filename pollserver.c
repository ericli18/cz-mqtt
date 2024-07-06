#include <asm-generic/socket.h>
#include <netdb.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT "3490"
#define MAX_CONNECTIONS 10

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

  if (listen(sockfd, MAX_CONNECTIONS) == -1) {
    return -1;
  }

  return sockfd;
}

int main() {
  int listen_fd = listener();
  if(listen_fd == -1)
  {
    fprintf(stderr, "Error getting listening socket\n");
    exit(1);
  }

  int fd_count = 0;
  int fd_size = 5;

  struct pollfd *pfds = malloc(sizeof(struct pollfd) * fd_size);
  
  if(pfds == NULL)
  {
    perror("error assigning pfds: ");
    exit(1);
  }
  
  pfds[0].fd = listen_fd;
  pfds[0].events = POLLIN;
  
  fd_count = 1; //added the first file descriptor
  
  while(1)
  {
    int responses = poll(pfds, fd_count, -1);

    
  }
  



  return 0;
}
