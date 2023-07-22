#include "SkrTestFramework/framework.hpp"

class BaseTests
{

};

TEST_CASE_METHOD(BaseTests, "Empty")
{
    EXPECT_EQ(nullptr, nullptr);
}