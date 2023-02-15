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
                msg.parse(client_fd);

                threadUser = createAccountMessage.userName;

                socketDictionary[threadUser] = client_fd;

            case LOGIN:

            case LOGOUT:

            case LIST_USERS:

            case SEND_MESSAGE:
                SendMessageMessage sendMessageMessage;
                sendMessageMessage.parse(client_fd);

                UserPair userPair(threadUser, sendMessageMessage.recipientUsername);

                messagesDictionary[userPair].addMessage(threadUser, sendMessageMessage.recipientUsername, sendMessageMessage.messageContent);

                NewMessageMessage newMessageMessage()

            case QUERY_NOTIFICATIONS:

            case QUERY_MESSAGES:

            case DELETE_ACCOUNT:

            case MESSAGES_SEEN:

            default:
                // TODO: should report error if not a recognized operation
                break;
        }
    }
}