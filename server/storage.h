#include "../messageTypes.h"

#include <map>
#include <tuple>
#include <thread>
#include <mutex>
#include <algorithm>
#include <atomic>
#include <unordered_map>

struct CurrentConversation {
    char username[g_UsernameLimit];
    int messagesSentStartIndex;
    int messagesSentEndIndex;
};

// Key: user with active conversations, Value: map from users to number of notifications they have
struct ConversationsDictionary {
    std::unordered_map<std::string, std::unordered_map<std::string, int> > conversations;
    std::mutex notificationsMutex; // TODO: had to move lock up 1 level :(

    // increment new messages
    void newNotification(char senderUsername[g_UsernameLimit], char recipientUsername[g_UsernameLimit]) {
        notificationsMutex.lock();
        conversations[recipientUsername][senderUsername]++;
        notificationsMutex.unlock();
    }  

    // decrement seen messages
    void notificationSeen(char senderUsername[g_UsernameLimit], char recipientUsername[g_UsernameLimit]) {
        notificationsMutex.lock();
       conversations[recipientUsername][senderUsername]--;
        notificationsMutex.unlock();
    }

    // TODO: Package notifications to be sent out
    std::vector<std::pair<char [g_UsernameLimit], char> > getNotifications(char recipientUsername [g_UsernameLimit]) {
        std::vector<std::pair<char [g_UsernameLimit], char> > allNotifications;

        for (auto const& pair : conversations[recipientUsername]) {
            if (pair.second > 0) {
                std::pair<char [g_UsernameLimit], char> notificationItem;
                strcpy(notificationItem.first, pair.first.c_str());
                notificationItem.second = pair.second;

                allNotifications.push_back(notificationItem);
            }
        }

        return allNotifications;
    }


};

ConversationsDictionary conversationsDictionary;

// Messages dictionary key, consists of two usernames.
struct UserPair {
    std::string smallerUsername; // lexicographically smaller username
    std::string largerUsername; // lexicographically larger username

    // Initializes user pair using lexicographic ordering
    UserPair (char username1[g_UsernameLimit], char username2[g_UsernameLimit]) {
        std::string username1String = username1;
        std::string username2String = username2;

        int compResult = username1String.compare(username2String);

        if (compResult >= 0) {
            smallerUsername = std::string(username2);
            largerUsername = std::string(username1);
        } else {
            smallerUsername = std::string(username1);
            largerUsername = std::string(username2);
        }
    }

    friend bool operator== (const UserPair& pair1, const UserPair& pair2);
};

bool operator== (const UserPair& pair1, const UserPair& pair2) {
    return (pair1.smallerUsername == pair2.smallerUsername) && (pair1.largerUsername == pair2.largerUsername);
}

// A single messsage from a the message dictionary value vector
struct StoredMessage {
    std::string senderUsername;
    bool isRead;
    std::string messageContent;

    StoredMessage (char username[g_UsernameLimit], bool read, char content[g_MessageLimit]) {
        senderUsername = username;
        isRead = read;
        messageContent = content;
    }
};

// Wrapping for the return value of getStoredMessages in the StoredMessages struct
struct GetStoredMessagesReturnValue {
    int lastMessageIndex;
    int firstMessageIndex;
    std::vector<ReturnMessage> messageList;
};

// A list of stored messages
struct StoredMessages {
    std::vector<StoredMessage> messageList;
    std::mutex messageMutex;

    // Adding a new message onto the messageList
    // TODO: have this return thread of the recipient?
    void addMessage(char senderUsername[g_UsernameLimit], char recipientUsername[g_UsernameLimit], char message[g_MessageLimit]) {
        messageMutex.lock();

        StoredMessage newMessage(senderUsername, false, message);
        messageList.push_back(newMessage);

        // Increment unread messages for recipient 
        conversationsDictionary.newNotification(senderUsername, recipientUsername);
        messageMutex.unlock();
    }

    // Setting a subset of messages as read given the username of the reader
    void setRead(int startingIndex, int endingIndex, char readerUsername[g_UsernameLimit]) {
        messageMutex.lock();
        for (int i = startingIndex; i < endingIndex + 1; i++) {
            if (messageList[i].senderUsername != readerUsername && messageList[i].isRead != true ) {
                messageList[i].isRead = true;
                conversationsDictionary.notificationSeen(const_cast<char*>(messageList[i].senderUsername.c_str()), readerUsername);
            }
        }
        messageMutex.unlock();

    }

