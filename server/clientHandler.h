#include "storage.h"

// Function that each thread corresponding with a user uses to reads from socket and perform operations.
void handleClient(int client_fd) {

    // Thread-based variables
    int valread;
    CurrentConversation currentConversation;    // Keeps track of conversation that was last queried.
    char clientUsername[g_UsernameLimit];       // Username of client associated with the thread.

    std::cout << "New thread up!" << std::endl;

    while (true) {
        // Force session to end if user logged in on a different device.
        if (forceLogoutDictionary.find(client_fd) != forceLogoutDictionary.end()) {
            std::cout << clientUsername << " forced to exit" << std::endl;
            forceLogoutDictionary.erase(client_fd);

            // Closing the thread
            strcpy(threadExitReturnVal, "Thread exited");
            pthread_exit(threadExitReturnVal);
        }

        // Send new message notifications to the client if they exist.
        if (queuedOperationsDictionary.find(client_fd) != queuedOperationsDictionary.end()) {
            std::cout << "Notifying '" << clientUsername << "' of new messages" << std::endl;
            for (int i = 0; i < queuedOperationsDictionary[client_fd].size(); i++) {
                NewMessageMessage newMessageMessage = queuedOperationsDictionary[client_fd][i];
                send(client_fd, &newMessageMessage, sizeof(newMessageMessage), 0);
            }

            queuedOperationsDictionary.erase(client_fd);
        }
        
        // Attempting to read an operation code from the socket.
        opCode operation;
        valread = read(client_fd, &operation, sizeof(opCode));

        // Cleans up if the client is disconnected.
        if (valread == 0) {
            std::cout << "User '"<< clientUsername <<"' disconnected" << std::endl;
            cleanup(clientUsername, client_fd);

            // Closing the thread
            strcpy(threadExitReturnVal, "Thread exited");
            pthread_exit(threadExitReturnVal);
        }

        // Handling different operations.
        switch (operation) {
            case CREATE_ACCOUNT:
            {
                // Parse Message.
                CreateAccountMessage createAccountMessage;
                createAccountMessage.parse(client_fd);
                int queryResult = 0; // No error.

                // Verify username.
                std::string username = createAccountMessage.userName;
                std::string password = createAccountMessage.password;

                if (userTrie.userExists(username)) {
                    std::cout << "Username '" << clientUsername << "' already exists." << std::endl;
                    queryResult = 1; // Username exists already. 
                } else {
                    // Update thread's username 
                    strcpy(clientUsername, createAccountMessage.userName);

                    // Update storage data structures with new user.
                    userTrie_mutex.lock();
                    userTrie.addUsername(username, password);
                    userTrie_mutex.unlock();

                    socketDictionary_mutex.lock();
                    socketDictionary[std::string(clientUsername)] = client_fd;
                    socketDictionary_mutex.unlock();
                }


                // Create and send a response.
                CreateAccountReply createAccountReply(queryResult);
                send(client_fd, &createAccountReply, sizeof(createAccountReply), 0);

                std::cout << "Acount created for '" << clientUsername << "'" << std::endl; 
            }
            break;
            case LOGIN:
            {
                // Parse message.
                LoginMessage loginMessage;
                loginMessage.parse(client_fd);
                int queryResult = 0; // No error

                // Verifying the user.
                std::string username = loginMessage.userName;
                std::string password = loginMessage.password;

                userTrie_mutex.lock();
                bool verified = userTrie.verifyUser(username, password);
                userTrie_mutex.unlock();


                if (verified) {
                    socketDictionary_mutex.lock();

                    // Check if person is already logged in.
                    if (socketDictionary.find(username) != socketDictionary.end()) {
                        std::cout << "Person '" << username << " already logged in" << std::endl;

                        // Send older session a forceLogOut message.
                        ForceLogOutReply forceLogOutReply;
                        send(socketDictionary[username], &forceLogOutReply, sizeof(forceLogOutReply), 0);

                        // Force the thread to clean itself up the next time it runs.
                        forceLogoutDictionary[socketDictionary[username]] = true;
                         
                    }
                    // Update socket with session and thread with username.
                    strcpy(clientUsername, username.c_str()); 
                    socketDictionary[std::string(clientUsername)] = client_fd;

                    socketDictionary_mutex.unlock();

                    std::cout << "'" << clientUsername << "' logged in" << std::endl;

                } else {
                    std::cout << "'" << clientUsername << "' not found." << std::endl;
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

                // Deleting user from session-related storage
                socketDictionary_mutex.lock();
                cleanup(std::string(clientUsername), client_fd);
                socketDictionary_mutex.unlock();

                // Killing the thread
                strcpy(threadExitReturnVal, "Thread exited");
                pthread_exit(threadExitReturnVal);
            } 
            break;
            case LIST_USERS:
            {
                // Parse message.
                ListUsersMessage listUsersMessage;
                listUsersMessage.parse(client_fd);

                // Get users with prefix.
                userTrie_mutex.lock();
                std::vector<std::string> usernames = userTrie.returnUsersWithPrefix(listUsersMessage.prefix);
                userTrie_mutex.unlock();

                // Sending code and number of usernames to expect.
                ListUsersReply listUsersReply(usernames.size());
                send(client_fd, &listUsersReply, sizeof(listUsersReply), 0);

                // Sending usernames.
                for (int i = 0; i < usernames.size(); i++) {
                    Username user(usernames[i]);
                    int valsent = send(client_fd, &user, sizeof(user), 0);
                }

                std::cout << "Listed users by request of '" << clientUsername << "'" << std::endl;

            }
            break;
            case SEND_MESSAGE:
            {
                // Parse message.
                SendMessageMessage sendMessageMessage;
                sendMessageMessage.parse(client_fd);
                int queryResult = 0; // Message sent!

                // Check if user exists
                bool userExists = userTrie.userExists(sendMessageMessage.recipientUsername);
                if (userExists) {
                    std::cout << "Adding message to storage from '" << clientUsername << "' to '" << sendMessageMessage.recipientUsername << "'" << std::endl;
                    // Add message to messages dictionary
                    UserPair userPair(clientUsername, sendMessageMessage.recipientUsername);
                    messagesDictionary[userPair].addMessage(clientUsername, sendMessageMessage.recipientUsername, sendMessageMessage.messageContent);

                    // W
                    NewMessageMessage newMessageMessage(clientUsername);
                    socketDictionary_mutex.lock();
                    if (socketDictionary.find(sendMessageMessage.recipientUsername) != socketDictionary.end()){
                        queuedOperations_mutex.lock();
                        queuedOperationsDictionary[socketDictionary[sendMessageMessage.recipientUsername]].push_back(newMessageMessage);
                        queuedOperations_mutex.unlock();
                    }

                    socketDictionary_mutex.unlock();

                } else {
                    std::cout << "Tried to send a message to a user that doesn't exist '" << sendMessageMessage.recipientUsername << "'" << std::endl;
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

                // for (int i = 0; i < notifications.size(); i++) {
                //     std::cout << "Username: " << notifications[i].first << ", " << std::to_string(notifications[i].second) << " notifications" << std::endl;
                // }

                // Construct and send a reply
                QueryNotificationReply queryNotificationsReply(notifications.size());
                send(client_fd, &queryNotificationsReply, sizeof(queryNotificationsReply), 0);
                
                for (int i = 0; i < notifications.size(); i++) {
                    std::cout << "Username: " << notifications[i].first << ", " << std::to_string(notifications[i].second) << " notifications" << std::endl;
                    int valsent = send(client_fd, &notifications[i], sizeof(std::pair<char[g_UsernameLimit], char>),0);
                    std::cout << "Sent " << std::to_string(valsent) << std::endl;
                }
                // int valsent = send(client_fd, notifications.data(), notifications.size()*sizeof(std::pair<char [g_UsernameLimit], char>),0);

                // std::cout << valsent << " bytes sent" << std::endl;

            }
            break;
            case QUERY_MESSAGES:
            {
                // Parse message
                QueryMessagesMessage queryMessagesMessage;
                queryMessagesMessage.parse(client_fd);
                int queryStatus = 0;

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

                for (int i = 0; i < returnVal.messageList.size(); i++) {
                    std::cout << returnVal.messageList[i].senderUsername<< " : " << returnVal.messageList[i].messageContent << std::endl;
                }

                // Construct and send a reply
                QueryMessagesReply queryMessagesReply(returnVal.messageList.size(), returnVal.firstMessageIndex);
                send(client_fd, &queryMessagesReply, sizeof(queryMessagesReply), 0);

                send(client_fd, returnVal.messageList.data(), sizeof(ReturnMessage) * returnVal.messageList.size(), 0);

                // for (int i = 0; i < returnVal.messageList.size(); i++) {
                //     int valsent = send(client_fd, &returnVal.messageList[i], sizeof(ReturnMessage), 0);
                std::cout << "QueryMessage reply sent" << std::endl;
                // }
                // send(client_fd, returnVal.messageList.data(), returnVal.messageList.size() * sizeof(ReturnMessage), 0);
                

            }
            break;
            case DELETE_ACCOUNT:
            {   
                std::cout << "Deleting account of '" << clientUsername << "'" << std::endl;
                // Flag user account as deleted in trie
                userTrie_mutex.lock();
                userTrie.deleteUser(std::string(clientUsername));
                userTrie_mutex.unlock();

                // Closes thread
                socketDictionary_mutex.lock();
                cleanup(clientUsername, client_fd);
                socketDictionary_mutex.unlock();

                strcpy(threadExitReturnVal, "Thread exited");
                pthread_exit(threadExitReturnVal);
 
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
            case REFRESH_REQUEST:
            {
                break;
            }
            break;
            default:
                std::cout << "Unrecognized operation\n";
                break;
        }
    }
}