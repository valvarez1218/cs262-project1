#include "../messageTypes.h"

#include <netinet/in.h>
#include <stdlib.h>
#include <netdb.h>

#define PORT 8080

int main (int argc, char const* argv[]) {
    int server_fd, new_socket, valread;
    sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = { 0 };
    char* hello = "Hello from server";

 
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
 
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // For getting host IP address we followed tutorial found here: 
    //      https://www.tutorialspoint.com/how-to-get-the-ip-address-of-local-computer-using-c-cplusplus
    char host[256];
    char *IP;
    hostent *host_entry;
    int hostname;
    hostname = gethostname(host, sizeof(host)); //find the host name
    host_entry = gethostbyname(host); //find host information
    IP = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0])); //Convert into IP string
    // printf("Current Host Name: %s\n", host);
    printf("Host IP: %s\n", IP);

 
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (sockaddr*)&address,
             sizeof(address))
        < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket
         = accept(server_fd, (sockaddr*)&address,
                  (socklen_t*)&addrlen))
        < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    while (true) {
        SendMessageMessage msg;
        valread = read(new_socket, &msg, sizeof(SendMessageMessage));
        printf("%s\n", msg.messageContent);
        // printf("valread: %d\n", valread);
    }
    send(new_socket, hello, strlen(hello), 0);
    printf("Hello message sent\n");
 
    // closing the connected socket
    close(new_socket);
    // closing the listening socket
    shutdown(server_fd, SHUT_RDWR);
    return 0;
}