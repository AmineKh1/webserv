#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <vector>
// using namespace std;
#define TRUE             1
#define FALSE            0

void print_fd_set(fd_set* set) {
    int max_fd = FD_SETSIZE; // Assuming FD_SETSIZE is the maximum file descriptor value

    printf("File descriptors in the set:\n");
    for (int fd = 0; fd < max_fd; ++fd) {
        if (FD_ISSET(fd, set)) {
            printf("%d ", fd);
        }
    }
    printf("\n");
}


std::vector<int> open_sockets(int size) {
    int rc, on = 1;
    std::vector<int>     listen_sds;

    for(int i=0; i < size; i++) {
        listen_sds.push_back(socket(AF_INET6, SOCK_STREAM, 0));

        if(listen_sds.at(i) < 0) {
            perror("socket() failed");
            exit(-1);
        }
        rc = setsockopt(listen_sds.at(i), SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on));
        if (rc < 0) {
            perror("setsockopt() failed");
            close(listen_sds.at(i));
            exit(-1);
        }
        rc = fcntl(listen_sds.at(i), F_SETFL, O_NONBLOCK);
        if (rc < 0) {
           perror("fcntl() failed");
           close(listen_sds.at(i));
           exit(-1);
        }
    }
    return listen_sds;
}

void bind_sockets(std::vector<int> ports, std::vector<int> sockets)
{
    int rc, on = 1;
    struct sockaddr_in6 addr;

    for(int i = 0; i < ports.size(); i++) {

        memset(&addr, 0, sizeof(addr));
        addr.sin6_family    = AF_INET6;
        memcpy(&addr.sin6_addr, &in6addr_any, sizeof(in6addr_any));
        addr.sin6_port  = htons(ports[i]);
        rc = bind(sockets[i], (struct sockaddr *)&addr, sizeof(addr));
        if (rc < 0) {
            std::cerr << "bind() failed" << rc << std::endl;
            close(sockets[i]);
            exit(-1);
        }
    }
    
}

void listen_sockets(std::vector<int> sockets, fd_set* master_set, int* max_sd) {
    int rc = 1;
    int  new_sd;
    FD_ZERO(master_set);
    for (int i = 0; i < sockets.size(); i++) {
        rc = listen(sockets[i], 10);
        if (rc < 0)
        {
           perror("listen() failed");
           close(sockets[i]);
           exit(-1);
        }
        
        *max_sd = sockets[i];
        FD_SET(sockets[i], master_set);
    }
    //print content of sockets
    // for(int i = 0; i < sockets.size(); i++) {
    //     std::cout << sockets[i] << std::endl;
    // }
    // print_fd_set(&master_set);
}


int main(int argc, char *argv[]) {
    int listen_sd, max_sd, new_sd, i, close_conn, len;
    int desc_ready, end_server = FALSE;
    int rc;
    char buffer[20000];
    fd_set  master_set, working_set;
    std::vector<int>    ports;
    std::vector<int>    sockets;
    ports.push_back(7000);
    ports.push_back(9000);

    sockets= open_sockets(ports.size());
    bind_sockets(ports, sockets);
    listen_sockets(sockets, &master_set, &max_sd);
    // print_fd_set(&master_set);
    while(end_server == FALSE) {
        memcpy(&working_set, &master_set, sizeof(master_set));
        std::cout <<"Waiting on select()..." << std::endl;
        rc = select(max_sd + 1, &working_set, NULL, NULL, NULL);
        if (rc < 0)
        {
           perror("  select() failed");
           break;
        }

        if (rc == 0)
        {
           std::cout <<"  select() timed out.  End program." <<  std::endl;
           break;
        }

        desc_ready = rc;
        for(i=3; i <= max_sd && desc_ready > 0; i++) {
            
            if(FD_ISSET(i, &working_set)) {
                desc_ready -= 1;
                if(std::find(sockets.begin(), sockets.end(), i) != sockets.end()) {
                    std::cout << " Listening socket is readable" << std::endl;

                        new_sd = accept(i, NULL, NULL);
                        if(new_sd < 0) {
                            if(errno != EWOULDBLOCK) {
                                perror("  accept() failed");
                                end_server = TRUE;
                            }
                            break;
                        }
                        else {
                            std::cout << "  New incoming connection - " << new_sd << std::endl;
                            FD_SET(new_sd, &master_set);
                            if(new_sd > max_sd) {
                                max_sd = new_sd;
                            }
                        }
                        

                }
                else {
                    std::cout << "Discriptor " << i << "is readale" << std::endl;
                    close_conn = FALSE;
                    while (TRUE) {
                        rc = recv(i, buffer, sizeof(buffer), 0);
                        if(rc < 0) {
                            if(errno != EWOULDBLOCK) {
                                perror("  recv() failed");
                                close_conn = TRUE;
                            }
                            break;
                        }
                        if (rc == 0) {
                            
                            std::cout << "  Connection closed" << std::endl;
                            close_conn = TRUE;
                            break;
                        }
                        len = rc;
                        std::cout << "  " << len << " bytes received" << std::endl;
                        //print content of buffer
                        for(int i = 0; i < len; i++) {
                            std::cout << buffer[i];
                        }
                        std::cout << std::endl;
                        rc = send(i, buffer, len, 0);
                        if (rc < 0) {
                            perror("  send() failed");
                            close_conn = TRUE;
                            break;
                        }
                    }
                    if (close_conn) {
                        close(i);
                        FD_CLR(i, &master_set);
                        if(i == max_sd) {
                            while(FD_ISSET(max_sd, &master_set) == FALSE) {
                                max_sd -= 1;
                            }
                        }
                    }

                }
            }
        }
    }
     for (i=0; i <= max_sd; ++i)
   {
      if (FD_ISSET(i, &master_set))
         close(i);
   }

}