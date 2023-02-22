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
#define FORCE_LOG_OUT               20

// Server->Client Replies 
#define CREATE_ACCOUNT_REPLY        11
#define LOGIN_REPLY                 12
#define LIST_USERS_REPLY            13
#define SEND_MESSAGE_REPLY          14
#define QUERY_NOTIFICATIONS_REPLY   15
#define QUERY_MESSAGES_REPLY        16
#define MESSAGES_SEEN_REPLY         18

#define REFRESH_REQUEST             21
#define HELP                        22
// #define NEW_MESSAGE_REPLY           19

// This is a value corresponding to the supported operations
typedef char opCode;


const size_t g_UsernameLimit = 32;
const size_t g_PasswordLimit = 32;
const size_t g_MessageLimit = 1000;
const size_t g_MessageQueryLimit = 20;

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


// Given a field and its character limit checks that the passed in value adheres to alphabet and 
//  character limit restrictions
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


// Message type sent by client when creating an account.
//      Contains the validated username and password the user
//      is requesting to make their account with
struct CreateAccountMessage {
    opCode operation;
    char userName[g_UsernameLimit];
    char password[g_PasswordLimit];
    
    // Default constructor, sets the operation code to CREATE_ACCOUNT
    CreateAccountMessage () {
        operation = CREATE_ACCOUNT;
    }

    // Given a socket file descriptor, the class fills in its own fields.
    //      This method is used by the server to read in the client request.
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

    // Given a vector of the user inputs, the class validates the input and populates
    //      its fields. This method is used by the client after the user input has been 
    //      parsed and turned into a vector of strings to create the message which is
    //      sent to the server.
    void populate (std::vector<std::string> inputField) {
        if (inputField.size() != 2) {
            throw std::invalid_argument("create_account takes 2 inputs: username password");
        }

        for (int idx = 0; idx < inputField.size(); idx++) {
            if (idx == 0) {
                std::string user = inputField[idx];
                // TODO: try validateUsername
                try {
                    validateField("Username", user, g_UsernameLimit);
                } catch (std::invalid_argument &e) {
                    throw e;
                }
                user += '\0';
                strcpy(userName, user.c_str());
            }

            if (idx == 1) {
                std::string pass_word = inputField[idx];
                try {
                    validateField("Password", pass_word, g_PasswordLimit);
                } catch (std::invalid_argument &e) {
                    throw e;
                }
                pass_word += '\0';
                strcpy(password, pass_word.c_str());
            }
        }
    }
};


// Message type sent by client to server to attempt to login to an account.
struct LoginMessage {
    opCode operation;
    char userName[g_UsernameLimit] = {0};
    char password[g_PasswordLimit] = {0};
    
    // Default constructor, sets operation code to LOGIN
    LoginMessage() {
        operation = LOGIN;
    }

    // Method used by the server. Given a socket file descriptor the class populates its
    //      own fields by reading from the socket. 
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

    // Method used by client. Given a vector representing the user input the class populates
    //      its own fields. 
    void populate(std::vector<std::string> inputField) {
        if (inputField.size() != 2) {
            throw std::invalid_argument("login takes 2 inputs: username password");
        }

        for (int idx = 0; idx < inputField.size(); idx++) {
            if (idx == 0) {
                std::string user = inputField[idx];
                try {
                    validateField("Username", user, g_UsernameLimit);
                } catch (std::invalid_argument &e) {
                    throw e;
                }
                user += '\0';
                strcpy(userName, user.c_str());
            }

            if (idx == 1) {
                std::string pass_word = inputField[idx];
                try {
                    validateField("Password", pass_word, g_PasswordLimit);
                } catch (std::invalid_argument &e) {
                    throw e;
                }
                pass_word += '\0';
                strcpy(password, pass_word.c_str());
            }
        }
    }
};


// Message sent from client to server to let it know that the client is
//      ending its session and the server can clean up its threads/socket
struct LogoutMessage {
    opCode operation;
    
    // Default constructor, sets operation code to LOGOUT
    LogoutMessage () {
        operation = LOGOUT;
    }

    // Method used by client. Given a vector representing the user input
    //      checks that there are no other inputs given
    void populate(std::vector<std::string> inputs) {
        if (inputs.size() != 0) {
            throw std::invalid_argument("logout takes 0 inputs.");
        }
    }
};


// Message sent from client to server to request a list of users that match an optionally
//      specified prefix.
struct ListUsersMessage {
    opCode operation;
    char prefix[g_UsernameLimit] = {0};

