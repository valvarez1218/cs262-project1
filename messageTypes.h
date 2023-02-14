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

const size_t g_ClientUsernameLimit = g_UsernameLimit - 1;
const size_t g_ClientPasswordLimit = g_PasswordLimit - 1;
const size_t g_ClientMessageLimit = g_MessageLimit - 1;

const std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";


std::string characterError(std::string field, std::string fieldValue, size_t limit) {
    return "The " + field + " field takes input of at most " + std::to_string(limit) +
            " characters.\n '" + fieldValue + "' is too long.";
}


// check that character follows allowed alphabet
bool validString(std::string inputString) {
    int found = inputString.find_first_not_of(alphabet);
    if (found != std::string::npos) {
        return false;
    }
    return true;
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
                std::string user = inputField[idx];
                if (user.size() > g_ClientUsernameLimit) {
                    std::string errorMessage = characterError("username", user, g_ClientUsernameLimit);
                    throw std::invalid_argument(errorMessage);
                }
                user += '\0';
                strcpy(userName, user.c_str());
            }

            if (idx == 1) {
                std::string pass_word = inputField[idx];
                if (pass_word.size() > g_ClientPasswordLimit) {
                    std::string errorMessage = characterError("password", pass_word, g_ClientPasswordLimit);
                    throw std::invalid_argument(errorMessage);
                }
                pass_word += '\0';
                strcpy(password, pass_word.c_str());
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
                std::string user = inputField[idx];
                if (user.size() > g_ClientUsernameLimit) {
                    std::string errorMessage = characterError("username", user, g_ClientUsernameLimit);
                    throw std::invalid_argument(errorMessage);
                }
                user += '\0';
                strcpy(userName, user.c_str());
            }

            if (idx == 1) {
                std::string pass_word = inputField[idx];
                if (pass_word.size() > g_ClientPasswordLimit) {
                    std::string errorMessage = characterError("password", pass_word, g_ClientPasswordLimit);
                    throw std::invalid_argument(errorMessage);
                }
                pass_word += '\0';
                strcpy(password, pass_word.c_str());
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
            std::string input = inputs[0];
            if (input.size() > g_ClientUsernameLimit) {
                std::string errorMessage = characterError("username", input, g_ClientUsernameLimit);
                throw std::invalid_argument(errorMessage);
            }
            input += '\0';
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
                std::string recipient = inputs[idx];
                if (recipient.size() > g_ClientUsernameLimit) {
                    std::string errorMessage = characterError("username", recipient, g_ClientUsernameLimit);
                    throw std::invalid_argument(errorMessage);
                }
                recipient += '\0';
                strcpy(recipientUsername, recipient.c_str());
            }

            if (idx == 1) {
                std::string content = inputs[idx];
                if (content.size() > g_ClientMessageLimit) {
                    std::string errorMessage = characterError("message_content", content, g_ClientMessageLimit);
                    throw std::invalid_argument(errorMessage);
                }
                content += '\0';
                strcpy(messageContent, content.c_str());
            }
        }
    }
};



struct QueryNotificationsMessage : Message {

    QueryNotificationsMessage() {
        operation = QUERY_NOTIFICATIONS;
    }

    void populate(std::vector<std::string> inputs) {
        if (inputs.size() != 0) {
            throw std::invalid_argument("query_notifications takes 0 inputs.");
        }
    }    
};



struct QueryMessagesMessage : Message {
    char username[g_UsernameLimit];

    QueryMessagesMessage () {
        operation = QUERY_MESSAGES;
    }

    bool parse (int socket_fd) {
        ssize_t valread = read(socket_fd, &username[0], g_UsernameLimit);
        return valread == -1 ? false : true;
    }

    void populate(std::vector<std::string> inputs) {
        if (inputs.size() != 1) {
            throw std::invalid_argument("query_messages takes 1 input: username");
        }

        std::string user = inputs[0];
        if (user.size() > g_ClientUsernameLimit) {
            std::string errorMessage = characterError("username", user, g_ClientUsernameLimit);
            throw std::invalid_argument(errorMessage);
        }
        user += '\0';
        strcpy(username, user.c_str());
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

    void populate(std::vector<std::string> inputs) {
        if (inputs.size() != 2) {
            throw std::invalid_argument("delete_account takes 2 inputs: username password");
        }

        for (int idx = 0; idx < inputs.size(); idx++) {
            if (idx == 0) {
                std::string user = inputs[idx];
                if (user.size() > g_ClientUsernameLimit) {
                    std::string errorMessage = characterError("username", user, g_ClientUsernameLimit);
                    throw std::invalid_argument(errorMessage);
                }
                user += '\0';
                strcpy(username, user.c_str());
            }

            if (idx == 1) {
                std::string pass_word = inputs[idx];
                if (pass_word.size() > g_ClientPasswordLimit) {
                    std::string errorMessage = characterError("password", pass_word, g_ClientPasswordLimit);
                    throw std::invalid_argument(errorMessage);
                }
                pass_word += '\0';
                strcpy(password, pass_word.c_str());
            }
        }
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