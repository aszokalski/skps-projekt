#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <random>
#include <string>
#include "message_queue.h"

const std::string SERVER_ADDRESS = "127.0.0.1";
const int SERVER_PORT = 1234;

const std::string CLIENT_ADDRESS = "127.0.0.1";
const int CLIENT_PORT = 1234;

//int main() {
//    srand(3141592);
//
//    if (atexit(close_queue)) {
//        std::cerr << "Close_queue: failed" << std::endl;
//        return EXIT_FAILURE;
//    }
//
//    open_queue();
//
//    int socket_file_descriptor = create_socket();
//
//    struct pollfd poll_descriptor = {.fd = queue_data, .events = POLLIN};
//    struct pollfd udp_descriptor = {.fd = socket_file_descriptor, .events = POLLOUT};
//
//    char buffer[BUFFER_SIZE];
//
//    struct sockaddr_in server_address;
//    setup_server_address(server_address);
//
//    struct sockaddr_in client_address;
//    setup_client_address(client_address);
//
//    bind_socket(socket_file_descriptor, server_address);
//
//    std::cout << "Hello, Connection!" << std::endl;
//
//    // Start UDP connection
//    receive(socket_file_descriptor, buffer, client_address);
//
//    std::cout << "Hello, Client!" << std::endl;
//
//    while (true) {
//        int ready = poll(&poll_descriptor, 1, -1);
//
//        if (ready < 0) {
//            std::cerr << "Poll: failed" << std::endl;
//            return EXIT_FAILURE;
//        }
//
//        if (poll_descriptor.revents & POLLIN) {
//            struct mq_attr attribute;
//            mq_getattr(queue_data, &attribute);
//            char buffer[attribute.mq_msgsize];
//            ssize_t received = mq_receive(queue_data, buffer, attribute.mq_msgsize, NULL);
//
//            if (received < 0) {
//                std::cerr << "Mq_receive: failed" << std::endl;
//                return EXIT_FAILURE;
//            }
//
//            int done = poll(&udp_descriptor, 1, 30000);
//            if (done < 0) {
//                std::cerr << "UDP Descriptor: failed" << std::endl;
//                return EXIT_FAILURE;
//            } else if (udp_descriptor.revents & POLLOUT) {
//                send(socket_file_descriptor, buffer, client_address);
//            } else if (udp_descriptor.revents & POLLHUP) {
//                close(socket_file_descriptor);
//                return 0;
//            }
//        }
//    }
//
//    return 0;
//}
struct ReceivedMessage {
    std::string message_text;
    std::string ip_addr;
    int port{};
};

class Server {
public:
    virtual void bind_socket(const std::string &address, int port) = 0;
    virtual void send_message(const std::string &message, const std::string &client_addr, int client_port) = 0;
    virtual ReceivedMessage receive_message() = 0;

    virtual ~Server() = default;
};

class UDPServer : public Server {
public:
    UDPServer() : sockfd(-1) {}
    ~UDPServer() override {
        if (sockfd >= 0) {
            close(sockfd);
            sockfd = -1;
        }
    }

    void bind_socket(const std::string &address, int port) override {
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            throw std::runtime_error("Error creating socket");
        }

        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr(address.c_str());
        server_addr.sin_port = htons(port);

        if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            throw std::runtime_error("Error while binding");
        }
    }


    void send_message(const std::string &message, const std::string &client_addr, int client_port) override {
        struct sockaddr_in cli_addr;
        memset(&cli_addr, 0, sizeof(cli_addr));
        cli_addr.sin_family = AF_INET;
        cli_addr.sin_addr.s_addr = inet_addr(client_addr.c_str());
        cli_addr.sin_port = htons(client_port);

        if (sendto(sockfd, message.c_str(), message.size(), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr)) < 0) {
            throw std::runtime_error("Error while sending");
        }
    }

    ReceivedMessage receive_message() override {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        std::vector<char> buffer(1024);

        ssize_t bytes_read = recvfrom(
                sockfd, buffer.data(), buffer.size() - 1, 0, (struct sockaddr *)&client_addr, &client_len
                        );

        if (bytes_read == 0) {std::cout << "Client disconnected" << std::endl; }
        else if (bytes_read < 0) {
            throw std::runtime_error("Error while reading");
        }

        buffer[bytes_read] = '\0';

        ReceivedMessage message;
        message.message_text = std::string(buffer.data());
        message.ip_addr = inet_ntoa(client_addr.sin_addr);
        message.port = ntohs(client_addr.sin_port);

        return message;
    }

private:
    int sockfd;
};


int main() {
    UDPServer server;
    server.bind_socket(SERVER_ADDRESS, SERVER_PORT);

    MessageQueue queue;

    while(true) {

        std::string data = queue.receive_data();

        ReceivedMessage message = server.receive_message();
//        std::cout << message.message_text << std::endl;
//        std::string response = "Your ip is: " + message.ip_addr;
        server.send_message(data, CLIENT_ADDRESS, CLIENT_PORT);
    }
}