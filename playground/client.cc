#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#define PORT 8080

struct test_message {
    int num_messsage;
    char msg[1024*20];

    test_message(int v, char m[1024*20]) {
        num_messsage = v;
        strcpy(msg, m);
    }

};

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
    if (inet_pton(AF_INET, "10.250.202.200", &serv_addr.sin_addr)
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

    char msgs[1024*20];
    printf("%s\n", msgs);
    int curr_idx = 0;
    for (int i = 0; i < 13; i++) {
        char* m = "Test message\n";
        strcpy(&msgs[curr_idx], m);
        curr_idx += strlen(m);
    }

    test_message* messages = new test_message(13, msgs);
    printf("Size of message struct: %li\n", sizeof(*messages));
    send(sock, messages, sizeof(*messages), 0);
    
    // char* five = new char;
    // *five = 13;
    // send(sock, five, sizeof(char), 0);

    valread = read(sock, buffer, 1024);
    printf("Server: %s\n", buffer);
    // closing the connected socket
    close(client_fd);
    return 0;
}