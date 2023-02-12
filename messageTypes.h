#include <string>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <vector>

// Client->Server Messages 
#define CREATE_ACCOUNT              1
#define LOGIN                       2
#define LOGOUT                      3
#define LIST_USERS                  4
#define SEND_MESSAGE                5
#define QUERY_NOTIFICATIONS         6
#define QUERY_MESSAGES              7
#define DELETE_ACCOUNT              8
#define MESSAGES_SEEN               9

// Server->Client Messages
#define NEW_MESSAGE                 10

// Server->Client Replies 
#define CREATE_ACCOUNT_REPLY        11
#define LOGIN_REPLY                 12
#define LIST_USERS_REPLY            13
#define SEND_MESSAGE_REPLY          14
#define QUERY_NOTIFICATIONS_REPLY   15
#define QUERY_MESSAGES_REPLY        16
#define DELETE_ACCOUNT_REPLY        17 
#define MESSAGES_SEEN_REPLY         18
#define NEW_MESSAGE_REPLY           19

// This is a value corresponding to the supported operations
typedef char opCode;

const int NotImplementedException = 505;

const size_t g_UsernameLimit = 31;
const size_t g_PasswordLimit = 31;
const size_t g_MessageLimit = 1001;
const size_t g_MessageQueryLimit = 20;

const size_t g_ClientUsernameLimit = 30;
const size_t g_ClientPasswordLimit = 30;
const size_t g_ClientMessageLimit = 1000;


struct Message {
    opCode operation;

    virtual bool parse (int socket_fd) {
        throw NotImplementedException;
    };

    virtual void populate (std::vector<std::string> inputFields) {
        throw std::runtime_error("'populate' method not implemented for class ");
    };
};


struct Reply {
    opCode operation;
    int errno;
};

struct CreateAccountMessage : Message {
    char userName[g_UsernameLimit] = {0};
    char password[g_PasswordLimit] = {0};

    CreateAccountMessage () {
        operation = CREATE_ACCOUNT;
    }

    bool parse (int socket_fd) {
        ssize_t valread = read(socket_fd, &userName[0], g_UsernameLimit);
        if (valread == -1) {
            return false;   
        }
        
        valread = read(socket_fd, &password[0], g_PasswordLimit);
        if (valread == -1) {
            return false;
        }

        return true;
    }

    void populate (std::vector<std::string> inputField) {
        if (inputField.size() > 2) {
            throw std::invalid_argument("Number of inputs does not match number of fields to create account.");
        }

        for (std::string s : inputField) {
            
        }
    }
};


struct LoginMessage : Message {
    char userName[g_UsernameLimit] = {0};
    char password[g_PasswordLimit] = {0};
    
    LoginMessage() {
        operation = LOGIN;
    }

    bool parse (int socket_fd) {
        ssize_t valread = read(socket_fd, &userName[0], g_UsernameLimit);
        if (valread == -1) {
            return false;   
        }
        
        valread = read(socket_fd, &password[0], g_PasswordLimit);
        if (valread == -1) {
            return false;
        }

        return true;
    }
};

struct LogoutMessage : Message  {
    
    LogoutMessage () {
        operation = LOGOUT;
    }
};


struct ListUsersMessage : Message {
    char prefix[g_UsernameLimit] = {0};

    ListUsersMessage() {
        operation = LIST_USERS;
    }

    bool parse (int socket_fd) {
        ssize_t valread = read(socket_fd, &prefix[30], g_UsernameLimit);
        return valread == -1 ? false : true;
    }
};


struct SendMessageMessage : Message {
    char recipientUsername[g_UsernameLimit];
    char messageContent[g_MessageLimit];

    SendMessageMessage() {
        operation = SEND_MESSAGE;
    }

    bool parse (int socket_fd) {
        ssize_t valread = read(socket_fd, &recipientUsername[0], g_UsernameLimit);
        if (valread == -1) {
            return false;
        }

        valread = read(socket_fd, &messageContent[0], g_MessageLimit);
        return valread == -1 ? false : true;
    }
};



struct QueryNotificationsMessage : Message {

    QueryNotificationsMessage() {
        operation = QUERY_NOTIFICATIONS;
    }
};



struct QueryMessagesMessage : Message {
    char user[g_UsernameLimit];

    QueryMessagesMessage () {
        operation = QUERY_MESSAGES;
    }

    bool parse (int socket_fd) {
        ssize_t valread = read(socket_fd, &user[0], g_UsernameLimit);
        return valread == -1 ? false : true;
    }
};



struct DeleteAccountMessage : Message {
    char username[g_UsernameLimit];
    char password[g_PasswordLimit];

    DeleteAccountMessage() {
        operation = DELETE_ACCOUNT;
    }

    bool parse (int socket_fd) {
        ssize_t valread = read(socket_fd, &username[0], g_UsernameLimit);
        if (valread == -1) {
            return false;
        }
        valread = read(socket_fd, &password[0], g_PasswordLimit);
        return valread == -1 ? false : true;
    }
};


struct MessagesSeenMessage : Message {
    char otherUsername[g_UsernameLimit];
    u_int messagesSeen;
    u_int startingIndex;

    MessagesSeenMessage() {
        operation = MESSAGES_SEEN;
    }

    bool parse (int socket_fd) {
        ssize_t valread = read(socket_fd, &otherUsername[0], g_UsernameLimit);
        if (valread == -1) {
            return false;
        }
        valread = read(socket_fd, &messagesSeen, sizeof(u_int));
        if (valread == -1) {
            return false;
        }
        valread = read(socket_fd, &startingIndex, sizeof(u_int));
        return valread == -1 ? false : true;
    }
};



struct NewMessageMessage : Message {
    char senderUsername[g_UsernameLimit];
    char messageContent[g_MessageLimit];

    NewMessageMessage() {
        operation = NEW_MESSAGE;
    }

    bool parse (int socket_fd) {
        ssize_t valread = read(socket_fd, &senderUsername[0], g_UsernameLimit);
        if (valread == -1) {
            return false;
        }
        valread = read(socket_fd, &messageContent[0], g_MessageLimit);
        return valread == -1 ? false : true;
    }
};


// TODO
struct CreateAccountReply : Reply {

    CreateAccountReply() {
        operation = CREATE_ACCOUNT_REPLY;
    }
};


// TODO
struct LoginReply : Reply {

    LoginReply() {
        operation = LOGIN_REPLY;
    }
};


// TODO
struct ListUsersReply : Reply {

    ListUsersReply() {
        operation = LIST_USERS_REPLY;
    }
};


// TODO
struct SendMessageReply : Reply {

    SendMessageReply() {
        operation = SEND_MESSAGE_REPLY;
    }
};


// TODO
struct QueryNotificationRepy : Reply {

    QueryNotificationRepy() {
        operation = QUERY_NOTIFICATIONS_REPLY;
    }
};


// TODO
struct QueryMessagesReply : Reply {

    QueryMessagesReply() {
        operation = QUERY_MESSAGES_REPLY;
    }
};


// TODO
struct DeleteAccountReply : Reply {

    DeleteAccountReply() {
        operation = DELETE_ACCOUNT_REPLY;
    }
};


// TODO
struct MessagesSeenReply : Reply {

    MessagesSeenReply() {
        operation = MESSAGES_SEEN_REPLY;
    }
};


// TODO
struct NewMessageReply : Reply {

    NewMessageReply() {
        operation = NEW_MESSAGE_REPLY;
    }
};