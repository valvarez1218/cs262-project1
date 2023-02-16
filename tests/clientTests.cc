#include <gtest/gtest.h>
#include "../client/takeInput.h"

std::vector<std::string> makeVectorHelper(std::string str) {
    try {
        return makeStringVector(str);
    } catch (std::invalid_argument &e) {
        return std::vector<std::string> {e.what()};
    }
}

TEST(TakeInput, ParsingUserInput) {
    std::string test1 = "send_message username \"Hello there\"";
    std::vector<std::string> expected1 {"send_message", "username", "Hello there"};
    EXPECT_EQ(makeStringVector(test1), expected1);

    std::string test2 = "one two three";
    std::vector<std::string> expected2 {"one", "two", "three"};
    EXPECT_EQ(makeStringVector(test2), expected2);

    std::string test3 = "one two \"three";
    std::vector<std::string> expected3 {"Format Error: Closing quotation mark expected."};
    EXPECT_EQ(makeVectorHelper(test3), expected3);

    std::string test4 = "one two \"three\" four";
    std::vector<std::string> expected4 {"Format Error: Should not have characters after the final quotation mark."};
    EXPECT_EQ(makeVectorHelper(test4), expected4);
}

