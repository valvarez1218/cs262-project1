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


std::string characterError(std::string field, std::string fieldValue, size_t limit) {
    return "The " + field + " field takes input of at most " + std::to_string(limit) +
            " characters.\n '" + fieldValue + "' is too long.";
}

struct Message {
    opCode operation;

    virtual bool parse (int socket_fd) {
        throw NotImplementedException;
    };

    virtual void populate (std::vector<std::string> inputFields) {
        throw std::runtime_error("'populate' method not implemented for this class.");
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
        if (inputField.size() != 2) {
            throw std::invalid_argument("create_account takes 2 inputs: username password");
        }

        for (int idx = 0; idx < inputField.size(); idx++) {
            if (idx == 0) {
                std::string input = inputField[idx];
                if (input.size() > g_UsernameLimit) {
                    std::string errorMessage = characterError("username", input, g_UsernameLimit);
                    throw std::invalid_argument(errorMessage);
                }
                strcpy(userName, input.c_str());
            }

            if (idx == 1) {
                std::string input = inputField[idx];
                if (input.size() > g_PasswordLimit) {
                    std::string errorMessage = characterError("password", input, g_PasswordLimit);
                    throw std::invalid_argument(errorMessage);
                }
                strcpy(password, input.c_str());
            }
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

    void populate(std::vector<std::string> inputField) {
        if (inputField.size() != 2) {
            throw std::invalid_argument("login takes 2 inputs: username password");
        }

        for (int idx = 0; idx < inputField.size(); idx++) {
            if (idx == 0) {
                std::string input = inputField[idx];
                if (input.size() > g_UsernameLimit) {
                    std::string errorMessage = characterError("username", input, g_UsernameLimit);
                    throw std::invalid_argument(errorMessage);
                }
                strcpy(userName, input.c_str());
            }

            if (idx == 1) {
                std::string input = inputField[idx];
                if (input.size() > g_PasswordLimit) {
                    std::string errorMessage = characterError("password", input, g_PasswordLimit);
                    throw std::invalid_argument(errorMessage);
                }
                strcpy(password, input.c_str());
            }
        }
    }
};

struct LogoutMessage : Message  {
    
    LogoutMessage () {
        operation = LOGOUT;
    }

    void populate(std::vector<std::string> inputs) {
        if (inputs.size() != 0) {
            throw std::invalid_argument("logout takes 0 inputs.");
        }
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

    void populate(std::vector<std::string> inputs) {
        if (inputs.size() > 1) {
            throw std::invalid_argument("list_users takes one optional input: username_prefix");
        }

        if (inputs.size() == 1) {
            if (inputs[0].size() > g_UsernameLimit) {
                std::string errorMessage = characterError("username", inputs[0], g_UsernameLimit);
                throw std::invalid_argument(errorMessage);
            }
            strcpy(prefix, inputs[0].c_str());
        }
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

    void populate (std::vector<std::string> inputs) {
        if (inputs.size() != 2) {
            throw std::invalid_argument("send takes 2 inputs: recipient_username message_content");
        }

        for (int idx = 0; idx < inputs.size(); idx++) {
            if (idx == 0) {
                if (inputs[idx].size() > g_UsernameLimit) {
                    std::string errorMessage = characterError("username", inputs[idx], g_UsernameLimit);
                    throw std::invalid_argument(errorMessage);
                }
                strcpy(recipientUsername, inputs[idx].c_str());
            }

            if (idx == 1) {
                if (inputs[idx].size() > g_MessageLimit) {
                    std::string errorMessage = characterError("message_content", inputs[idx], g_MessageLimit);
                    throw std::invalid_argument(errorMessage);
                }
                strcpy(messageContent, inputs[idx].c_str());
            }
        }
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