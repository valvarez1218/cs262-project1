// This file should have the main code for handling a client (i.e. constantly listening 
//      to socket and executing necessary commands)

#include "storage.h"

// TODO: Function should read from socket and perform operations based on what it read
void handleClient(int client_fd) {
    int valread;
    CurrentConversation currentConversation;
    char clientUsername[g_UsernameLimit];

    while (true) {
        opCode operation;
        valread = read(client_fd, &operation, sizeof(opCode));

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

                int verified = userTrie.verifyUser(username, password);

                if (verified) {
                    queryResult = 1; // Username exists already, 
                } else {
                    // Update storage with new user
                    userTrie.addUsername(username, password);
                    strcpy(clientUsername, createAccountMessage.userName);
                    socketDictionary[clientUsername] = client_fd;
                }

                // Create and send a response
                CreateAccountReply createAccountReply(queryResult);
                send(client_fd, &createAccountReply, sizeof(createAccountReply), 0);

            }
            case LOGIN:
            {
                // Parse message
                LoginMessage loginMessage;
                loginMessage.parse(client_fd);
                int queryResult = 0; // No error

                std::string username = loginMessage.userName;
                std::string password = loginMessage.password;

                int verified = userTrie.verifyUser(username, password);

                if (verified) {
                    // Logged in! Update storage accordingly
                    strcpy(clientUsername, username.c_str());
                    socketDictionary[clientUsername] = client_fd;
                } else {
                    queryResult = 1; // Username and password don't match, username doesn't exist, user deleted
                }

                // Create and send response
                LoginReply loginReply(queryResult);
                send(client_fd, &loginReply, sizeof(loginReply), 0);

            }
            case LOGOUT:
                // Close the socket and remove account username from socket dictionary
                close(client_fd);
                socketDictionary.erase(clientUsername);
                break;

            case LIST_USERS:
            {
                // Parse message
                ListUsersMessage listUsersMessage;
                listUsersMessage.parse(client_fd);

                // TODO: check with nictor that this is how his thing works
                std::vector<std::string> usernames = userTrie.returnUsersWithPrefix(listUsersMessage.prefix);

                // Construct and send a reply
                ListUsersReply listUsersReply(usernames.size(), usernames);
                send(client_fd, &listUsersReply, sizeof(listUsersReply), 0);
            }

            case SEND_MESSAGE:
            {
                // Read from socket into sendMessageMessage
                SendMessageMessage sendMessageMessage;
                sendMessageMessage.parse(client_fd);

                // Add message to messages dictionary
                UserPair userPair(clientUsername, sendMessageMessage.recipientUsername);
                messagesDictionary[userPair].addMessage(clientUsername, sendMessageMessage.recipientUsername, sendMessageMessage.messageContent);

                // Notify recipient of a new message
                NewMessageMessage newMessageMessage(clientUsername, sendMessageMessage.messageContent);

                if (socketDictionary.find(sendMessageMessage.recipientUsername) != socketDictionary.end()){
                     send(socketDictionary[sendMessageMessage.recipientUsername], &newMessageMessage, sizeof(newMessageMessage), 0);

                }

            }
            case QUERY_NOTIFICATIONS:
            {
                // Retrieve notifications from the conversations dictionary
                std::vector<std::pair<char [g_UsernameLimit], char> > notifications = conversationsDictionary.getNotifications(clientUsername);

                // Construct and send a reply
                QueryNotificationReply queryMessagesReply(notifications.size(), notifications);
                send(client_fd, &queryMessagesReply, sizeof(queryMessagesReply), 0);
            }
            case QUERY_MESSAGES:
            {
                // Parse message
                QueryMessagesMessage queryMessagesMessage;
                queryMessagesMessage.parse(client_fd);

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
            case DELETE_ACCOUNT:
            {
                // TODO: Flag user account as deleted in trie

                // Close the socket and remove account username from socket dictionary
                close(client_fd);
                socketDictionary.erase(clientUsername);
                break;
            }
            case MESSAGES_SEEN:
            {
                // Parse message
                MessagesSeenMessage messagesSeenMessage;
                messagesSeenMessage.parse(client_fd);

                // Set messages as read
                UserPair userPair(messagesSeenMessage.otherUsername, clientUsername);
                messagesDictionary[userPair].setRead(messagesSeenMessage.startingIndex, messagesSeenMessage.startingIndex+messagesSeenMessage.messagesSeen - 1, clientUsername);

            }
            default:
                // TODO: should report error if not a recognized operation
                break;
        }
    }
}