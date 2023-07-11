#include "gtest/gtest.h"
#include "misc/log.hpp"
#include "SkrRT/platform/crash.h"
#include "SkrRT/platform/guid.hpp"
#include "containers/sptr.hpp"
#include "serde/json/writer.h"
#include "../types/types.hpp"

class RTTI : public ::testing::Test
{
protected:
    void SetUp() override
    {
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

#ifdef _WIN32
TEST_F(RTTI, TestConvert)
{
    uint32_t a = 0;
    skr_value_ref_t a_ref{a};
    a = 10;
    EXPECT_TRUE(a_ref.HasValue());
    EXPECT_TRUE(a_ref.Is<uint32_t>());
    EXPECT_TRUE(a_ref.Convertible<float>());
    EXPECT_FLOAT_EQ(a_ref.Convert<float>(), 10.0f);
    EXPECT_TRUE(!a_ref.Convertible<skr::SPtr<uint32_t>>());
    skr::vector<uint32_t> vec;
    vec.push_back(1);
    skr_value_ref_t vec_ref{vec};
    EXPECT_TRUE(vec_ref.HasValue());
    EXPECT_TRUE(vec_ref.Is<skr::vector<uint32_t>>());
    EXPECT_TRUE(vec_ref.Convertible<skr::span<uint32_t>>());
    EXPECT_EQ(vec_ref.Convert<skr::span<uint32_t>>()[0], 1);
    uint32_t arr[3] = {1, 2, 3};
    skr_value_ref_t arr_ref{arr, skr::type::type_of<uint32_t[3]>::get()};
    EXPECT_TRUE(arr_ref.HasValue());
    vec = arr_ref.Convert<skr::vector<uint32_t>>();
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
    EXPECT_EQ(vec[2], 3);
}

TEST_F(RTTI, TestTextSerialize)
{
    auto uint32Type = skr::type::type_of<uint32_t>::get();
    auto dynarrType = (skr::type::DynArrayType*)skr::type::make_dynarray_type(uint32Type);
    auto dynarr = dynarrType->Malloc();
    dynarrType->Construct(dynarr, nullptr, 0);
    dynarrType->Reset(dynarr, 2);
    *(uint32_t*)dynarrType->Get(dynarr, 0) = 1;
    *(uint32_t*)dynarrType->Get(dynarr, 1) = 2;
    *(uint32_t*)dynarrType->Insert(dynarr, 1) = 3;
    skr_json_writer_t writer(2, skr_json_format_t{false});
    dynarrType->SerializeText(dynarr, &writer);
    auto string = writer.Str();
    SKR_LOG_FMT_DEBUG(u8"dynarr: {}", string);
    EXPECT_EQ(string, skr::string(u8"[1,3,2]"));
    skr::vector<uint32_t>& vec = *(skr::vector<uint32_t>*)dynarr;
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 3);
    EXPECT_EQ(vec[2], 2);
    dynarrType->Destruct(dynarr);
}
#endif

TEST_F(RTTI, DynamicRecord)
{
    
}

int main(int argc, char** argv)
{
    skr_initialize_crash_handler();
    skr_log_initialize_async_worker();

    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();

    skr_log_finalize_async_worker();
    skr_finalize_crash_handler();
    return result;
}