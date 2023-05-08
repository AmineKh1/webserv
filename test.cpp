#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
int main() {
    const int PORT = 8080;
    int server_fd;
    int bind_n;
    int new_socket;
    struct sockaddr_in address;
    socklen_t   address_len;
    char buffer[1024] = {0};
    int valread;
    memset((char *)&address, 0, sizeof(address));
    memset((char *)&address_len, 0, sizeof(address_len));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(PORT);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "cannot create socket" << std::endl;
        return 1;
    }
    if ((bind_n = bind(server_fd, (struct sockaddr *)&address, sizeof(address))) < 0)
    {
        std::cerr << "bind failed" << std::endl;
        return 1;
    }
    if(listen(server_fd, 3) < 0)
    {
        std::cerr << "In listen" << std::endl;
        exit(EXIT_FAILURE);
    }
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr * )&address, (socklen_t*)&address_len)) < 0) {
            std::cerr << "In accept" << std::endl;
            exit(EXIT_FAILURE);
        }
        valread = read(new_socket, buffer, 1024);
        std::cout << buffer << std::endl;
        if ( valread < 0) {
            std::cout << "No bytes are there to read" << std::endl;
        }
        char *hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
        write (new_socket, hello, strlen(hello));
        close(new_socket);
    }
}