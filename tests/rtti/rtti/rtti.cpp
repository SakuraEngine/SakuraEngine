#include "gtest/gtest.h"
#include "misc/log.hpp"
#include "platform/guid.hpp"
#include "../types/types.hpp"

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

    auto guid0 = u8"1a0b91c7-6690-41d6-acfd-0c2b61f181f3"_guid;
    auto guid1 = skr::type::type_id<Types::TestEnum>::get();

    static_assert(skr::type::type_id<Types::TestEnum>::get() == u8"1a0b91c7-6690-41d6-acfd-0c2b61f181f3"_guid, "");
    const bool equal = guid0 == guid1;
    EXPECT_TRUE(equal);
    SKR_LOG_FMT_DEBUG(u8"u8 type id: {}", skr::type::type_id<uint32_t>::get());
    SKR_LOG_FMT_DEBUG(u8"TestEnum type id: {}", skr::type::type_id<Types::TestEnum>::get());
}

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
        SKR_LOG_FMT_DEBUG(u8"enumerator: {} -> {}", enumerator.name, enumerator.value);
        auto fieldName = skr::format(u8"Value{}", enumerator.value);
        EXPECT_EQ(enumerator.name, skr::string_view(fieldName.u8_str()));

        auto str = enumType->ToString(&enumerator.value);
        EXPECT_EQ(enumerator.name, skr::string_view(str.u8_str()));

        Types::TestEnum value = Types::TestEnum::Value0;
        enumType->FromString(&value, enumerator.name);
        EXPECT_EQ(enumerator.value, value);
    }
}

TEST_F(RTTI, TestRecordType)
{
    auto recordType = static_cast<const skr::type::RecordType*>(skr::type::type_of<Types::TestSon>::get());
    EXPECT_TRUE(recordType != nullptr);
    EXPECT_EQ(recordType->guid, skr::type::type_id<Types::TestSon>::get());
    EXPECT_EQ(recordType->base, skr::type::type_of<Types::TestParent>::get());
    EXPECT_EQ(recordType->size, sizeof(Types::TestSon));
    EXPECT_EQ(recordType->align, alignof(Types::TestSon));
    EXPECT_EQ(recordType->fields.size(), 17);
    for(auto&& field : recordType->fields)
    {
        SKR_LOG_FMT_DEBUG(u8"field {}: {} -> offset {}, size {}, align {}", field.name, field.type->Name(), field.offset, field.type->Size(), field.type->Align());
    }
}

TEST_F(RTTI, TestConvert)
{

}

TEST_F(RTTI, TestSerialize)
{

}

TEST_F(RTTI, TestTextSerialize)
{

}

TEST_F(RTTI, Construct)
{

}

TEST_F(RTTI, CopyConstruct)
{

}

TEST_F(RTTI, MoveConstruct)
{

}


TEST_F(RTTI, DynamicRecord)
{
    
}


int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    return result;
}