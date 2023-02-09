#include <string>

// Client->Server Messages 
#define CREATE_ACCOUNT 1
#define LOGIN 2
#define LIST_USERS 3
#define SEND_MESSAGE 4
#define QUERY_NOTIFICATIONS 5
#define QUERY_MESSAGES 6
#define DELETE_ACCOUNT 7

// Server->Client Replies 
#define CREATE_ACCOUNT_REPLY 8
#define LOGIN_REPLY 9
#define LIST_USERS_REPLY 10
#define SEND_MESSAGE_REPLY 11
#define QUERY_NOTIFICATIONS_REPLY 12
#define QUERY_MESSAGES_REPLY 13
#define DELETE_ACCOUNT_REPLY 14

// This is a value corresponding to the supported operations
typedef int opCode;

struct Message {
    opCode operation;

    Message(opCode op) {
        operation = op;
    }

    virtual void parse (int socket_fd);
};


struct CreateAccountMessage : Message {
    std::string userName;
    std::string password;

    
    void parse (int socket_fd) {
        size_t* next_size = new size_t;
        valread = read(socket_fd, next_size, sizeof(size_t));
        
    }
};