    // Returning the messages a user queries
    // TODO: lastMessageDeliveredIndex should be updated upon CONSECUTIVE query messages calls and reset when the user queries other messages
    // It should also not run if the lastMessageDelivered index is zero, 
    GetStoredMessagesReturnValue getStoredMessages(char readerUsername[g_UsernameLimit], int lastMessageDeliveredIndex) {
        // assert(lastMessageDeliveredIndex!=0);
        messageMutex.lock();

        // Keep track of the last unread message for that user
        GetStoredMessagesReturnValue returnValue;
        int currNumberOfMessages = messageList.size();
        int firstMessageIndex; 
        int lastMessageIndex; 

        // Calculate which messages need to be returned
        if (lastMessageDeliveredIndex == -1 || lastMessageDeliveredIndex == 0) {
            // If no previous messages were delivered
            firstMessageIndex = std::max(currNumberOfMessages - int(g_MessageQueryLimit), 0);
            lastMessageIndex = std::min(firstMessageIndex + int(g_MessageQueryLimit), currNumberOfMessages -1);
        } else {
            // If there were previous consecutive queries
            lastMessageIndex = lastMessageDeliveredIndex - 1;
            firstMessageIndex = std::max(lastMessageIndex - int(g_MessageQueryLimit), 0);
        }

        returnValue.firstMessageIndex = firstMessageIndex;
        returnValue.lastMessageIndex = lastMessageIndex;


        // Grab relevant messages
        for (int i = firstMessageIndex; i < lastMessageIndex+1; i++) {
            ReturnMessage newItem;
            
            strcpy(newItem.senderUsername, messageList[i].senderUsername.c_str());
            strcpy(newItem.messageContent, messageList[i].messageContent.c_str());

            returnValue.messageList.push_back(newItem);
        }

        messageMutex.unlock();

        return returnValue;

        
    }

};


template<>
struct std::hash<UserPair>
{
    size_t operator()(const UserPair& pair) const
    {
        std::string concat = pair.smallerUsername + pair.largerUsername;
        return std::hash<std::string>{}(concat);
    }
};

std::unordered_map<UserPair, StoredMessages> messagesDictionary;

std::mutex socketDictionary_mutex;
std::map<std::string, int> socketDictionary;

struct CharNode {
    char character;
    std::unordered_map<char, CharNode*> children;
    bool isTerminal;

    CharNode(char c, bool b) {
        character = c;
        isTerminal = b;
    }

    friend struct UserTrie;
};

std::unordered_map<CharNode*, std::string> userPasswordMap;


struct UserTrie {
    private:
        std::unordered_map<char, CharNode*> roots;

    public:
        // add new username to trie; return whether username was added successfully or not
        //      If username could not be added throws invalid_argument exception
        void addUsername(std::string username, std::string password) {
            if (!validString(username)) {
                std::cout << "Username '" << username << "' is invalid. Must be alphanumeric and at least 1 character." << std::endl;
            }

            std::pair<CharNode*, int> nodeIdxPair = findLongestMatchingPrefix(username);
            CharNode* currNode = nodeIdxPair.first;

            if (currNode == nullptr) {
                currNode = new CharNode(username[0], false);
                roots[username[0]] = currNode;
                nodeIdxPair.second = 0;
            }
            else if (currNode->isTerminal) {
                std::cout << "Username '" << username << "' has already been taken." << std::endl;
            }

            for (int idx = nodeIdxPair.second+1; idx < username.size(); idx++) {
                char c = username[idx];
                CharNode* newChild = new CharNode(c, false);
                currNode->children[c] = newChild;
                currNode = newChild;
            }

            currNode->isTerminal = true;
            userPasswordMap[currNode] = password;
        }

