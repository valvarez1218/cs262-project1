#include <gtest/gtest.h>
#include "../server/chatServer.cc"
#include "../server/storage.h"

// Demonstrate some basic assertions.
TEST(MessagesDictionaryTest, BasicAssertions) {
  char username1[g_UsernameLimit] = {'Carolyn'};
  char username2[g_UsernameLimit] = {'Victor'};
  UserPair testpair1(username1, username2);

  // Expect smallerUsername to be Carolyn.
  EXPECT_EQ(testpair1.smallerUsername, username2);
  
  // Expect largerUsername to be Victor.
  EXPECT_EQ(testpair1.largerUsername, username2);

  UserPair testpair2(username2, username1);

  // Expect smallerUsername to be Carolyn.
  EXPECT_EQ(testpair2.smallerUsername, username1);
  
  // Expect largerUsername to be Carolyn.
  EXPECT_EQ(testpair2.largerUsername, username2);
}