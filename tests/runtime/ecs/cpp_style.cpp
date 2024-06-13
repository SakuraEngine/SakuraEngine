#include "SkrGuid/guid.hpp"
#include "SkrRT/ecs/sugoi.h"
#include "SkrRT/ecs/array.hpp"
#include "SkrRT/ecs/type_registry.hpp"
#include "SkrTestFramework/framework.hpp"

void _EXPECT_OK(sugoi::TypeRegisterResult&& r)
{
    using namespace skr::archive;
    r.and_then([](auto type_index) {
        EXPECT_EQ(true, true);
    })
    .error_then([](auto error) {
        EXPECT_EQ(true, false);
    });
}

void _EXPECT_ERROR(sugoi::TypeRegisterResult&& r, sugoi::TypeRegisterError err_code)
{
    using namespace skr::archive;
    r.and_then([](auto type_index) {
        EXPECT_EQ(true, false);
    })
    .error_then([=](auto error) {
        EXPECT_EQ(error, err_code);
    });
}

// for better ide display
#define EXPECT_OK _EXPECT_OK
#define EXPECT_ERROR _EXPECT_ERROR

struct ECSTest {
    ECSTest() SKR_NOEXCEPT
    {
        storage = sugoiS_create();
    }
    ~ECSTest() SKR_NOEXCEPT
    {
        ::sugoiS_release(storage);
    }
    sugoi_storage_t* storage;
};

TEST_CASE_METHOD(ECSTest, "RegisterType")
{
    using namespace skr::guid::literals;
    auto& type_registry = sugoi::TypeRegistry::get();

    SUBCASE("PlainComponent")
    {
        static constexpr auto kIntComponentGUID = u8"4a18a3b3-e6f9-47ec-aa88-994a1d260da8"_guid;
        EXPECT_ERROR(
            type_registry.new_type<int>()
                            .guid(kIntComponentGUID)
                            .size(0)
                            .commit()
            , sugoi::TypeRegisterError::ZeroSize);

        EXPECT_ERROR(
            type_registry.new_type<int>()
                            .commit()
            , sugoi::TypeRegisterError::InvalidGUID);

        EXPECT_ERROR(
            type_registry.new_type<int>()
                            .guid(kIntComponentGUID)
                            .commit()
            , sugoi::TypeRegisterError::InvalidName);

        EXPECT_OK(
            type_registry.new_type<int>()
                            .name(u8"int_component")
                            .guid(kIntComponentGUID)
                            .commit());

        EXPECT_ERROR(
            type_registry.new_type<int>()
                            .name(u8"int_component2")
                            .guid(kIntComponentGUID)
                            .commit()
            , sugoi::TypeRegisterError::GUIDAlreadyExists);

        static constexpr auto kAnotherGUID = u8"1d29de69-b2b0-44f6-a1aa-4acf070bf8ba"_guid;
        EXPECT_ERROR(
            type_registry.new_type<int>()
                            .name(u8"int_component")
                            .guid(kAnotherGUID)
                            .commit()
            , sugoi::TypeRegisterError::NameAlreadyExists);

        EXPECT_OK(
            type_registry.new_type<int>()
                            .name(u8"int_component2")
                            .guid(kAnotherGUID)
                            .commit());
        
        auto tid = type_registry.get_type(kIntComponentGUID);
        auto tinfo = *type_registry.get_type_desc(tid);
        EXPECT_EQ(tinfo.guid, kIntComponentGUID);
        EXPECT_EQ(tinfo.size, sizeof(int));
        EXPECT_EQ(tinfo.alignment, alignof(int));
    }

    SUBCASE("TagComponent")
    {
        static constexpr auto kTagGUID = u8"1d29de69-b2b0-44f6-a1aa-4acf070bf8bb"_guid;
        EXPECT_OK(
            type_registry.new_tag()
                            .name(u8"tag_component")
                            .guid(kTagGUID)
                            .commit());
        auto tid = type_registry.get_type(kTagGUID);
        auto tinfo = *type_registry.get_type_desc(tid);
        EXPECT_EQ(tinfo.guid, kTagGUID);
        EXPECT_EQ(tinfo.size, 0);
        EXPECT_EQ(tinfo.alignment, 0);
    }

    SUBCASE("ArrayComponent")
    {
        static constexpr auto kArrayGUID = u8"02472e6f-592f-4f10-a7b8-1d6c2bf86669"_guid;
        EXPECT_ERROR(
            type_registry.new_array<int, 5>()
                            .name(u8"array_component")
                            .guid(kArrayGUID)
                            .element_size(0)
                            .commit()
                , sugoi::TypeRegisterError::ZeroSize);
        EXPECT_ERROR(
            type_registry.new_array<int, 5>()
                            .name(u8"array_component")
                            .guid(kArrayGUID)
                            .element_count(0)
                            .commit()
                , sugoi::TypeRegisterError::ZeroArrayLength);
        EXPECT_OK(
            type_registry.new_array<int, 5>()
                            .name(u8"array_component")
                            .guid(kArrayGUID)
                            .commit());

        auto tid = type_registry.get_type(kArrayGUID);
        auto tinfo = *type_registry.get_type_desc(tid);
        EXPECT_EQ(tinfo.guid, kArrayGUID);
        EXPECT_EQ(tinfo.size, sizeof(sugoi::ArrayComponent<int, 5>));
        EXPECT_EQ(tinfo.alignment, alignof(sugoi::ArrayComponent<int, 5>));
    }

    SUBCASE("ArrayComponent(BigAligned)")
    {
        static constexpr auto kArrayGUID = u8"5eac5536-2faa-49d3-99c7-3d4963af0e2b"_guid;
        struct alignas(16) V
        {
            int v;
        };
        EXPECT_OK(
            type_registry.new_array<V, 5>()
                            .name(u8"array_component2")
                            .guid(kArrayGUID)
                            .commit());

        auto tid = type_registry.get_type(kArrayGUID);
        auto tinfo = *type_registry.get_type_desc(tid);
        EXPECT_EQ(tinfo.guid, kArrayGUID);
        EXPECT_EQ(tinfo.size, sizeof(sugoi::ArrayComponent<V, 5>));
        EXPECT_EQ(tinfo.alignment, alignof(sugoi::ArrayComponent<V, 5>));
    }
}