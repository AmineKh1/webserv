#ifndef REQUEST_HPP
#define REQUEST_HPP
#include<map>

#include <iostream>
#include <sys/socket.h>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

class Client {
private:
    std::string request;
    int socket_fd;

public:
    Client(int new_socket_fd) : socket_fd(new_socket_fd) {}

    void set_request(const std::string& new_request) {
        request = new_request;
    }

    std::string get_request() const {
        return request;
    }

    int get_socket_fd() const {
        return socket_fd;
    }

    void close() {
        ::close(socket_fd);
    }
};
#endif