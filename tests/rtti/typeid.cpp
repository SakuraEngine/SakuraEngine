#include "gtest/gtest.h"
#include "utils/log.hpp"
#include "platform/guid.hpp"
#if !defined(__meta__) && defined(__cplusplus)
#include "rtti-test/typeid.generated.hpp"
#endif

class TypeId : public ::testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST(TypeId, Test0)
{
    using namespace skr::guid::literals;

    auto guid0 = "1a0b91c7-6690-41d6-acfd-0c2b61f181f3"_guid;
    auto guid1 = skr::type::type_id<Types::TestEnum>::get();

    static_assert(skr::type::type_id<Types::TestEnum>::get() == "1a0b91c7-6690-41d6-acfd-0c2b61f181f3"_guid, "");
    const bool equal = guid0 == guid1;
    EXPECT_TRUE(equal);
    SKR_LOG_FMT_DEBUG("u8 type id: {}", skr::type::type_id<uint32_t>::get());
    SKR_LOG_FMT_DEBUG("TestEnum type id: {}", skr::type::type_id<Types::TestEnum>::get());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    return result;
}