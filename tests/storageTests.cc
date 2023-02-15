#include <gtest/gtest.h>
#include "../server/storage.h"
#include "./testInfra.h"

void dummyIncrement(int Z, ConversationsDictionary* testConversationsDictionary, char sender[g_UsernameLimit], char recipient[g_UsernameLimit]) {
  for (int i = 0; i < Z; i++) {
      testConversationsDictionary->newNotification(sender, recipient);
  }

  std::cerr << "incremented\n";

};

void dummyDecrement(int Z, ConversationsDictionary* testConversationsDictionary, char sender[g_UsernameLimit], char recipient[g_UsernameLimit]) {
  for (int i = 0; i < Z; i++) {
      testConversationsDictionary->notificationSeen(sender, recipient);
  }

  std::cerr << "decremented\n";

};

TEST(ConversationsDictionaryTest, SingleThreadModifications) {
  char sender[g_UsernameLimit] = "carolyn";
  char recipient[g_UsernameLimit] = "victor";

  ConversationsDictionary testConversationsDictionary;

  // Incrementing
  dummyIncrement(1, &testConversationsDictionary, sender, recipient);

  EXPECT_EQ(testConversationsDictionary.conversations[recipient][sender], 1);

  dummyIncrement(20, &testConversationsDictionary, sender, recipient);


  EXPECT_EQ(testConversationsDictionary.conversations[recipient][sender], 21);

  // Decrementing
  dummyDecrement(5, &testConversationsDictionary, sender, recipient);

  EXPECT_EQ(testConversationsDictionary.conversations[recipient][sender], 16);

}

TEST(ConversationsDictionaryTest, MultiThreadedModifications) {
  char sender[g_UsernameLimit] = "carolyn";
  char recipient[g_UsernameLimit] = "victor";

  ConversationsDictionary testConversationsDictionary;

  std::thread t1(dummyIncrement,1, &testConversationsDictionary, sender, recipient);
  std::thread t2(dummyDecrement,1, &testConversationsDictionary, sender, recipient);

  t1.join();
  t2.join();

  EXPECT_EQ(testConversationsDictionary.conversations[recipient][sender], 0);
}


TEST(MessagesDictionaryTest, UserPairExpectations) {
  char username1[g_UsernameLimit] = "CaRolyn";
  char username2[g_UsernameLimit] = "VictOr";

  // Arguments in alphabetical order.
  UserPair testpair1(username1, username2);

  // Expect smallerUsername to be Carolyn.
  EXPECT_EQ(testpair1.smallerUsername, username1);
  
  // Expect largerUsername to be Victor.
  EXPECT_EQ(testpair1.largerUsername, username2);

  // Arguments in reverse alphabetical order.
  UserPair testpair2(username2, username1);

  // Expect smallerUsername to be Carolyn.
  EXPECT_EQ(testpair2.smallerUsername, username1);
  
  // Expect largerUsername to be Carolyn.
  EXPECT_EQ(testpair2.largerUsername, username2);

  char username3[g_UsernameLimit] = "1fdgaslkf";
  char username4[g_UsernameLimit] = "3kjdsfUHFD!";

  // Arguments contain non-alphabetic characters.
  UserPair testpair3(username3, username4);

  // Expect smallerUsername to be 1fdgaslkf.
  EXPECT_EQ(testpair3.smallerUsername, username3);
  
  // Expect largerUsername to be 3kjdsfUHFD!
  EXPECT_EQ(testpair3.largerUsername, username4);
}

TEST(MessagesDictionaryDict, StoringAndReadingMessages) {
  char username1[g_UsernameLimit] = "carolyn";
  char username2[g_UsernameLimit] = "victor";
  char message[g_MessageLimit] = "Hello, Victor!";

  StoredMessages testStoredMessages;

  // Adding messages
  testStoredMessages.addMessage(username1, username2, message);

  // Messages are added correctly
  EXPECT_EQ(testStoredMessages.messageList[0].senderUsername, username1);
  EXPECT_EQ(testStoredMessages.messageList[0].isRead, false);
  EXPECT_EQ(testStoredMessages.messageList[0].messageContent, message);

  // Unread messages incremented for recipient
  EXPECT_EQ(conversationsDictionary.conversations[username2][username1], 1);

  // Setting a message as read
  testStoredMessages.setRead(0, 0, username2);

  EXPECT_EQ(testStoredMessages.messageList[0].isRead, true);
  EXPECT_EQ(conversationsDictionary.conversations[username1][username2], 0);
}

TEST(MessagesDictionaryDict, GettingMessages) {
  char username1[g_UsernameLimit] = "carolyn";
  char username2[g_UsernameLimit] = "victor";
  char message[g_MessageLimit] = "Hello, Victor!";

  StoredMessages testStoredMessages;

  // Getting less than g_MessageQueryLimit messages on initial query
  for (int i = 0; i < 10; i++) {
    testStoredMessages.addMessage(username1, username2, message);
  }
  
  GetStoredMessagesReturnValue test1 = testStoredMessages.getStoredMessages(username2, -1);

  EXPECT_EQ(test1.messageList.size(), 10);
  EXPECT_EQ(test1.firstMessageIndex, 0);
  EXPECT_EQ(test1.lastMessageIndex, 9);
  
  // Unread messages incremented for recipient
  EXPECT_EQ(conversationsDictionary.conversations[username2][username1], 10);

  // Getting g_MessageQueryLimit messages on initial query
  for (int i = 0; i < g_MessageQueryLimit; i++) {
    testStoredMessages.addMessage(username1, username2, message);
  }
  
  GetStoredMessagesReturnValue test2 = testStoredMessages.getStoredMessages(username2, -1);

  EXPECT_EQ(test2.messageList.size(), g_MessageQueryLimit);
  EXPECT_EQ(test2.firstMessageIndex, 10);
  EXPECT_EQ(test2.lastMessageIndex, 10+g_MessageQueryLimit-1);

  // Unread messages incremented for recipient
  EXPECT_EQ(conversationsDictionary.conversations[username2][username1], 30);

  // Getting messages on second query
  GetStoredMessagesReturnValue test3 = testStoredMessages.getStoredMessages(username2, 10);

  EXPECT_EQ(test3.messageList.size(), 10);
  EXPECT_EQ(test3.firstMessageIndex, 0);
  EXPECT_EQ(test3.lastMessageIndex, 9);

}



int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}