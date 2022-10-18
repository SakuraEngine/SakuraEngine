#include "gtest/gtest.h"
#include "utils/log.hpp"
#include "platform/guid.hpp"
#include "../types/types.hpp"
#if !defined(__meta__) && defined(__cplusplus)
#include "rtti-test-types/typeid.generated.hpp"
#endif

class RTTI : public ::testing::Test
{
protected:
    void SetUp() override
    {
        PrintField("importModule\n");
    }

    void TearDown() override
    {
    }
};

TEST_F(RTTI, TypeId)
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

#if !defined(__meta__) && defined(__cplusplus)
#include "rtti-test-types/types.rtti.generated.hpp"
#endif

TEST_F(RTTI, TestEnumType)
{
    auto registry = skr::type::GetTypeRegistry();
    //auto enumType = skr::type::EnumType::FromName("Types::TestEnum");
    auto type = registry->get_type(skr::type::type_id<Types::TestEnum>::get());
    auto enumType = static_cast<const skr::type::EnumType*>(type);
    EXPECT_TRUE(enumType != nullptr);
    EXPECT_EQ(enumType->guid, skr::type::type_id<Types::TestEnum>::get());
    for (auto&& enumerator : enumType->enumerators)
    {
        SKR_LOG_FMT_DEBUG("enumerator: {} -> {}", enumerator.name, enumerator.value);
        auto fieldName = eastl::string("Value").append(eastl::to_string(enumerator.value));
        EXPECT_EQ(enumerator.name, eastl::string_view(fieldName));

        auto str = enumType->ToString(&enumerator.value);
        EXPECT_EQ(enumerator.name, eastl::string_view(str));

        Types::TestEnum value = Types::TestEnum::Value0;
        enumType->FromString(&value, enumerator.name);
        EXPECT_EQ(enumerator.value, value);
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    return result;
}