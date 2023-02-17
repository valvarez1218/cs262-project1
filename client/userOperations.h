#include "../messageTypes.h"
#include "establishConnection.h"

// Boolean determining whether the user has logged in
bool USER_LOGGED_IN = false;

std::string loggedInErrorMsg(std::string operationAttempted) {
    return "User must be logged in to perform " + operationAttempted;
}

void createAccount(int socket_fd, CreateAccountMessage &create_account_message) {
    if (USER_LOGGED_IN) {
        throw std::runtime_error("Cannot create account if already logged in.");
    }

    int valsent = send(socket_fd, &create_account_message, sizeof(CreateAccountMessage), 0);

    CreateAccountReply serverReply;
    int valread = read(socket_fd, &serverReply, sizeof(CreateAccountReply));

    if (valread == -1) {
        throw std::runtime_error("Error reading server response from socket.");
    }

    if (serverReply.queryStatus == 1) {
        throw std::runtime_error("Username was taken, could not create account.");
    }

    std::cout << "Welcome " << create_account_message.userName << "!" << std::endl;

    USER_LOGGED_IN = true;
}


void login(int socket_fd, LoginMessage &login_message) {
    if (USER_LOGGED_IN) {
        throw std::runtime_error("Cannot log in if already logged in.");
    }

    send(socket_fd, &login_message, sizeof(LoginMessage), 0);
    LoginReply serverReply;
    int valread = read(socket_fd, &serverReply, sizeof(LoginReply));

    if (valread == -1) {
        throw std::runtime_error("Error reading server response from socket.");
    }

    if (serverReply.queryStatus == 1) {
        throw std::runtime_error("Incorrect username or password.");
    }

    USER_LOGGED_IN = true;
}


// ALL FUNCTIONS BELOW HERE REQUIRE USER TO BE LOGGED IN


void logout(int socket_fd, LogoutMessage &logout_message) {
    if (!USER_LOGGED_IN) {
        throw std::runtime_error(loggedInErrorMsg("logout"));
    }

    send(socket_fd, &logout_message, sizeof(LogoutMessage), 0);
}


void listUsers(int socket_fd, ListUsersMessage &list_users_message) {
    if (!USER_LOGGED_IN) {
        throw std::runtime_error(loggedInErrorMsg("list_users"));
    }

    send(socket_fd, &list_users_message, sizeof(ListUsersMessage), 0);

    ListUsersReply serverReply;
    
    try {
        // the parse function prints usernames to command line
        serverReply.readUsernames(socket_fd);
    } catch(std::runtime_error &e) {
        throw e;
    }
}


void sendMessage(int socket_fd, SendMessageMessage &send_message_message) {
    if (!USER_LOGGED_IN) {
        throw std::runtime_error(loggedInErrorMsg("send_message"));
    }

    send(socket_fd, &send_message_message, sizeof(SendMessageMessage), 0);
}


void queryNotifications(int socket_fd, QueryNotificationsMessage &query_notification_message) {
    if (!USER_LOGGED_IN) {
        throw std::runtime_error(loggedInErrorMsg("query_notifications"));
    }

    send(socket_fd, &query_notification_message, sizeof(QueryMessagesMessage), 0);

    QueryNotificationReply serverReply;
    serverReply.readNotifications(socket_fd);
}


void queryMessages(int socket_fd, QueryMessagesMessage &query_messages_message) {
    if (!USER_LOGGED_IN) {
        throw std::runtime_error(loggedInErrorMsg("query_messages"));
    }

    send(socket_fd, &query_messages_message, sizeof(QueryMessagesMessage), 0);

    QueryMessagesReply serverReply;
    try {
        int messagesRead = serverReply.readMessages(socket_fd);
        int firstIdx = serverReply.firstMessageIndex;
        MessagesSeenMessage msg(messagesRead, firstIdx);
        send(socket_fd, &msg, sizeof(MessagesSeenMessage), 0);
    } catch (std::runtime_error &e) {
        MessagesSeenMessage msg(0, -1);
        send(socket_fd, &msg, sizeof(MessagesSeenMessage), 0);
        throw e;
    }

}


void deleteAccount(int socket_fd, DeleteAccountMessage &delete_account_message) {
    if (!USER_LOGGED_IN) {
        throw std::runtime_error(loggedInErrorMsg("delete_account"));
    }

    send(socket_fd, &delete_account_message, sizeof(DeleteAccountMessage), 0);
}


// handle server messages
void readSocket() {
    opCode operation;
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 400;
    setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    int valread = read(server_socket, &operation, sizeof(opCode));
    std::cout << "We're in read socket. We read " << std::to_string(operation) << std::endl;

    switch (operation) {
        case NEW_MESSAGE:
            {
                NewMessageMessage msg;
                try {
                    msg.parse(server_socket);
                } catch (std::runtime_error &e) {
                    throw e;
                }
            }
            break;
        case FORCE_LOG_OUT:
            {
                std::cout << "Log in on another device detected. Session ended." << std::endl;
                USER_LOGGED_IN = false;
            }
            break;
    }
    std::cout << "We're leaving read socket" << std::endl;
    tv.tv_usec = 0;
    setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
}