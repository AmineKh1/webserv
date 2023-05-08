#ifndef REQUEST_HPP
#define REQUEST_HPP
#include<map>

#include <iostream>
#include <sys/socket.h>
// #define FD_SETSIZE 1024

typedef struct recv_buffer_s {
    char requestbuf[4024];
    int first_empty_byte;
}   recv_buffer_t;
typedef struct request_s {
    std::string method;
    std::string path;
    std::string http_version;
    std::string host;
    std::map<std::string, std::string>  headers;
    std::string body;

}   request_t;
typedef struct pool_s {
    int maxfd;
    fd_set read_set;
    fd_set ready_set;
    int nready;
    int maxi;
    int clientfd[FD_SETSIZE];
    recv_buffer_t protos[FD_SETSIZE];
    int receiving_event[FD_SETSIZE];
}  pool_t;
#endif