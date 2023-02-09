#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#define PORT 8080

#define CREATE_ACCOUNT      1
#define LOGIN               2
#define LIST_USERS          3
#define SEND_MESSAGE        4
#define QUERY_MESSAGES      5