    // Default constructor, sets operation code to LIST_USERS
    ListUsersMessage() {
        operation = LIST_USERS;
    }

    // Method used by server. Given a socket file descriptor the class fills in its prefix
    //      field by reading from the socket
    bool parse (int socket_fd) {
        ssize_t valread = read(socket_fd, &prefix[0], g_UsernameLimit);
        return valread == -1 ? false : true;
    }

    // Method used by client. Given a vector representing user input the class validates the
    //      given username prefix and populates its prefix field.  
    void populate(std::vector<std::string> inputs) {
        if (inputs.size() > 1) {
            throw std::invalid_argument("list_users takes one optional input: username_prefix");
        }

        if (inputs.size() == 1) {
            std::string userPrefix = inputs[0];
            try {
                validateField("Username", userPrefix, g_UsernameLimit);
            } catch (std::invalid_argument &e) {
                throw e;
            }
            userPrefix += '\0';
            strcpy(prefix, inputs[0].c_str());
        }
    }
};


// Message sent from client to server to request that the message is sent to the specified
//      user. 
struct SendMessageMessage {
    opCode operation;
    char recipientUsername[g_UsernameLimit];
    char messageContent[g_MessageLimit];

    // Default constructor, sets operation code to SEND_MESSAGE
    SendMessageMessage() {
        operation = SEND_MESSAGE;
    }

    // Method used by server. Given a socket file descriptor the class fills in its fields
    //      by reading from the socket. 
    bool parse (int socket_fd) {
        ssize_t valread = read(socket_fd, &recipientUsername[0], g_UsernameLimit);
        if (valread == -1) {
            return false;
        }

        valread = read(socket_fd, &messageContent[0], g_MessageLimit);
        return valread == -1 ? false : true;
    }

    // Method used by client. Given a vector representing the user input the class validates
    //      the inputs and populates its field. 
    void populate (std::vector<std::string> inputs) {
        if (inputs.size() != 2) {
            throw std::invalid_argument("send_message takes 2 inputs: recipient_username message_content");
        }

        for (int idx = 0; idx < inputs.size(); idx++) {
            if (idx == 0) {
                std::string recipient = inputs[idx];
                try {
                    validateField("Username", recipient, g_UsernameLimit);
                } catch (std::invalid_argument &e) {
                    throw e;
                }
                recipient += '\0';
                strcpy(recipientUsername, recipient.c_str());
            }

            if (idx == 1) {
                std::string content = inputs[idx];
                if (content.size() > g_MessageLimit) {
                    std::string errorMessage = "The message_content field takes input of at most " + std::to_string(g_MessageLimit) +
                                                " characters.\n '" + content + "' is too long.";
                    throw std::invalid_argument(errorMessage);
                }
                content += '\0';
                strcpy(messageContent, content.c_str());
            }
        }
    }
};


// Message sent from client to server to request its notifications
struct QueryNotificationsMessage {
    opCode operation;

    // Default constructor, sets operatio code to QUERY_NOTIFICATIONS
    QueryNotificationsMessage() {
        operation = QUERY_NOTIFICATIONS;
    }

    // Method used by client. Given a vector representing user inputs class
    //      checks that no other input was given. 
    void populate(std::vector<std::string> inputs) {
        if (inputs.size() != 0) {
            throw std::invalid_argument("query_notifications takes 0 inputs.");
        }
    }
};


// Message sent from client to server to request messages with a specified users.
struct QueryMessagesMessage {
    opCode operation;
    char username[g_UsernameLimit];

    // Default constructor, sets oepration code to QUERY_MESSAGE
    QueryMessagesMessage () {
        operation = QUERY_MESSAGES;
    }

    // Method used by server. Given a socket file descriptor the class fills in
    //      its field by reading from the socket.
    bool parse (int socket_fd) {
        ssize_t valread = read(socket_fd, &username[0], g_UsernameLimit);
        return valread == -1 ? false : true;
    }

    // Message used by client. Given a vector representing the user inputs the field
    //      validates the inputs and populates its fields. 
    void populate(std::vector<std::string> inputs) {
        if (inputs.size() != 1) {
            throw std::invalid_argument("query_messages takes 1 input: username");
        }

        std::string user = inputs[0];
        try {
            validateField("Username", user, g_UsernameLimit);
        } catch (std::invalid_argument &e) {
            throw e;
        }
        user += '\0';
        strcpy(username, user.c_str());
    }
};


// Message sent from client to server to request that the user's account is deleted
struct DeleteAccountMessage {
    opCode operation;

