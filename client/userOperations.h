#include "../messageTypes.h"

// Boolean determining whether the user has logged in
bool USER_LOGGED_IN = false;

void printLoggedInError(std::string operationAttempted) {
    std::cout << "User must be logged in to perform " << operationAttempted << std::endl;
}

void createAccount(int socket_fd, CreateAccountMessage create_account_message) {
    throw std::runtime_error("createAccount() not implemented");


    USER_LOGGED_IN = true;
}


void login(int socket_fd, LoginMessage login_message) {
    throw std::runtime_error("login() not implemented");


    USER_LOGGED_IN = true;
}


// ALL FUNCTIONS BELOW HERE REQUIRE USER TO BE LOGGED IN
void logout(int socket_fd, LogoutMessage logout_message) {
    if (!USER_LOGGED_IN) {
        printLoggedInError("logout");
        return;
    }

    // TODO

    throw std::runtime_error("logout() not implemented");
}


void listUsers(int socket_fd, ListUsersMessage list_users_message) {
    if (!USER_LOGGED_IN) {
        printLoggedInError("list_users");
        return;
    }

    throw std::runtime_error("listUsers() not implemented");
}


void sendMessage(int socket_fd, SendMessageMessage send_message_message) {
    if (!USER_LOGGED_IN) {
        printLoggedInError("send_message");
        return;
    }
    // throw std::runtime_error("sendMessage() not implemented");
    send(socket_fd, &send_message_message, sizeof(SendMessageMessage), 0);
}


void queryNotifications(int socket_fd, QueryNotificationsMessage query_notification_message) {
    if (!USER_LOGGED_IN) {
        printLoggedInError("query_notifications");
        return;
    }

    throw std::runtime_error("queryNotifications() not implemented");
}


void queryMessages(int socket_fd, QueryMessagesMessage query_messages_message) {
    if (!USER_LOGGED_IN) {
        printLoggedInError("query_messages");
        return;
    }

    throw std::runtime_error("queryMessages() not implemented");
}


void deleteAccount(int socket_fd, DeleteAccountMessage delete_account_message) {
    if (!USER_LOGGED_IN) {
        printLoggedInError("delete_account");
        return;
    }

    throw std::runtime_error("deleteAccount() not implemented");
}