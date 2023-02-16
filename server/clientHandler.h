// This file should have the main code for handling a client (i.e. constantly listening 
//      to socket and executing necessary commands)

#include "storage.h"

// TODO: Function should read from socket and perform operations based on what it read
void handleClient(int client_fd) {
    int valread;
    CurrentConversation currentConversation;
    char threadUser[g_UsernameLimit];

    while (true) {
        opCode operation;
        valread = read(client_fd, &operation, sizeof(opCode));

        switch (operation) {
            case CREATE_ACCOUNT:
                CreateAccountMessage createAccountMessage;
                createAccountMessage.parse(client_fd);

                strcpy(threadUser, createAccountMessage.userName);

                socketDictionary[threadUser] = client_fd;

            case LOGIN:

            case LOGOUT:
                // remove account username from socket dictionary
                socketDictionary.erase(threadUser);

            case LIST_USERS:

            case SEND_MESSAGE:
            {
                // Read from socket into sendMessageMessage
                SendMessageMessage sendMessageMessage;
                sendMessageMessage.parse(client_fd);

                // Add message to messages dictionary
                UserPair userPair(threadUser, sendMessageMessage.recipientUsername);
                messagesDictionary[userPair].addMessage(threadUser, sendMessageMessage.recipientUsername, sendMessageMessage.messageContent);

                // Notify recipient of a new message
                NewMessageMessage newMessageMessage(threadUser, sendMessageMessage.messageContent);

                if (socketDictionary.find(sendMessageMessage.recipientUsername) != socketDictionary.end()){
                     send(socketDictionary[sendMessageMessage.recipientUsername], &newMessageMessage, sizeof(newMessageMessage), 0);

                }
            }
            case QUERY_NOTIFICATIONS:
            {
                // make getNoti
                std::vector<std::pair<char [g_UsernameLimit], char> > notifications = conversationsDictionary.getNotifications(threadUser);

                QueryNotificationReply queryMessagesReply(notifications.size(), notifications);

                send(client_fd, &queryMessagesReply, sizeof(queryMessagesReply), 0);
            }
            case QUERY_MESSAGES:
            {
                QueryMessagesMessage queryMessagesMessage;
                queryMessagesMessage.parse(client_fd);

                UserPair userPair(queryMessagesMessage.username, threadUser);
                int lastMessageDeliveredIndex = -1;
                if (currentConversation.username == queryMessagesMessage.username) {
                    lastMessageDeliveredIndex = currentConversation.messagesSentStartIndex;
                } else {
                    strcpy(currentConversation.username, queryMessagesMessage.username);
                }

                GetStoredMessagesReturnValue returnVal = messagesDictionary[userPair].getStoredMessages(threadUser,lastMessageDeliveredIndex);

                currentConversation.messagesSentStartIndex = returnVal.firstMessageIndex;
                currentConversation.messagesSentEndIndex = returnVal.lastMessageIndex;

                send(client_fd, &returnVal.messageList, sizeof(returnVal.messageList), 0);

                
            }
            case DELETE_ACCOUNT:
            // remove account username from socket dictionary
            socketDictionary.erase(threadUser);

            case MESSAGES_SEEN:

            default:
                // TODO: should report error if not a recognized operation
                break;
        }
    }
}