    // Default constructor, sets operation code to DELETE_ACCOUNT
    DeleteAccountMessage() {
        operation = DELETE_ACCOUNT;
    }


    // Method called by client. Given a vector of inputs checks that no other input was given.
    void populate(std::vector<std::string> inputs) {
        if (inputs.size() != 0) {
            throw std::invalid_argument("delete_account takes no arguments");
        }
    }
};


// Message sent from client to server to indicate how many messages it read after
//      querying for messages
struct MessagesSeenMessage {
    opCode operation;
    char messagesSeen;
    char startingIndex;

    // Default constructor, sets operation code to MESSAGES_SEEN
    MessagesSeenMessage() {
        operation = MESSAGES_SEEN;
    }

    // Overloaded constructor, takes a value for number of messages seen and first
    //      message index
    MessagesSeenMessage(int numSeen, int startingIdx) {
        operation = MESSAGES_SEEN;
        messagesSeen = numSeen;
        startingIndex = startingIdx;
    }

    // Method used by server. Given a socket file descriptor class fills in its fields
    //      by reading from the socket. 
    bool parse (int socket_fd) {
        ssize_t valread = read(socket_fd, &messagesSeen, sizeof(char));
        if (valread == -1) {
            return false;
        }
        valread = read(socket_fd, &startingIndex, sizeof(char));
        return valread == -1 ? false : true;
    }
};


// Message sent from server to client. Tells the client that there is a new message from the 
//      specified user.
struct NewMessageMessage {
    opCode operation;
    char senderUsername[g_UsernameLimit];

    // Default constructor, initializes nothing
    NewMessageMessage(){}

    // Overloaded constructor, takes values for the username
    NewMessageMessage(char username[g_UsernameLimit]) {
        operation = NEW_MESSAGE;
        strcpy(senderUsername, username);
    }

    // Method used by client. Given a socket file descriptor the class fills in its field.
    void parse (int socket_fd) {
        ssize_t valread = read(socket_fd, &senderUsername[0], g_UsernameLimit);
        if (valread == -1) {
            throw std::runtime_error("Error reading new message from socket.");
        }
        std::cout << "New message from " << senderUsername << "!" << std::endl;
    }
};


// Message sent from server to client. After requesting to create account the server
//      lets tue client know if the account was created successfully or if there was an error.
struct CreateAccountReply {
    opCode operation;
    char queryStatus;


    // Default constructor, initializes nothing.
    CreateAccountReply() {
    }

    // Overloaded constructor, sets query status.
    CreateAccountReply(int c_queryStatus) {
        operation = CREATE_ACCOUNT_REPLY;
        queryStatus = c_queryStatus;
    }
};


// Message sent from server to client. After client reqeusts login
//      lets the client know whether login was successful
struct LoginReply {
    // 0 = success, 1 = failure (username or password incorrect)
    opCode operation;
    char queryStatus;

    // Default constructor, initializes nothing
    LoginReply() {
    }

    // Overloaded constructor, sets the query status
    LoginReply(int c_queryStatus) {
        operation = LOGIN_REPLY;
        queryStatus = c_queryStatus;
    }
};

// Wrapper for username strings. Allows for storing vectors of usernames and
//      faciliates sending over sockets.
struct Username {
    char username[g_UsernameLimit];

    // default constructor, initializes nothing.
    Username(){}


    // Overloaded constructor, sets username.    
    Username(std::string c_username) {
        strcpy(username, c_username.c_str());
    }
};


// Message sent from server to client. Returns the list of users found (or none) after
//       the client requests users matching a specified prefix.
struct ListUsersReply {
    opCode operation;
    char numberOfUsers;

    // Default Constructor, initializes nothing.
    ListUsersReply(){}

    // Overloaded constructor, sets number of users
    ListUsersReply(int c_numberOfUsers) {
        operation = LIST_USERS_REPLY;
        numberOfUsers = c_numberOfUsers;
    }

    // Method used by client. Given a socket file descriptor class fills in fields and reads
    //      all users returned.
    void readUsernames(int socket_fd) {
        int valread = read(socket_fd, &operation, sizeof(opCode));
        if (valread == -1) {
            throw std::runtime_error("Error reading operation code from socket.");
        }

        valread = read(socket_fd, &numberOfUsers, sizeof(char));
        if (valread == -1) {
            throw std::runtime_error("Error reading number of users from socket.");
        }

        for (int userCount = 0; userCount < numberOfUsers; userCount++) {
            char user[g_UsernameLimit];
            valread = recv(socket_fd, &user, g_UsernameLimit, MSG_WAITALL);
            if (valread == -1) {
                throw std::runtime_error("Error reading username from socket.");
            }
            std::cout << user << std::endl;
        }
    }
};


