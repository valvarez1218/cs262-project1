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
#define FORCE_LOG_OUT               20

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

// check that character follows allowed alphabet
bool validString(std::string inputString) {
    // Check that all characters are from alphabet
    int found = inputString.find_first_not_of(alphabet);
    if (found != std::string::npos) {
        return false;
    }
    return true;
}

void validateField(std::string fieldName, std::string fieldValue, const size_t fieldLimit) {
    if (fieldValue.size() < 1) {
        std::string errorMsg = fieldName + " cannot be empty string.";
        throw std::invalid_argument(errorMsg);
    }
    if (!validString(fieldValue)) {
        std::string errorMsg = fieldName + " must be alphanumeric.";
        throw std::invalid_argument(errorMsg);
    }
    if (fieldValue.size() > fieldLimit) {
        std::string errorMsg = "The " + fieldName + " field takes input of at most " + std::to_string(fieldLimit) +
            " characters.\n '" + fieldValue + "' is too long.";
        throw std::invalid_argument(errorMsg);
    }
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
    char userName[g_UsernameLimit];
    char password[g_PasswordLimit];

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
                // TODO: try validateUsername
                try {
                    validateField("Username", user, g_ClientUsernameLimit);
                } catch (std::invalid_argument &e) {
                    throw e;
                }
                user += '\0';
                strcpy(userName, user.c_str());
            }

            if (idx == 1) {
                std::string pass_word = inputField[idx];
                try {
                    validateField("Password", pass_word, g_ClientPasswordLimit);
                } catch (std::invalid_argument &e) {
                    throw e;
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
                try {
                    validateField("Username", user, g_ClientUsernameLimit);
                } catch (std::invalid_argument &e) {
                    throw e;
                }
                user += '\0';
                strcpy(userName, user.c_str());
            }

            if (idx == 1) {
                std::string pass_word = inputField[idx];
                try {
                    validateField("Password", pass_word, g_ClientPasswordLimit);
                } catch (std::invalid_argument &e) {
                    throw e;
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
        ssize_t valread = read(socket_fd, &prefix[0], g_UsernameLimit);
        return valread == -1 ? false : true;
    }

    void populate(std::vector<std::string> inputs) {
        if (inputs.size() > 1) {
            throw std::invalid_argument("list_users takes one optional input: username_prefix");
        }

        if (inputs.size() == 1) {
            std::string userPrefix = inputs[0];
            try {
                validateField("Username", userPrefix, g_ClientUsernameLimit);
            } catch (std::invalid_argument &e) {
                throw e;
            }
            userPrefix += '\0';
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
                try {
                    validateField("Username", recipient, g_ClientUsernameLimit);
                } catch (std::invalid_argument &e) {
                    throw e;
                }
                recipient += '\0';
                strcpy(recipientUsername, recipient.c_str());
            }

            if (idx == 1) {
                std::string content = inputs[idx];
                if (content.size() > g_ClientMessageLimit) {
                    std::string errorMessage = "The message_content field takes input of at most " + std::to_string(g_ClientMessageLimit) +
                                                " characters.\n '" + content + "' is too long.";
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
        try {
            validateField("Username", user, g_ClientUsernameLimit);
        } catch (std::invalid_argument &e) {
            throw e;
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
                try {
                    validateField("Username", user, g_ClientUsernameLimit);
                } catch (std::invalid_argument &e) {
                    throw e;
                }
                user += '\0';
                strcpy(username, user.c_str());
            }

            if (idx == 1) {
                std::string pass_word = inputs[idx];
                try {
                    validateField("Password", pass_word, g_ClientPasswordLimit);
                } catch (std::invalid_argument &e) {
                    throw e;
                }
                pass_word += '\0';
                strcpy(password, pass_word.c_str());
            }
        }
    }
};


struct MessagesSeenMessage : Message {
    u_int messagesSeen;
    u_int startingIndex;
    char otherUsername[g_UsernameLimit];

    MessagesSeenMessage() {
        operation = MESSAGES_SEEN;
    }

    bool parse (int socket_fd) {
        ssize_t valread = read(socket_fd, &messagesSeen, sizeof(u_int));
        if (valread == -1) {
            return false;
        }
        valread = read(socket_fd, &startingIndex, sizeof(u_int));
        return valread == -1 ? false : true;

        valread = read(socket_fd, &otherUsername[0], g_UsernameLimit);
        if (valread == -1) {
            return false;
        }
    }
};



struct NewMessageMessage : Message {
    char senderUsername[g_UsernameLimit];
    char messageContent[g_MessageLimit];

    NewMessageMessage(char username[g_UsernameLimit], char message[g_MessageLimit]) {
        operation = NEW_MESSAGE;
        strcpy(senderUsername, username);
        strcpy(messageContent, message);
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
    // 0 = success, 1 = username taken
    int queryStatus;

    CreateAccountReply() {
    }

    CreateAccountReply(int c_queryStatus) {
        operation = CREATE_ACCOUNT_REPLY;
        queryStatus = c_queryStatus;
    }
};


// TODO
struct LoginReply : Reply {
    // 0 = success, 1 = failure (username or password incorrect)
    int queryStatus;

    LoginReply() {        
    }

    LoginReply(int c_queryStatus) {
        operation = LOGIN_REPLY;
        queryStatus = c_queryStatus;
    }
};

struct Username {
    char username[g_UsernameLimit];
    
    Username(std::string c_username) {
        strcpy(username, c_username.c_str());
    }
};


// TODO
struct ListUsersReply : Reply {
    int numberOfUsers;
    std::vector<Username> usernames;

    ListUsersReply(int c_numberOfUsers, std::vector<std::string> c_usernames) {
        operation = LIST_USERS_REPLY;
        numberOfUsers = c_numberOfUsers;

        for (int i = 0; i < c_usernames.size(); i++) {
            Username newUsername(c_usernames[i]);
            usernames.push_back(newUsername);
        }

    }
};


// TODO
struct SendMessageReply : Reply {
    // 0 = success, 1 = user doesn't exist
    int queryStatus;

    SendMessageReply(int c_queryStatus) {
        operation = SEND_MESSAGE_REPLY;
        queryStatus = c_queryStatus;
    }
};


// TODO
struct QueryNotificationReply : Reply {
    int numberOfUsers;
    std::vector<std::pair<char [g_UsernameLimit], char> > notifications;

    QueryNotificationReply(int users, std::vector<std::pair<char [g_UsernameLimit], char> > notificationsList) {
        numberOfUsers = users;

        for (int i=0; i < notificationsList.size(); i++) {
            std::pair<char [g_UsernameLimit], char> newVectorElement;
            strcpy(newVectorElement.first, notificationsList[0].first);
            newVectorElement.second = notificationsList[0].second;

            notifications.push_back(newVectorElement);
        }
        operation = QUERY_NOTIFICATIONS_REPLY;
    }

    // TODO; reads from socket and fills in fields
    void parse(int socket_fd) {

    }
};

// Messages formatted for returning to client across the wire
struct ReturnMessage {
    char senderUsername[g_UsernameLimit];
    char messageContent[g_MessageLimit];
};

// Holds messages
struct QueryMessagesReply : Reply {
    int numberOfMessages;
    int firstMessageIndex;
    std::vector<ReturnMessage> messageList;

    QueryMessagesReply(int c_numberOfMessages, int c_firstMessageIndex, std::vector<ReturnMessage> c_messageList) {
        operation = QUERY_MESSAGES_REPLY;
        numberOfMessages = c_numberOfMessages;
        firstMessageIndex = c_firstMessageIndex;
        messageList = c_messageList;
    }
};


// TODO
struct DeleteAccountReply : Reply {

    DeleteAccountReply() {
        operation = DELETE_ACCOUNT_REPLY;
    }
};

struct ForceLogOutReply : Reply {
    
    ForceLogOutReply() {
        operation = FORCE_LOG_OUT;
    }
};


// PROBABLY DON"T NEED
// struct MessagesSeenReply : Reply {

//     MessagesSeenReply() {
//         operation = MESSAGES_SEEN_REPLY;
//     }
// };


// PROBABLY DON"T NEED??
// struct NewMessageReply : Reply {

//     NewMessageReply() {
//         operation = NEW_MESSAGE_REPLY;
//     }
// };