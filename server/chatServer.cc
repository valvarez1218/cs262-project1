#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#define PORT 8080

#define CREATE_ACCOUNT 1
#define LOGIN 2
#define LIST_USERS 3
#define SEND_MESSAGE 4
#define QUERY_NOTIFICATIONS 5
#define QUERY_MESSAGES 6
#define DELETE_ACCOUNT 7
#define CREATE_ACCOUNT_REPLY 8
#define LOGIN_REPLY 9
#define LIST_USERS_REPLY 10
#define SEND_MESSAGE_REPLY 11
#define QUERY_NOTIFICATIONS_REPLY 12
#define QUERY_MESSAGES_REPLY 13
#define DELETE_ACCOUNT_REPLY 14

int main (void) {

}