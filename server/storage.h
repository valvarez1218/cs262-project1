#include "../messageTypes.h"

#include <map>
#include <tuple>
#include <thread>
#include <mutex>
#include <algorithm>
#include <atomic>
#include <unordered_set>


// TODO: ask nictor why this doesnt work
// struct Notifications {
//     int value = 0;
//     // std::mutex valueMutex;

//     void change(int toAdd) {
//         // valueMutex.lock();
//         std::cerr << "value before " << value << "\n";
//         value += toAdd;
//         std::cerr << "value after " << value << "\n";
//         // valueMutex.unlock();
//     }
// };

struct CurrentConversation {
    char username[g_UsernameLimit];
    int requestsReceived;
};

// Key: user with active conversations, Value: map from users to number of notifications they have
struct ConversationsDictionary {
    std::map<std::string, std::map<std::string, int>> conversations;
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
    void getNotifications(char recipientUsername) {
    
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
};

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
    std::vector<StoredMessage> messageList;
    int lastMessageIndex;
    int firstMessageIndex;
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
            if (messageList[i].senderUsername != readerUsername) {
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
        assert(lastMessageDeliveredIndex!=0);
        messageMutex.lock();

        // Keep track of the last unread message for that user
        GetStoredMessagesReturnValue returnValue;
        int currNumberOfMessages = messageList.size();
        int firstMessageIndex; 
        int lastMessageIndex; 

        // Calculate which messages need to be returned
        if (lastMessageDeliveredIndex == -1) {
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
            returnValue.messageList.push_back(messageList[i]);
        }

        messageMutex.unlock();

        return returnValue;

        
    }

};

std::map<UserPair, StoredMessages> messagesDictionary;
std::map<char[g_UsernameLimit], int> socketDictionary;


struct CharNode {
    char character;
    std::unordered_map<char, CharNode*> children;
    bool endsEntry;

    CharNode(char c, bool b) {
        character = c;
        endsEntry = b;
    }
};


struct UserTrie {
    std::unordered_map<char, CharNode*> roots;

    void returnUsersWithPrefix(std::string usernamePrefix) {
        if (roots.find(usernamePrefix[0]) == roots.end()) {
            // TODO: Throw error that says it wasnt found?
            return;
        }

        CharNode* root = roots[usernamePrefix[0]];
        CharNode* currNode = root;

        std::vector<std::string> user
        // perform BFS over user Trie
        for (char c : usernamePrefix) {

        }

    }
}
// TODO: Users Trie
// Initialize Trie
// Add user
// Find all users associated with a substring
// Verify username/password pair
