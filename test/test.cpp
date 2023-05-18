// #include <sys/socket.h>
// #include <iostream>
// #include <netinet/in.h>
// #include <unistd.h>
// int main() {
//     const int PORT = 8080;
//     int server_fd;
//     int bind_n;
//     int new_socket;
//     struct sockaddr_in address;
//     socklen_t   address_len;
//     char buffer[1024] = {0};
//     int valread;
//     memset((char *)&address, 0, sizeof(address));
//     memset((char *)&address_len, 0, sizeof(address_len));
//     address.sin_family = AF_INET;
//     address.sin_addr.s_addr = htonl(INADDR_ANY);
//     address.sin_port = htons(PORT);
//     if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
//         std::cerr << "cannot create socket" << std::endl;
//         return 1;
//     }
//     if ((bind_n = bind(server_fd, (struct sockaddr *)&address, sizeof(address))) < 0)
//     {
//         std::cerr << "bind failed" << std::endl;
//         return 1;
//     }
//     if(listen(server_fd, 3) < 0)
//     {
//         std::cerr << "In listen" << std::endl;
//         exit(EXIT_FAILURE);
//     }
//     while (1) {
//         if ((new_socket = accept(server_fd, (struct sockaddr * )&address, (socklen_t*)&address_len)) < 0) {
//             std::cerr << "In accept" << std::endl;
//             exit(EXIT_FAILURE);
//         }
//         valread = read(new_socket, buffer, 1024);
//         std::cout << buffer << std::endl;
//         if ( valread < 0) {
//             std::cout << "No bytes are there to read" << std::endl;
//         }
//         char *hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
//         write (new_socket, hello, strlen(hello));
//         close(new_socket);
//     }
// }

//-------------------------------------------------
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include "request.hpp"
#include <sstream>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout) {
    int ret;
    if ((ret = select(nfds, readfds, writefds, exceptfds, timeout)) < 0) {
        std::cerr << "select error" << std::endl;
        exit(EXIT_FAILURE);
    }
    return ret;
}

std::vector<std::string>    split_words(const std::string &str) {
    std::vector<std::string> words;

    if (str.at(str.length() - 1) != '\r')
        exit(0);
    std::istringstream iss(str);
    while (iss)
    {
        std::string word;
        iss >> word;
        // std::cout << word << std::endl;
        words.push_back(word);
    }
    return words;
}
request_t split_lines(const std::string &str) {
    std::vector<std::vector<std::string> >  vectors;
    std::istringstream iss(str);
    std::string line;
    request_t request;
    while (std::getline(iss, line, '\n'))
    {
        vectors.push_back(split_words(line));
    }
    for (int i = 0; i < vectors.size(); i++)
    {
        for(int j = 0; j < vectors.at(i).size(); j++) {
            switch (i) {
                case 0: {
                    switch (j) {
                    case 0:
                        request.method = vectors.at(i).at(j);
                    case 1:
                        request.path = vectors.at(i).at(j);
                    case 2:
                        request.http_version = vectors.at(i).at(j);
                    }
                case 1:
                    request.host =  vectors.at(i).at(1);
                }
            }
                
        }
        }

    return request;
}
void init_pool(int listenfd, pool_t *p) {
    int i;
    p->maxi = -1;
    for (i = 0; i < FD_SETSIZE; i++) {
        p->clientfd[i] = -1;
        p->receiving_event[i] = 0;
    }
    p->maxfd = listenfd;
    FD_ZERO(&p->read_set);
    FD_SET(listenfd, &p->read_set);
}
// void check_client (pool *p) {

// }
void add_client(int confd, pool_t *p) {
    int i;
    p->nready--;
    for (i = 0; i < FD_SETSIZE; i++) {
        if (p->clientfd[i] < 0) {
            p->clientfd[i] = confd;
            p->receiving_event[i] = 0;
            FD_SET(confd, &p->read_set);
            if (confd > p->maxfd)
                p->maxfd = confd;
            if (i > p->maxi)
                p->maxi = i;
            break;
        }
    }
    if (i == FD_SETSIZE)
        std::cerr << "too many clients" << std::endl;
}
void err_sys(const char* x) 
{ 
    perror(x); 
    exit(1); 
}
void check_client(pool_t *pool) {
    int i, j, connfd, n, offset;
    char *end_of_uri;
    char *uri_ptr;
    char buf[4096];
    recv_buffer_t *rb;
    for (i = 0; (i <=pool->maxi) && (pool->nready > 0); i++) {
        connfd = pool->clientfd[i];
        rb = &pool->protos[i];
        offset = pool->protos[i].first_empty_byte;
        if ((connfd > 0) && (FD_ISSET(connfd, &pool->ready_set))) {
            pool->nready--;
            fprintf(stderr, "connfd %d is ready\n", connfd);
            n = read(connfd, buf, sizeof(buf));
            fprintf(stderr, "read %d bytes\n", n);

            if (n == 0) {
                close(connfd);
            }
            
        }
    }

}
int main() {
    const int PORT = 8080;
    int server_fd;
    int new_socket;
    struct sockaddr_in address;
    socklen_t address_len;
    request_t request; 
    int valread;
    char buffer[1024] = {0};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(PORT);
    static pool_t pool;
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "cannot create socket" << std::endl;
        return 1;
    }
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "bind failed" << std::endl;
        return 1;
    }
    if(listen(server_fd, 3) < 0) {
        std::cerr << "In listen" << std::endl;
        exit(EXIT_FAILURE);
    } 
    std::string str;
    int connfd;
    init_pool(server_fd, &pool);
    while(1) {
        pool.ready_set = pool.read_set;
                // fprintf(stderr, "read bytes\n");

        pool.nready = Select(pool.maxfd + 1, &pool.ready_set, NULL, NULL, NULL);
        fprintf(stderr, "read bytes\n");
        if (FD_ISSET(server_fd, &pool.ready_set)) {

            connfd = accept(server_fd, (struct sockaddr * )&address, (socklen_t*)&address_len);
            add_client(connfd, &pool);
        }
        
        check_client(&pool);
        // if ((new_socket = accept(server_fd, (struct sockaddr * )&address, (socklen_t*)&address_len)) < 0) {
        //     std::cerr << "In accept" << std::endl;
        //     exit(EXIT_FAILURE);
        // }
        // read from socket into buffer with chunks of 1024 bytes
        // valread = read(new_socket, buffer, 1024);
        // str.assign(buffer);
        // while (str.find("\r\n\r\n") == std::string::npos) {
        //     valread = read(new_socket, buffer, 10);
        //     str.append(buffer);
        // }

        
        
        // inet_ntop(AF_INET, &(address.sin_addr), buffer, INET_ADDRSTRLEN);
        // std::cout << "Client IP: " << buffer << std::endl;
        // str.assign(buffer);
        // std::cout << str << std::endl; 
        // request = split_lines(str);
        // std::cout << request.method << " " << request.path << " " << request.http_version << " " << request.host << std::endl;
        // if ( valread < 0) {
        //     std::cout << "No bytes are there to read" << std::endl;
        // }
        // close(connfd);
    }
}