        // Returns a vector of users with given prefix, if none found returns a runtime exception
        std::vector<std::string> returnUsersWithPrefix(std::string usernamePrefix) {
            if (usernamePrefix.size() == 0) {
                // Perform DFS starting at deepest node
                std::vector<std::string> usersFound;
                performDFS(usernamePrefix, nullptr, usersFound);
                std::cout << "Got usernames" << std::endl;
                return usersFound;
            }
            std::pair<CharNode*, int> nodeIdxPair = findLongestMatchingPrefix(usernamePrefix);
            CharNode* deepestNode = nodeIdxPair.first;
            int index = nodeIdxPair.second;

            if (index != usernamePrefix.size()-1) {
                std::cout << "No usernames found for prefix '" << usernamePrefix << "'"<< std::endl;
                return {};
            }

            // Perform DFS starting at deepest node
            std::vector<std::string> usersFound;
            performDFS(usernamePrefix, deepestNode, usersFound);
            return usersFound;
        }

        std::pair<CharNode*, int> findLongestMatchingPrefix(std::string username) {
            if (roots.find(username[0]) == roots.end()) {
                return std::make_pair(nullptr, -1);
            }

            CharNode* deepestNode = roots[username[0]];

            // Find deepest matching prefix
            int idx = 1;
            while (idx < username.size()) {
                char c = username[idx];
                if ((deepestNode->children).find(c) == (deepestNode->children).end()) {
                    break;
                }
                deepestNode = deepestNode->children[c];
                idx++;
            }

            return std::make_pair(deepestNode, idx-1);
        }


        // Given a substring and current node, append to vector if node ends a username
        void performDFS(std::string substring, CharNode* currNode, std::vector<std::string> &usersFound) {
            if (substring.size() == 0 || currNode == nullptr) {
                for (auto it = roots.begin(); it != roots.end(); ++it) {
                    std::string newSubstr = substring + (*it).first;
                    performDFS(newSubstr, (*it).second, usersFound);
                }
                return;
            }
            if (currNode->isTerminal) {
                usersFound.push_back(substring);
            }
            for (auto it = currNode->children.begin(); it != currNode->children.end(); ++it) {
                std::string newSubstr = substring + (*it).first;
                performDFS(newSubstr, (*it).second, usersFound);
            }
        }

        bool userExists(std::string user) {
            std::pair<CharNode*, int> nodeIdxPair = findLongestMatchingPrefix(user);
            if (nodeIdxPair.first == nullptr || nodeIdxPair.second < user.size()-1 || !nodeIdxPair.first->isTerminal) {
                return false;
            }

            return nodeIdxPair.first->isTerminal;
        }

        bool verifyUser(std::string username, std::string password) {
            std::pair<CharNode*, int> nodeIdxPair = findLongestMatchingPrefix(username);
            if (nodeIdxPair.first == nullptr || nodeIdxPair.second < username.size()-1 || !nodeIdxPair.first->isTerminal) {
                std::cout << "User '" << username << "' not found." << std::endl;
                return false;
            }

            // return strcmp(password.c_str(), (userPasswordMap[nodeIdxPair.first]).c_str()) == 0;
            return password == userPasswordMap[nodeIdxPair.first];
        }

        void deleteUser(std::string username) {
            std::pair<CharNode*, int> nodeIdxPair = findLongestMatchingPrefix(username);
            if (nodeIdxPair.first == nullptr || nodeIdxPair.second < username.size()-1 || !nodeIdxPair.first->isTerminal) {
                std::cout << "User '" << username << "' not found." << std::endl;
                return;
            }

            nodeIdxPair.first->isTerminal = false;
            userPasswordMap.erase(nodeIdxPair.first);
        }
};
// TODO: Users Trie
std::mutex userTrie_mutex;
UserTrie userTrie;

// Global storage for new messsage operations
std::mutex queuedOperations_mutex;
std::unordered_map<int, std::vector<NewMessageMessage>> queuedOperationsDictionary;
std::unordered_map<int, bool> forceLogoutDictionary;


// Cleaning up session-related storage structures
void cleanup(std::string clientUsername, int client_fd) {
    std::cout << "Disconnecting client: " << clientUsername << std::endl;
    queuedOperationsDictionary.erase(client_fd);
    socketDictionary.erase(clientUsername);
    close(client_fd);
}

char threadExitReturnVal [50];
// Initialize Trie
// Add user
// Find all users associated with a substring
// Verify username/password pair