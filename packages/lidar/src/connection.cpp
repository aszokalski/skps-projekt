// Server side implementation of UDP client-server model
#include <arpa/inet.h>
#include <fcntl.h>
#include <mqueue.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <random>
#include <string>

#define PORT 8888
#define BUFFER_SIZE 256

// UDP
// https://www.geeksforgeeks.org/udp-server-client-implementation-c/

int create_socket() {
    // Create socket file descriptor
    int socket_file_descriptor = socket(AF_INET, SOCK_DGRAM, 0);

    if (socket_file_descriptor < 0) {
        std::cerr << "Create_Socket: creation failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    return socket_file_descriptor;
}

void setup_server_address(sockaddr_in &server_address) {
    memset(&server_address, 0, sizeof(server_address));

    // Fill server information
    server_address.sin_family = AF_INET;  // IPv4
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);
}

void setup_client_address(sockaddr_in &client_address) {
    memset(&client_address, 0, sizeof(client_address));
}

void bind_socket(int &socket_file_descriptor, struct sockaddr_in &server_address) {
    // Bind the socket with the server address
    int status = bind(socket_file_descriptor, (const struct sockaddr *)&server_address,
                      sizeof(server_address));
    if (status < 0) {
        std::cerr << "Bind: failed" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void send(int &socket_file_descriptor, const std::string &message,
          const sockaddr_in &client_address) {
    socklen_t length = sizeof(client_address);

    int sent = sendto(socket_file_descriptor, message.c_str(), message.size() + 1, MSG_CONFIRM,
                      (const sockaddr *)&client_address, length);

    if (sent < 0) {
        std::cerr << "Send: failed" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void receive(int &socket_file_descriptor, char *buffer, sockaddr_in &client_address) {
    socklen_t length = sizeof(client_address);
    int received = recvfrom(socket_file_descriptor, buffer, BUFFER_SIZE, MSG_WAITALL,
                            (sockaddr *)&client_address, &length);
    if (received < 0) {
        std::cerr << "Receive: failed" << std::endl;
        exit(EXIT_FAILURE);
    }
    buffer[received] = '\0';
}

// Message Queue
// https://man7.org/linux/man-pages/man2/mq_open.2.html
// https://w3.cs.jmu.edu/kirkpams/OpenCSF/Books/csf/html/MQueues.html

mqd_t volatile queue_data = 0;

void open_queue() {
    mq_unlink("/data");
    queue_data = mq_open("/data", O_CREAT | O_RDONLY | O_EXCL, 0700, NULL);
    if (queue_data < 0) {
        std::cerr << "Open_queue: Failed to open" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void close_queue() {
    if (queue_data > 0) mq_close(queue_data);
    mq_unlink("/data");
}

int main() {
    srand(3141592);

    if (atexit(close_queue)) {
        std::cerr << "Close_queue: failed" << std::endl;
        return EXIT_FAILURE;
    }

    open_queue();

    int socket_file_descriptor = create_socket();

    struct pollfd poll_descriptor = {.fd = queue_data, .events = POLLIN};
    struct pollfd udp_descriptor = {.fd = socket_file_descriptor, .events = POLLOUT};

    char buffer[BUFFER_SIZE];

    struct sockaddr_in server_address;
    setup_server_address(server_address);

    struct sockaddr_in client_address;
    setup_client_address(client_address);

    bind_socket(socket_file_descriptor, server_address);

    std::cout << "Hello, Connection!" << std::endl;

    // Start UDP connection
    receive(socket_file_descriptor, buffer, client_address);

    std::cout << "Hello, Client!" << std::endl;

    while (true) {
        int ready = poll(&poll_descriptor, 1, -1);

        if (ready < 0) {
            std::cerr << "Poll: failed" << std::endl;
            return EXIT_FAILURE;
        }

        if (poll_descriptor.revents & POLLIN) {
            struct mq_attr attribute;
            mq_getattr(queue_data, &attribute);
            char buffer[attribute.mq_msgsize];
            ssize_t received = mq_receive(queue_data, buffer, attribute.mq_msgsize, NULL);

            if (received < 0) {
                std::cerr << "Mq_receive: failed" << std::endl;
                return EXIT_FAILURE;
            }

            int done = poll(&udp_descriptor, 1, 30000);
            if (done < 0) {
                std::cerr << "UDP Descriptor: failed" << std::endl;
                return EXIT_FAILURE;
            } else if (udp_descriptor.revents & POLLOUT) {
                send(socket_file_descriptor, buffer, client_address);
            } else if (udp_descriptor.revents & POLLHUP) {
                close(socket_file_descriptor);
                return 0;
            }
        }
    }

    return 0;
}