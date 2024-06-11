//
// Created by mbaj on 11/06/24.
//

#pragma once

#include <fcntl.h>
#include <mqueue.h>
#include <netinet/in.h>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>

#define QUEUE_NAME "/data"
#define MAX_MSG_SIZE 256

class MessageQueue{
public:
    MessageQueue(){
        queue_data = mq_open(QUEUE_NAME, O_WRONLY);
        if (queue_data < 0) {
            throw std::runtime_error(
                    (std::ostringstream()
                            << "MessageQueue data init failed"
                            << "\n"
                    ).str()
            );
        }
    }

    ~MessageQueue(){
        if (queue_data > 0) {
            mq_close(queue_data);
        }
    }

    void send_data(uint16_t angle, uint16_t distance) const {
        std::cout<< "Sending data" << std::endl;
        std::cout << "angle = " << angle << std::endl;
        std::cout << "distance = " << distance << std::endl;
        std::cout << std::endl;

        std::string data = std::to_string(angle) + "," + std::to_string(distance);
        int sent = mq_send(queue_data, data.c_str(), data.size() + 1, 1);
        if (sent < 0) {
            throw std::runtime_error(
                    (std::ostringstream()
                            << "Queue data failed to send"
                            << "\n"
                    ).str()
            );
        }
    }

    [[nodiscard]] std::string receive_data() const {
        std::vector<char> buffer(MAX_MSG_SIZE);

        uint priority;
        ssize_t bytes_read = mq_receive(queue_data, buffer.data(), buffer.size(), &priority);

        if (bytes_read < 0) {
            throw std::runtime_error(
                    (std::ostringstream()
                            << "Queue data failed to receive"
                            << "\n").str()
            );
        }

        buffer.resize(bytes_read);
        return buffer.data();
    }

private:
    mqd_t volatile queue_data = 0;
};
