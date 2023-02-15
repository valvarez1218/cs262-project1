#include <iostream>
#include <string>
#include <cstdio>
#include <vector>
#include <exception>

const size_t g_InputLimit = 100;



std::vector<std::string> extractWords (std::string inputString) {
    // Convert remainder of string into vector of strings
    std::vector<std::string> remainingInputVector;
    std::string delimiters = " \t";
    int start = inputString.find_first_not_of(delimiters);
    printf("%i\n", start);
    int end = inputString.find_first_of(delimiters, start);
    printf("%i\n", end);
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

// This function takes a string and returns a vector of all substrings separated by
//     by a space character
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



int main (void) {
    char msg[g_InputLimit];

    while (true) {
        takeInput(msg);
        try {
            std::vector<std::string> strings = makeStringVector(msg);
            for (std::string s : strings) {
                std::cout << s << std::endl;
            }
        } catch (std::invalid_argument &e) {
            std::cout << e.what() << std::endl;
        }
    }


    return 0;
}