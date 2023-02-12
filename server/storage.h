#include <string>

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include "../messageTypes.h"
#include <map>
#include <tuple>
#include <thread>
#include <mutex>


// struct storedMessage {
//     std::vector<char> senderUsername;
//     bool isRead;
//     std::vector<char> messageContent;

//     storedMessage (char username[g_UsernameLimit], bool isRead, char content[g_MessageLimit]) {
//         senderUsername = username;
//     }
// }


std::map<char[g_UsernameLimit], std::vector<std::pair<char[g_UsernameLimit], int>>> ConversationsDictionary;

struct MessagesDictionaryValue {
    // std::vector<std::tuple<char[g_UsernameLimit],bool,char[g_MessageLimit]>> messageList;
    std::vector<std::tuple<char*,bool,char*>> messageList;
    std::mutex messageMutex;

    void addMessage(char username[g_UsernameLimit], char message[g_MessageLimit]) {
        messageMutex.lock();
        // auto newMessage = std::make_tuple(username, false, message);
        std::tuple<char*,bool,char*> newMessage {username, false, message};

        messageList.push_back(newMessage);
        messageMutex.unlock();
    }

    void setRead(int startingIndex, char[g_UsernameLimit] readerUsername) {
        messageMutex.lock();
        
        messageMutex.unlock();

    }
};

std::map<std::pair<char[g_UsernameLimit], char[g_UsernameLimit]>, MessagesDictionaryValue> MessagesDictionary;
std::map<char[g_UsernameLimit], std::thread::id> ThreadDictionary;

// TODO: Users Trie
// TODO: mutex