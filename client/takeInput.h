// #include "../messageTypes.h"
#include "userOperations.h"
#include "establishConnection.h"

#include <unordered_map>

// DECLARING GLOBALS
// a map from user commands to operation codes
const std::unordered_map<std::string, opCode> operationMap {
    {"create_account",              CREATE_ACCOUNT},
    {"login",                       LOGIN},
    {"logout",                      LOGOUT},
    {"list",                        LIST_USERS},
    {"send_message",                SEND_MESSAGE},
    {"query_notifications",         QUERY_NOTIFICATIONS},
    {"query_messages",              QUERY_MESSAGES},
    {"delete_account",              DELETE_ACCOUNT},
};


// Limit to the number of characters the command line will read
const size_t g_InputLimit = 1303;

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

// This is a helper function for vectorizing the user input
std::vector<std::string> extractWords (std::string inputString) {
    // Convert remainder of string into vector of strings
    std::string delimiters = " \t";
    std::vector<std::string> remainingInputVector;
    int start = inputString.find_first_not_of(delimiters);
    int end = inputString.find_first_of(delimiters, start);
    while (end != std::string::npos && start != std::string::npos) {
        std::string substr = inputString.substr(start, end - start);
        remainingInputVector.push_back(substr);
        start = inputString.find_first_not_of(delimiters, end);
        end = inputString.find_first_of(delimiters, start);
    }

    if (start != std::string::npos) {
        std::string substr = inputString.substr(start, std::string::npos);
        if (substr.size() != 0) {
            remainingInputVector.push_back(substr);
        }
    }

    return remainingInputVector;
}

// This function takes a string and returns input as vector of strings as the message classes would expect it
std::vector<std::string> makeStringVector (std::string inputString) {
    int firstQuoteIdx = inputString.find_first_of("'\"");
    if (firstQuoteIdx == std::string::npos) {
        return extractWords(inputString);
    }


    std::string substr1 = inputString.substr(0, firstQuoteIdx);

    int lastQuoteIdx = inputString.find_last_of("'\"");
    if (lastQuoteIdx == firstQuoteIdx) {
        throw std::invalid_argument("Format Error: Closing quotation mark expected.");
    }
    std::string substr2 = inputString.substr(firstQuoteIdx+1, lastQuoteIdx-firstQuoteIdx-1);

    int start_last = inputString.find_first_not_of(" ", lastQuoteIdx+1);
    if (start_last != std::string::npos) {
        throw std::invalid_argument("Format Error: Should not have characters after the final quotation mark.");
    }

    // Convert remainder of string into vector of strings
    std::vector<std::string> remainingInputVector = extractWords(substr1);
    remainingInputVector.push_back(substr2);

    return remainingInputVector;
}



// This function will parse the user input and return the corresponding operation code
void parseInput (std::string userInput) {

    // Convert input string into vector of strings
    std::vector<std::string> inputVector;
    try {
        inputVector = makeStringVector(userInput);
    } catch(std::invalid_argument &e) {
        std::cout << e.what() << std::endl;
        return;
    }

    // check that first token is operation recognized by program
    std::string firstToken = inputVector[0];
    if (operationMap.find(firstToken) == operationMap.end()) {
        std::string errorMessage = "'";
        errorMessage += firstToken;
        errorMessage += "' is not a defined operation.";
        throw std::invalid_argument(errorMessage);
    }
    opCode operation = operationMap.at(firstToken);

    // remove operation from beginning of input vector
    std::vector<std::string> remainingInputVector(inputVector.begin()+1, inputVector.end());
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
                    std::cout << "In case LOGIN:" << std::endl;
                    std::cout << e.what() << std::endl;
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
                    std::cout << "In case LOGOUT:" << std::endl;
                    std::cout << e.what() << std::endl;
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
                    std::cout << "In case LIST_USERS:" << std::endl;
                    std::cout << e.what() << std::endl;
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
                    std::cout << "In case SEND_MESSAGE:" << std::endl;
                    std::cout << e.what() << std::endl;
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
                    std::cout << "In case QUERY_NOTIFICATIONS:" << std::endl;
                    std::cout << e.what() << std::endl;
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
                    std::cout << "In case QUERY_MESSAGES:" << std::endl;
                    std::cout << e.what() << std::endl;
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
                    std::cout << "In case DELETE_ACCOUNT:" << std::endl;
                    std::cout << e.what() << std::endl;
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
    std::cout << "Chat Usage:" << std::endl;
    std::cout << "      If Not Logged In:" << std::endl;
    std::cout << "          - create_account username password:         creates account and logs you in." << std::endl;
}