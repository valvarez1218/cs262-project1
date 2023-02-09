#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#define PORT 8080


int main(int argc, char const* argv[])
{
    int sock = 0, valread, client_fd;
    sockaddr_in serv_addr;
    // char* msg = "Hello from client";
    char buffer[1024] = { 0 };
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
 
    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    // 172.20.10.2
    if (inet_pton(AF_INET, "10.250.14.21", &serv_addr.sin_addr)
        <= 0) {
        printf(
            "\nInvalid address/ Address not supported \n");
        return -1;
    }
 
    if ((client_fd
         = connect(sock, (sockaddr*)&serv_addr,
                   sizeof(serv_addr)))
        < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    float vals[4] = {3.14, 2.27, 69, 4.20};
    char* msg = "Hello there";
    send(sock, msg, strlen(msg), 0);
    // printf("Message sent\n");
    valread = read(sock, buffer, 1024);
    printf("Server: %s\n", buffer);
 
    // closing the connected socket
    close(client_fd);
    return 0;
}