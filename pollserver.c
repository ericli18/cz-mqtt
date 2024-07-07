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

int create_listener_socket() {
    int listener_socket, getaddrinfo_status;
    struct addrinfo hints, *server_info, *current_addr;
    int reuse_addr_option = 1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;

    if ((getaddrinfo_status = getaddrinfo("localhost", PORT, &hints, &server_info)) != 0) {
        return -1;
    }

    for (current_addr = server_info; current_addr != NULL; current_addr = current_addr->ai_next) {
        if ((listener_socket = socket(current_addr->ai_family, current_addr->ai_socktype,
                                      current_addr->ai_protocol)) == -1) {
            continue;
        }

        if (setsockopt(listener_socket, SOL_SOCKET, SO_REUSEADDR, &reuse_addr_option, sizeof(reuse_addr_option)) == -1) {
            return -1;
        }

        if (bind(listener_socket, current_addr->ai_addr, current_addr->ai_addrlen) == -1) {
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

int main() {
    int listener_socket = create_listener_socket();
    if (listener_socket == -1) {
        fprintf(stderr, "Error creating listening socket\n");
        exit(1);
    }

    struct sockaddr_storage client_addr;
    socklen_t client_addr_len;
    int active_fd_count = 0;
    int poll_fd_capacity = 5;
    struct pollfd *poll_fds = malloc(sizeof(struct pollfd) * poll_fd_capacity);

    if (poll_fds == NULL) {
        perror("Error allocating poll_fds: ");
        exit(1);
    }

    poll_fds[0].fd = listener_socket;
    poll_fds[0].events = POLLIN;
    active_fd_count = 1; // Listener socket added as first file descriptor

    while (1) {
        int num_events = poll(poll_fds, active_fd_count, -1);
        if (num_events == -1) {
            perror("poll error: ");
            exit(1);
        }

        // Check listener socket
        if (poll_fds[0].revents & POLLIN) {
            client_addr_len = sizeof(client_addr);
            int new_client_socket;
            new_client_socket = accept(listener_socket, (struct sockaddr *)&client_addr, &client_addr_len);
            if (new_client_socket == -1) {
                perror("Error accepting new connection: ");
            }
            // TODO: Add new client socket to poll_fds array
        }

        // TODO: Handle events on other sockets
    }

    return 0;
}
