#include <gtest/gtest.h>
#include <thread>
#include "../client/takeInput.h"

// Server side C/C++ program to demonstrate Socket
// programming

// returns number of bytes read/send
// int handleMessage (int socket_fd, opCode messageType, bool isReading) {
//     switch (messageType) {
//         case CREATE_ACCOUNT:
// 			if (isReading) {
// 				CreateAccountMessage create_account_message;
// 				int valread = read(socket_fd, &create_account_message, sizeof(CreateAccountMessage));
// 				return valread;
// 			} else {
// 				CreateAccountMessage create_account_message;
// 				int valsent = send(socket_fd, &create_account_message, sizeof(CreateAccountMessage), 0);
// 				return valsent;
// 			}

//         case LOGIN:

//         case LOGOUT:

//         case LIST_USERS:

//         case SEND_MESSAGE:

//         case QUERY_NOTIFICATIONS:

//         case QUERY_MESSAGES:

//         case DELETE_ACCOUNT:

//         case MESSAGES_SEEN:

//         case NEW_MESSAGE:

//         case CREATE_ACCOUNT_REPLY:

//         case LOGIN_REPLY:

//         case LIST_USERS_REPLY:

//         case SEND_MESSAGE_REPLY:

// 		case QUERY_NOTIFICATIONS_REPLY:

// 		case QUERY_MESSAGES_REPLY:

// 		case MESSAGES_SEEN_REPLY:
//     }
// }

int setupServer(int &valread) {
	int server_fd, new_socket;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET,
				SO_REUSEADDR | SO_REUSEPORT, &opt,
				sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	// Forcefully attaching socket to the port 8080
	if (bind(server_fd, (struct sockaddr*)&address,
			sizeof(address))
		< 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
	if ((new_socket
		= accept(server_fd, (struct sockaddr*)&address,
				(socklen_t*)&addrlen))
		< 0) {
		perror("accept");
		exit(EXIT_FAILURE);
	}


	// closing the connected socket
	close(new_socket);
	// closing the listening socket
	shutdown(server_fd, SHUT_RDWR);
	pthread_exit(&valread);
}


// Client side C/C++ program to demonstrate Socket
// programming

int setupClient(int &valsent, opCode messageType) {
	int sock = 0, client_fd;
	struct sockaddr_in serv_addr;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n Socket creation error \n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	// Convert IPv4 and IPv6 addresses from text to binary
	// form
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)
		<= 0) {
		printf(
			"\nInvalid address/ Address not supported \n");
		return -1;
	}

	if ((client_fd
		= connect(sock, (struct sockaddr*)&serv_addr,
				sizeof(serv_addr)))
		< 0) {
		printf("\nConnection Failed \n");
		return -1;
	}

	// closing the connected socket
	close(client_fd);
    pthread_exit(&valsent);
}



TEST(MessageSizeTest, CheckSizeOfMessage) {
    // int valread;
    // int valsent;
    // std::thread clientThread(setupClient, valsent);
    // std::thread serverThrea(setupServer, valread);

	// CreateAccountMessage create_account_message;
	EXPECT_EQ(sizeof(CreateAccountMessage), sizeof(opCode)+g_UsernameLimit+g_PasswordLimit);
	EXPECT_EQ(sizeof(LoginMessage), sizeof(opCode)+g_UsernameLimit+g_PasswordLimit);
	EXPECT_EQ(sizeof(LogoutMessage), sizeof(opCode));
	EXPECT_EQ(sizeof(ListUsersMessage), sizeof(opCode)+g_UsernameLimit);
	EXPECT_EQ(sizeof(SendMessageMessage), sizeof(opCode)+g_UsernameLimit+g_MessageLimit);
	EXPECT_EQ(sizeof(QueryNotificationsMessage), sizeof(opCode));
	EXPECT_EQ(sizeof(QueryMessagesMessage), sizeof(opCode)+g_UsernameLimit);
	EXPECT_EQ(sizeof(DeleteAccountMessage), sizeof(opCode)+g_UsernameLimit+g_PasswordLimit);
	EXPECT_EQ(sizeof(MessagesSeenMessage), 2*sizeof(char)+sizeof(opCode));
	EXPECT_EQ(sizeof(NewMessageMessage), sizeof(opCode)+g_UsernameLimit);
	EXPECT_EQ(sizeof(CreateAccountReply), sizeof(char)+sizeof(opCode));
	EXPECT_EQ(sizeof(LoginReply), sizeof(char)+sizeof(opCode));
	EXPECT_EQ(sizeof(Username), g_UsernameLimit);

	ListUsersReply list_users_reply;
	list_users_reply.numberOfUsers = 4;
	// list_users_reply.usernames.push_back(Username("User1"));
	// list_users_reply.usernames.push_back(Username("User2"));
	// list_users_reply.usernames.push_back(Username("User3"));
	// list_users_reply.usernames.push_back(Username("User4"));
	EXPECT_EQ(sizeof(list_users_reply), sizeof(char)+sizeof(opCode));

	EXPECT_EQ(sizeof(SendMessageReply), sizeof(opCode)+sizeof(char));
	
	int numberUsers = 4;
	std::pair<char [g_UsernameLimit], char> notification1;
	std::pair<char [g_UsernameLimit], char> notification2;
	std::pair<char [g_UsernameLimit], char> notification3;
	std::pair<char [g_UsernameLimit], char> notification4;
	std::vector<std::pair<char [g_UsernameLimit], char>> notificationsList;
	notificationsList.push_back(notification1);
	notificationsList.push_back(notification2);
	notificationsList.push_back(notification3);
	notificationsList.push_back(notification4);
	QueryNotificationReply query_notifications_reply(numberUsers);
	EXPECT_EQ(sizeof(query_notifications_reply), sizeof(int)+sizeof(opCode)+4*sizeof(std::pair<char[g_UsernameLimit], char>));

	EXPECT_EQ(sizeof(ReturnMessage), g_UsernameLimit+g_MessageLimit);

	ReturnMessage msg1;
	ReturnMessage msg2;
	ReturnMessage msg3;
	ReturnMessage msg4;
	std::vector<ReturnMessage> messageList{msg1, msg2, msg3, msg4};
	QueryMessagesReply query_messages_reply(4, -1);
	EXPECT_EQ(sizeof(query_messages_reply), 2*sizeof(char)+sizeof(opCode));

	EXPECT_EQ(sizeof(ForceLogOutReply), sizeof(opCode));
}