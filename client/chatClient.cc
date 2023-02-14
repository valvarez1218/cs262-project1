// #include "userOperations.h"
// #include "establishConnection.h"
#include "takeInput.h"

#include <iomanip>
#include <cstring>


// Boolean determining whether program is still running
bool g_ProgramRunning = true;

int main (void) {

    if (!establishConnection()) {
        return -1;
    }

    // TODO: hide functionality that is not available until login if user not logged in

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
