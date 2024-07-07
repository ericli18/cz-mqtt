#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wait.h>

#define PORT "31415"

void *getaddr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int get_socket(void) {
  int listener_fd;
  int yes = 1;
  int status;

  struct addrinfo hints, *info, *curr;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((status = getaddrinfo(NULL, PORT, &hints, &info)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }

  // no error checking here

  for (curr = info; curr != NULL; curr = curr->ai_next) {
    if ((listener_fd = socket(curr->ai_family, curr->ai_socktype,
                              curr->ai_protocol)) == -1) {
      continue;
    }

    setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    if (bind(listener_fd, curr->ai_addr, curr->ai_addrlen) == -1) {
      close(listener_fd);
      continue;
    }

    break;
  }

  freeaddrinfo(info);

  // something would have failed above, so we looped through everything;

  if (curr == NULL) {
    return -1;
  }

  if (listen(listener_fd, 1) == -1) {
    return -1;
  }

  return listener_fd;
}

void reap_chld(int s) {
  int saved_errno = errno;
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;
  errno = saved_errno;
}

int main() {
  int status;
  struct addrinfo hints, *servinfo, *curr;
  int sockfd, their_fd;
  int yes = 1;
  struct sockaddr_storage their_addr;
  struct sigaction sa;
  socklen_t their_addr_len;
  char s[INET6_ADDRSTRLEN];

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

  printf("Listening on port: %s\n", PORT);

  sa.sa_handler = reap_chld;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  while (1) {

    their_addr_len = sizeof(their_addr);
    if ((their_fd = accept(sockfd, (struct sockaddr *)&their_addr,
                           &their_addr_len)) == -1) {
      perror("accept: ");
      continue;
    }

    inet_ntop(their_addr.ss_family, getaddr((struct sockaddr *)&their_addr), s,
              sizeof(s));

    printf("received connection from: %s\n", s);

    if (!fork()) {
      // Child process
      close(sockfd);
      if (send(their_fd, "Hello!", 6, 0) == -1) {
        perror("send: ");
      }
      close(their_fd);
      exit(0);
    }

    close(their_fd);
  }

  close(sockfd);

  return 0;
}
