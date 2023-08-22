#include "SkrRT/platform/crash.h"
#include "SkrRT/platform/guid.hpp"
#include "SkrRT/misc/log.hpp"
#include "SkrRT/containers/sptr.hpp"
#include "SkrRT/serde/json/writer.h"
#include "../types/types.hpp"

#include "SkrTestFramework/framework.hpp"

static struct ProcInitializer
{
    ProcInitializer()
    {
        ::skr_log_set_level(SKR_LOG_LEVEL_WARN);
        ::skr_initialize_crash_handler();
        ::skr_log_initialize_async_worker();
    }
    ~ProcInitializer()
    {
        ::skr_log_finalize_async_worker();
        ::skr_finalize_crash_handler();
    }
} init;

struct RTTITests
{

};

TEST_CASE_METHOD(RTTITests, "TypeId")
{
    using namespace skr::guid::literals;

    auto guid0 = u8"1a0b91c7-6690-41d6-acfd-0c2b61f181f3"_guid;
    auto guid1 = skr::type::type_id<Types::TestEnum>::get();

    static_assert(skr::type::type_id<Types::TestEnum>::get() == u8"1a0b91c7-6690-41d6-acfd-0c2b61f181f3"_guid, "");
    const bool equal = guid0 == guid1;
    REQUIRE(equal);
    SKR_LOG_FMT_DEBUG(u8"u8 type id: {}", skr::type::type_id<uint32_t>::get());
    SKR_LOG_FMT_DEBUG(u8"TestEnum type id: {}", skr::type::type_id<Types::TestEnum>::get());
}

TEST_CASE_METHOD(RTTITests, "TestEnumType")
{
    auto registry = skr::type::GetTypeRegistry();
    //auto enumType = skr::type::EnumType::FromName("Types::TestEnum");
    auto type = registry->get_type(skr::type::type_id<Types::TestEnum>::get());
    auto enumType = static_cast<const skr::type::EnumType*>(type);
    REQUIRE(enumType != nullptr);
    EXPECT_EQ(enumType->guid, skr::type::type_id<Types::TestEnum>::get());
    for (auto&& enumerator : enumType->GetEnumerators())
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

TEST_CASE_METHOD(RTTITests, "TestRecordType")
{
    auto recordType = static_cast<const skr::type::RecordType*>(skr::type::type_of<Types::TestSon>::get());
    REQUIRE(recordType != nullptr);
    EXPECT_EQ(recordType->GetGuid(), skr::type::type_id<Types::TestSon>::get());
    EXPECT_EQ(recordType->GetBaseType(), skr::type::type_of<Types::TestParent>::get());
    EXPECT_EQ(recordType->GetSize(), sizeof(Types::TestSon));
    EXPECT_EQ(recordType->GetAlign(), alignof(Types::TestSon));
    EXPECT_EQ(recordType->GetFields().size(), 17);
    for(auto&& field : recordType->GetFields())
    {
        SKR_LOG_FMT_DEBUG(u8"field {}: {} -> offset {}, size {}, align {}", field.name, field.type->Name(), field.offset, field.type->Size(), field.type->Align());
    }
}

template<class T>
typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
    almost_equal(T x, T y, int ulp = 4)
{
    // the machine epsilon has to be scaled to the magnitude of the values used
    // and multiplied by the desired precision in ULPs (units in the last place)
    return std::fabs(x - y) <= std::numeric_limits<T>::epsilon() * std::fabs(x + y) * ulp
        // unless the result is subnormal
        || std::fabs(x - y) < std::numeric_limits<T>::min();
}

TEST_CASE_METHOD(RTTITests, "TestConvert")
{
    uint32_t a = 0;
    skr_value_ref_t a_ref{a};
    a = 10;
    REQUIRE(a_ref.HasValue());
    REQUIRE(a_ref.Is<uint32_t>());
    REQUIRE(a_ref.Convertible<float>());
    REQUIRE(::almost_equal(a_ref.Convert<float>(), 10.0f));
    REQUIRE(!a_ref.Convertible<skr::SPtr<uint32_t>>());
    skr::vector<uint32_t> vec;
    vec.push_back(1);
    skr_value_ref_t vec_ref{vec};
    REQUIRE(vec_ref.HasValue());
    REQUIRE(vec_ref.Is<skr::vector<uint32_t>>());
    REQUIRE(vec_ref.Convertible<skr::span<uint32_t>>());
    EXPECT_EQ(vec_ref.Convert<skr::span<uint32_t>>()[0], 1);
    uint32_t arr[3] = {1, 2, 3};
    skr_value_ref_t arr_ref{arr, skr::type::type_of<uint32_t[3]>::get()};
    REQUIRE(arr_ref.HasValue());
    vec = arr_ref.Convert<skr::vector<uint32_t>>();
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
    EXPECT_EQ(vec[2], 3);
}

TEST_CASE_METHOD(RTTITests, "TestTextSerialize")
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

TEST_CASE_METHOD(RTTITests, "DynamicRecord")
{
    
}