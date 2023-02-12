#include "../messageTypes.h"
#include "userOperations.h"

#include <iomanip>
#include <cstring>
#include <unordered_map>

#define PORT 8080


// DECLARING GLOBALS
// a map from user commands to operation codes
const std::unordered_map<std::string, opCode> operationMap {
    {"create_account",              CREATE_ACCOUNT},
    {"login",                       LOGIN},
    {"logout",                      LOGOUT},
    {"list",                        LIST_USERS},
    {"send",                        SEND_MESSAGE},
    {"query_notifications",         QUERY_NOTIFICATIONS},
    {"query_messages",              QUERY_MESSAGES},
    {"delete",                      DELETE_ACCOUNT},
};

const size_t g_InputLimit = 1303;
bool g_ProgramRunning = true;
int server_socket = 0;


// FUNCTION DECLARATIONS

void parseInput (std::string userInput);
bool takeInput (char (&inputBuffer)[g_InputLimit]);
void printUsage();


int main (void) {

    // TODO: establish connection to server


    char userInput[g_InputLimit];

    while(g_ProgramRunning) {
        if (!takeInput(userInput)) {
            continue;
        }
        

        try {
            parseInput(userInput);
        } catch (std::invalid_argument &e) {
            std::cout << e.what() << std::endl;
            printUsage();
            continue;
        }
        
        // TODO: listen for new messages and responses

     }

    return 0;
}

// This function simply takes input from the user and checks that it does not go
//     over the character limit
bool takeInput (char (&inputBuffer)[g_InputLimit]) {
    std::cout << "chat262$ ";
    std::cin.getline(inputBuffer, g_InputLimit);

    if (std::cin.gcount() > g_InputLimit - 3) {
        std::cout << "The character limit is 1300 characters!" << std::endl;
        return false;
    }

    return true;
}


// This function will parse the user input and return the corresponding operation code
void parseInput (std::string userInput) {
    std::string delimiter = " ";
    int start = 0;
    int end = userInput.find(delimiter);

    std::string firstToken = userInput.substr(start, end);

    if (operationMap.find(firstToken) == operationMap.end()) {
        std::string errorMessage = "'";
        errorMessage += firstToken;
        errorMessage += "' is not a defined operation.";
        throw std::invalid_argument(errorMessage);
    }

    opCode operation = operationMap.at(firstToken);

    // Convert remainder of string into vector of strings
    std::vector<std::string> remainingInputVector;
    start = end + 1;
    end = userInput.find(delimiter, start);
    while (end != std::string::npos) {
        std::string substr = userInput.substr(start, end - start);
        remainingInputVector.push_back(substr);
        start = end + 1;
        end = userInput.find(delimiter, start);
    }
    std::string substr = userInput.substr(start, std::string::npos);
    remainingInputVector.push_back(substr);


    Message* message;
    switch (operation)
    {
        case CREATE_ACCOUNT:
            {
                CreateAccountMessage msg;
                message = &msg;
                try {
                    message->populate(remainingInputVector);
                    createAccount(server_socket, msg);
                } catch (std::runtime_error &e) {
                    std::cout << e.what() << "CreateAccountMessage" << std::endl;
                } catch (std::invalid_argument &e) {
                    std::cout << e.what() << std::endl;
                }
            }
            break;

        case LOGIN:
            {
                LoginMessage msg;
                message = &msg;
                try {
                    message->populate(remainingInputVector);
                    login(server_socket, msg);
                } catch (std::runtime_error &e) {
                    std::cout << e.what() << "LoginMessage" << std::endl;
                } catch (std::invalid_argument &e) {
                    std::cout << e.what() << std::endl;
                }
            }
            break;

        case LOGOUT:
            {
                LogoutMessage msg;
                message = &msg;
                try {
                    message->populate(remainingInputVector);
                    logout(server_socket, msg);
                } catch (std::runtime_error &e) {
                    std::cout << e.what() << "LogoutMessage" << std::endl;
                } catch (std::invalid_argument &e) {
                    std::cout << e.what() << std::endl;
                }
            }
            break;
        
        case LIST_USERS:
            {
                ListUsersMessage msg;
                message = &msg;
                try {
                    message->populate(remainingInputVector);
                    listUsers(server_socket, msg);
                } catch (std::runtime_error &e) {
                    std::cout << e.what() << "ListUsersMessage" << std::endl;
                } catch (std::invalid_argument &e) {
                    std::cout << e.what() << std::endl;
                }
            }
            break;
        
        case SEND_MESSAGE:
            {
                SendMessageMessage msg;
                message = &msg;
                try {
                    message->populate(remainingInputVector);
                    sendMessage(server_socket, msg);
                } catch (std::runtime_error &e) {
                    std::cout << e.what() << "SendMessageMessage" << std::endl;
                } catch (std::invalid_argument &e) {
                    std::cout << e.what() << std::endl;
                }
            }
            break;
        
        case QUERY_NOTIFICATIONS:
            {
                QueryNotificationsMessage msg;
                message = &msg;
                try {
                    message->populate(remainingInputVector);
                    queryNotification(server_socket, msg);
                } catch (std::runtime_error &e) {
                    std::cout << e.what() << "QueryNotificationsMessage" << std::endl;
                } catch (std::invalid_argument &e) {
                    std::cout << e.what() << std::endl;
                }
            }
            break;

        case QUERY_MESSAGES:
            {
                QueryMessagesMessage msg;
                message = &msg;
                try {
                    message->populate(remainingInputVector);
                    queryMessages(server_socket, msg);
                } catch (std::runtime_error &e) {
                    std::cout << e.what() << "QueryMessagesMessage" << std::endl;
                } catch (std::invalid_argument &e) {
                    std::cout << e.what() << std::endl;
                }
            }
            break;
        
        case DELETE_ACCOUNT:
            {
                DeleteAccountMessage msg;
                message = &msg;
                try {
                    message->populate(remainingInputVector);
                    deleteAccount(server_socket, msg);
                } catch (std::runtime_error &e) {
                    std::cout << e.what() << "DeleteAccountMessage" << std::endl;
                } catch (std::invalid_argument &e) {
                    std::cout << e.what() << std::endl;
                }
            }
            break;

        default:
            // TODO: should probably throw an exception here
            break;
    }

}


void printUsage() {
    // TODO: print usage for user
}