// Messages sent from server to client. After the client has requested to send a messge, tells 
//      the client whether or not the messages was sent successfully. 
struct SendMessageReply {
    opCode operation;
    // 0 = success, 1 = user doesn't exist
    char queryStatus;

    // Default constructor, initializes nothing.
    SendMessageReply(){
        operation = SEND_MESSAGE_REPLY;
    }

    // Overloaded constructor, sets query status.
    SendMessageReply(int c_queryStatus) {
        operation = SEND_MESSAGE_REPLY;
        queryStatus = c_queryStatus;
    }
};


// Messages sent from server to client. After client reqeusts notifications the server
//      reponds with the number of notifications to expect.
struct QueryNotificationReply {
    opCode operation;
    char numberOfUsers;

    // Default constructor, initializes nothing.
    QueryNotificationReply(){};

    // Overloaded constructor; sets number of users.
    QueryNotificationReply(int users) {
        numberOfUsers = users;
        operation = QUERY_NOTIFICATIONS_REPLY;
    }

    // Method used by client. Given a socket file descriptor the class fills in its fields
    //      and prints out notifications.
    void readNotifications(int socket_fd) {
        int valread = read(socket_fd, &operation, sizeof(opCode));
        if (valread == -1) {
            throw std::runtime_error("Error reading operation from socket.");
        }

        valread = read(socket_fd, &numberOfUsers, sizeof(char));
        if (valread == -1) {
            throw std::runtime_error("Error reading number of users from socket.");
        }
        std::cout << std::to_string(numberOfUsers) << " Notification(s)" << std::endl;

        for (int notificationsCounter = 0; notificationsCounter < numberOfUsers; notificationsCounter++) {
            std::pair<char[g_UsernameLimit], char> userNotesPair;
            valread = read(socket_fd, &userNotesPair, sizeof(userNotesPair));
            std::cout << "read " << std::to_string(valread) << std::endl;
            if (valread == -1) {
                throw std::runtime_error("Error reading notification from socket.");
            }
            std::cout << std::string(userNotesPair.first) << ": " << std::to_string(userNotesPair.second) << " new message(s)" << std::endl;
        }
        std::cout << "Those are all your notifications!" << std::endl;
    }
};

// Messages formatted for returning to client across the wire
struct ReturnMessage {
    char senderUsername[g_UsernameLimit];
    char messageContent[g_MessageLimit];
};

// Holds messages
struct QueryMessagesReply {
    opCode operation;
    char numberOfMessages;
    char firstMessageIndex;

    // Default constructor, initializes nothing
    QueryMessagesReply(){}

    // Overloaded constructor, sets number of messages and first message index.
    QueryMessagesReply(int c_numberOfMessages, int c_firstMessageIndex) {
        operation = QUERY_MESSAGES_REPLY;
        numberOfMessages = c_numberOfMessages;
        firstMessageIndex = c_firstMessageIndex;
    }

    // Method used by client. Given a socket file descriptor the class fills in its fields
    //      and prints out the messages read
    int readMessages(int socket_fd) {
        int valread = read(socket_fd, &operation, sizeof(opCode));
        if (valread == -1) {
            throw std::runtime_error("Error reading operation from socket.");
        }

        valread = read(socket_fd, &numberOfMessages, sizeof(char));
        if (valread == -1) {
            throw std::runtime_error("Error reading number of messages from socket.");
        }

        valread = read(socket_fd, &firstMessageIndex, sizeof(char));
        if (valread == -1) {
            throw std::runtime_error("Error reading first message index from socket.");
        }

        for (int numMessagesRead = 0; numMessagesRead < numberOfMessages; numMessagesRead++) {
            ReturnMessage msg;
            valread = recv(socket_fd, &msg, sizeof(ReturnMessage), MSG_WAITALL);
            if (valread == -1) {
                std::cout << "Error reading message from socket." << std::endl;
                return numMessagesRead;
            }
            std::cout << msg.senderUsername << ":  " << msg.messageContent << std::endl;
        }

        return numberOfMessages;
    }
};


// Message sent from server to client to indicate that their session must end as
//      a login was detected elsewhere.
struct ForceLogOutReply {
    opCode operation;

    // Default constructor, sets operation code to FORCE_LOG_OUT    
    ForceLogOutReply() {
        operation = FORCE_LOG_OUT;
    }
};