#include "../messageTypes.h"

void createAccount(int socket_fd, CreateAccountMessage create_account_message) {
    throw std::runtime_error("createAccount() not implemented");
}


void login(int socket_fd, LoginMessage login_message) {
    throw std::runtime_error("login() not implemented");
}


void logout(int socket_fd, LogoutMessage logout_message) {
    throw std::runtime_error("logout() not implemented");
}


void listUsers(int socket_fd, ListUsersMessage list_users_message) {
    throw std::runtime_error("listUsers() not implemented");
}


void sendMessage(int socket_fd, SendMessageMessage send_message_message) {
    // throw std::runtime_error("sendMessage() not implemented");
    send(socket_fd, &send_message_message, sizeof(SendMessageMessage), 0);
}


void queryNotifications(int socket_fd, QueryNotificationsMessage query_notification_message) {
    throw std::runtime_error("queryNotifications() not implemented");
}


void queryMessages(int socket_fd, QueryMessagesMessage query_messages_message) {
    throw std::runtime_error("queryMessages() not implemented");
}


void deleteAccount(int socket_fd, DeleteAccountMessage delete_account_message) {
    throw std::runtime_error("deleteAccount() not implemented");
}