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
#include <algorithm>


std::map<char[g_UsernameLimit], std::vector<std::pair<char[g_UsernameLimit], int>>> ConversationsDictionary;

struct UserPair {
    std::string smallerUsername;
    std::string largerUsername;

    // Initializes 
    UserPair (char username1[g_UsernameLimit], char username2[g_UsernameLimit]) {
        std::string username1String = username1;
        std::string username2String = username2;

        transform(username1String.begin(), username1String.end(), username1String.begin(), ::tolower);
        transform(username2String.begin(), username2String.end(), username2String.begin(), ::tolower);

        int compResult = username1String.compare(username2String);

        if (compResult >= 0) {
            smallerUsername = username2String;
            largerUsername = username1String;
        } else {
            smallerUsername = username1String;
            largerUsername = username2String;
        }


    }
};

struct StoredMessage {
    std::string senderUsername;
    bool isRead;
    std::string messageContent;

    StoredMessage (char username[g_UsernameLimit], bool isRead, char content[g_MessageLimit]) {
        senderUsername = username;
    }
};

struct MessagesDictionaryValue {
    std::vector<StoredMessage> messageList;
    std::mutex messageMutex;

    void addMessage(char username[g_UsernameLimit], bool read, char message[g_MessageLimit]) {
        messageMutex.lock();
        StoredMessage newMessage(username, read, message);

        messageList.push_back(newMessage);
        messageMutex.unlock();
    }

    void setRead(int startingIndex, int endingIndex, char readerUsername[g_UsernameLimit]) {
        messageMutex.lock();
        for (int i = startingIndex; i < endingIndex + 1; i++) {
            messageList[i].isRead = true;
        }
        messageMutex.unlock();

    }
};

std::map<UserPair, MessagesDictionaryValue> MessagesDictionary;
std::map<char[g_UsernameLimit], std::thread::id> ThreadDictionary;

// TODO: Users Trie
// TODO: mutex