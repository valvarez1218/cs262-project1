#include "../messageTypes.h"

// Boolean determining whether the user has logged in
bool USER_LOGGED_IN = false;

std::string loggedInErrorMsg(std::string operationAttempted) {
    return "User must be logged in to perform " + operationAttempted;
}

void createAccount(int socket_fd, CreateAccountMessage &create_account_message) {
    // throw std::runtime_error("createAccount() not implemented");
    send(socket_fd, &create_account_message, sizeof(CreateAccountMessage), 0);

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
    // throw std::runtime_error("login() not implemented");

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

    throw std::runtime_error("listUsers() not implemented");
}


void sendMessage(int socket_fd, SendMessageMessage &send_message_message) {
    if (!USER_LOGGED_IN) {
        throw std::runtime_error(loggedInErrorMsg("send_message"));
    }
    // throw std::runtime_error("sendMessage() not implemented");
    send(socket_fd, &send_message_message, sizeof(SendMessageMessage), 0);
}


void queryNotifications(int socket_fd, QueryNotificationsMessage &query_notification_message) {
    if (!USER_LOGGED_IN) {
        throw std::runtime_error(loggedInErrorMsg("query_notifications"));
    }

    throw std::runtime_error("queryNotifications() not implemented");
}


void queryMessages(int socket_fd, QueryMessagesMessage &query_messages_message) {
    if (!USER_LOGGED_IN) {
        throw std::runtime_error(loggedInErrorMsg("query_messages"));
    }

    throw std::runtime_error("queryMessages() not implemented");
}


void deleteAccount(int socket_fd, DeleteAccountMessage &delete_account_message) {
    if (!USER_LOGGED_IN) {
        throw std::runtime_error(loggedInErrorMsg("delete_account"));
    }

    throw std::runtime_error("deleteAccount() not implemented");
}