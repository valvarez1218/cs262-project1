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
int client_fd;


// FUNCTION DECLARATIONS

void parseInput (std::string userInput);
bool takeInput (char (&inputBuffer)[g_InputLimit]);
void printUsage();


int main (void) {

    sockaddr_in serv_addr;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket Creation Error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    bool connected_to_server = false;

    while (!connected_to_server) {
        std::string ip_addr;
        std::cout << "Input IP Address of Server: ";
        std::cin >> ip_addr;
        if (inet_pton(AF_INET, ip_addr.c_str(), &serv_addr.sin_addr) <= 0) {
            printf("\nInvalid address/ Address not supported \n");
            continue;
        }

        if ((client_fd = connect(server_socket, (sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
            printf("\n Connection Failed \n");
            continue;
        }

        connected_to_server = true;
    }



    // Main loop for user
    char userInput[g_InputLimit];
    std::cin.ignore();
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
    if (substr.size() != 0) {
        remainingInputVector.push_back(substr);
    }


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
                    std::cout << "In Case CREATE_ACCOUNT:" << std::endl;
                    std::cout << e.what() << std::endl;
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
                    queryNotifications(server_socket, msg);
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