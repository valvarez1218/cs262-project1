// This file should have the main code for handling a client (i.e. constantly listening 
//      to socket and executing necessary commands)

#include "storage.h"

// Reads from socket and perform operations based on what it read
void handleClient(int client_fd) {
    int valread;
    CurrentConversation currentConversation;
    char clientUsername[g_UsernameLimit];
    std::thread::id thread_id; std::this_thread::get_id();

    std::cout << "New thread up!" << std::endl;

    while (true) {
        // Run queued operations if they exist
        if (queuedOperationsDictionary.find(thread_id) != queuedOperationsDictionary.end()) {
            for (int i = 0; i < queuedOperationsDictionary[thread_id].size(); i++) {
                NewMessageMessage newMessageMessage = queuedOperationsDictionary[thread_id][i];

                send(socketDictionary[clientUsername].second, &newMessageMessage, sizeof(newMessageMessage), 0);

            }

            queuedOperationsDictionary.erase(thread_id);
        }
        
        opCode operation;
        valread = read(client_fd, &operation, sizeof(opCode));
        std::cout << "We've read an operation code!" << std::to_string(operation) << std::endl;

        switch (operation) {
            case CREATE_ACCOUNT:
            {
                // Parse Message
                CreateAccountMessage createAccountMessage;
                createAccountMessage.parse(client_fd);
                int queryResult = 0; // No error

                // Verify username
                std::string username = createAccountMessage.userName;
                std::string password = createAccountMessage.password;



                if (userTrie.userExists(username)) {
                    std::cout << "Username '" << clientUsername << "' already existed." << std::endl;
                    queryResult = 1; // Username exists already, 
                } else {
                    // Update storage with new user
                    std::cout << "Adding user '" << username << " to trie" << std::endl;
                    userTrie_mutex.lock();
                    userTrie.addUsername(username, password);
                    userTrie_mutex.unlock();

                    std::cout << "Adding user '" << username << " to socketDict" << std::endl;
                    strcpy(clientUsername, createAccountMessage.userName);
                    std::pair<std::thread::id, int> handlerDescriptor(thread_id, client_fd);
                    socketDictionary_mutex.lock();
                    socketDictionary[std::string(clientUsername)] = handlerDescriptor;
                    socketDictionary_mutex.unlock();

                    std::cout << "Username '" << clientUsername << " verified." << std::endl;
                }


                // Create and send a response
                CreateAccountReply createAccountReply(queryResult);
                send(client_fd, &createAccountReply, sizeof(createAccountReply), 0);

            }
            break;
            case LOGIN:
            {
                // Parse message
                LoginMessage loginMessage;
                loginMessage.parse(client_fd);
                int queryResult = 0; // No error

                std::string username = loginMessage.userName;
                std::string password = loginMessage.password;

                userTrie_mutex.lock();
                bool verified = userTrie.verifyUser(username, password);
                userTrie_mutex.unlock();

                if (verified) {
                    // Check if person is already logged in
                    socketDictionary_mutex.lock();
                    threadDictionary_mutex.lock();
                    if (socketDictionary.find(std::string(clientUsername)) != socketDictionary.end()) {
                        std::cout << "Person '" << clientUsername << " already logged in" << std::endl;
                        // Close socket
                        ForceLogOutReply forceLogOutReply;
                        send(socketDictionary[std::string(clientUsername)].second, &forceLogOutReply, sizeof(forceLogOutReply), 0);
                        
                        // Kill thread handling original user session
                        cleanup(std::string(clientUsername), socketDictionary[std::string(clientUsername)].first, socketDictionary[std::string(clientUsername)].second);
                         
                    }
                    // Logged in! Update storage accordingly
                    std::cout << "Logging in username '" << username << "'" << std::endl;
                    strcpy(clientUsername, username.c_str());
                    std::pair<std::thread::id, int> handlerDescriptor(thread_id, client_fd);
                    socketDictionary[std::string(clientUsername)] = handlerDescriptor;

                    threadDictionary_mutex.unlock();
                    socketDictionary_mutex.unlock();
                } else {
                    std::cout << "Username '" << clientUsername << "' not found." << std::endl;
                    queryResult = 1; // Username and password don't match, username doesn't exist, user deleted
                }

                // Create and send response
                LoginReply loginReply(queryResult);
                send(client_fd, &loginReply, sizeof(loginReply), 0);

            }
            break;
            case LOGOUT:
            {
                std::cout << "Logging out username '" << clientUsername << "'" << std::endl;
                // Closes thread
                socketDictionary_mutex.lock();
                threadDictionary_mutex.lock();
                cleanup(std::string(clientUsername), thread_id, client_fd);
                socketDictionary_mutex.unlock();
                threadDictionary_mutex.unlock();
            } 
            break;
            case LIST_USERS:
            {
                // Parse message
                ListUsersMessage listUsersMessage;
                listUsersMessage.parse(client_fd);

                std::cout << "Getting users for '" << clientUsername << "'" << std::endl;
                // Get users with prefix
                userTrie_mutex.lock();
                std::vector<std::string> usernames = userTrie.returnUsersWithPrefix(listUsersMessage.prefix);
                userTrie_mutex.unlock();

                // Construct and send a reply
                ListUsersReply listUsersReply(usernames.size(), usernames);
                send(client_fd, &listUsersReply, sizeof(listUsersReply), 0);

            }
            break;
            case SEND_MESSAGE:
            {
                // Read from socket into sendMessageMessage
                SendMessageMessage sendMessageMessage;
                sendMessageMessage.parse(client_fd);
                int queryResult = 0; // Message sent!

                bool userExists = userTrie.userExists(clientUsername);

                if (userExists) {
                    std::cout << "Adding message to storage for '" << clientUsername << "'" << std::endl;
                    // Add message to messages dictionary
                    UserPair userPair(clientUsername, sendMessageMessage.recipientUsername);
                    messagesDictionary[userPair].addMessage(clientUsername, sendMessageMessage.recipientUsername, sendMessageMessage.messageContent);

                    // Queue operation to send
                    // TOOD:   NewMessageMessage newMessageMessage(clientUsername);

                    NewMessageMessage newMessageMessage(clientUsername, sendMessageMessage.messageContent);
                    socketDictionary_mutex.lock();
                    if (socketDictionary.find(sendMessageMessage.recipientUsername) != socketDictionary.end()){
                        queuedOperations_mutex.lock();
                        queuedOperationsDictionary[socketDictionary[sendMessageMessage.recipientUsername].first].push_back(newMessageMessage);
                        queuedOperations_mutex.unlock();
                    }
                    socketDictionary_mutex.unlock();

                } else {
                    std::cout << "Tried to send a message to a user that doesn't exist '" << clientUsername << "'" << std::endl;
                     queryResult = 1; // User doesn't exist
                }

                SendMessageReply sendMessageReply(queryResult);
                send(client_fd, &sendMessageReply, sizeof(sendMessageReply), 0);

            }
            break;
            case QUERY_NOTIFICATIONS:
            {
                std::cout << "Getting notifications for '" << clientUsername << "'" << std::endl;
                // Retrieve notifications from the conversations dictionary
                std::vector<std::pair<char [g_UsernameLimit], char> > notifications = conversationsDictionary.getNotifications(clientUsername);

                // Construct and send a reply
                QueryNotificationReply queryNotificationsReply(notifications.size(), notifications);
                send(client_fd, &queryNotificationsReply, sizeof(queryNotificationsReply), 0);

            }
            break;
            case QUERY_MESSAGES:
            {
                // Parse message
                QueryMessagesMessage queryMessagesMessage;
                queryMessagesMessage.parse(client_fd);

                std::cout << "Getting messages between '" << clientUsername << "' and '"<< queryMessagesMessage.username << "'" << std::endl;

                // Get stored messages depending on if the client has the conversation open
                UserPair userPair(queryMessagesMessage.username, clientUsername);
                int lastMessageDeliveredIndex = -1;
                if (currentConversation.username == queryMessagesMessage.username) {
                    lastMessageDeliveredIndex = currentConversation.messagesSentStartIndex;
                } else {
                    strcpy(currentConversation.username, queryMessagesMessage.username);
                }

                GetStoredMessagesReturnValue returnVal = messagesDictionary[userPair].getStoredMessages(clientUsername,lastMessageDeliveredIndex);

                // Update current conversation information
                currentConversation.messagesSentStartIndex = returnVal.firstMessageIndex;
                currentConversation.messagesSentEndIndex = returnVal.lastMessageIndex;

                // Construct and send a reply
                QueryMessagesReply queryMessagesReply(returnVal.messageList.size(), returnVal.firstMessageIndex, returnVal.messageList);
                send(client_fd, &queryMessagesReply, sizeof(queryMessagesReply), 0);
                

            }
            break;
            case DELETE_ACCOUNT:
            {   
                std::cout << "Deleting account of '" << clientUsername << "'" << std::endl;
                // Flag user account as deleted in trie
                userTrie_mutex.lock();
                userTrie.deleteUser(clientUsername);
                userTrie_mutex.unlock();

                // Closes thread
                socketDictionary_mutex.lock();
                threadDictionary_mutex.lock();
                cleanup(clientUsername, thread_id, client_fd);
                socketDictionary_mutex.unlock();
                threadDictionary_mutex.unlock();
 
            }
            break;
            case MESSAGES_SEEN:
            {
                // Parse message
                MessagesSeenMessage messagesSeenMessage;
                messagesSeenMessage.parse(client_fd);

                // Set messages as read
                UserPair userPair(currentConversation.username, clientUsername);
                // if (messagesSeenMessage.startingIndex == -1)
                messagesDictionary[userPair].setRead(messagesSeenMessage.startingIndex, messagesSeenMessage.startingIndex+messagesSeenMessage.messagesSeen - 1, clientUsername);

            }
            break;
            default:
                std::cout << "Unrecognized operation\n";
                break;
        }
    }
}