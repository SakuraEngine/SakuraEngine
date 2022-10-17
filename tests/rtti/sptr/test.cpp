#include "test.hpp"

int main(int argc, char** argv)
 {
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    return result;
}