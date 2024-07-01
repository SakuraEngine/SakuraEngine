#include "cpp_style.hpp"
#include "SkrContainers/vector.hpp"

struct RegisterTypes {
    RegisterTypes() SKR_NOEXCEPT
        : storage(sugoiS_create()),
          type_registry(sugoi::TypeRegistry::get())
    {

    }
    ~RegisterTypes() SKR_NOEXCEPT
    {
        ::sugoiS_release(storage);
    }
    sugoi_storage_t* storage;
    sugoi::TypeRegistry& type_registry;
};

using namespace skr::literals;

TEST_CASE_METHOD(RegisterTypes, "PlainComponent")
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

TEST_CASE_METHOD(RegisterTypes, "TagComponent")
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

TEST_CASE_METHOD(RegisterTypes, "ArrayComponent")
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
    {
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
    {
        static constexpr auto kArrayGUID2 = u8"cb65fd41-2fda-46c6-9477-0d6d9b6b1466"_guid;
        EXPECT_OK(
            type_registry.new_array<int, 5>()
                            .name(u8"array_component(custom_size)")
                            .guid(kArrayGUID2)
                            .element_count(15)
                            .commit());

        auto tid = type_registry.get_type(kArrayGUID2);
        auto tinfo = *type_registry.get_type_desc(tid);
        EXPECT_EQ(tinfo.guid, kArrayGUID2);
        EXPECT_EQ(tinfo.size, sizeof(sugoi::ArrayComponent<int, 15>));
        EXPECT_EQ(tinfo.alignment, alignof(sugoi::ArrayComponent<int, 15>));
    }
}

TEST_CASE_METHOD(RegisterTypes, "ArrayComponent(BigAligned)")
{
    struct alignas(16) V
    {
        int v;
    };
    {
        static constexpr auto kArrayGUID = u8"5eac5536-2faa-49d3-99c7-3d4963af0e2b"_guid;
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
    {
        static constexpr auto kArrayGUID2 = u8"e6bc84c1-be53-43ed-9bce-0f4128274338"_guid;
        EXPECT_OK(
            type_registry.new_array<V, 5>()
                            .name(u8"array_component2(custom_size)")
                            .guid(kArrayGUID2)
                            .element_count(15)
                            .commit());
        auto tid = type_registry.get_type(kArrayGUID2);
        auto tinfo = *type_registry.get_type_desc(tid);
        EXPECT_EQ(tinfo.guid, kArrayGUID2);
        EXPECT_EQ(tinfo.size, sizeof(sugoi::ArrayComponent<V, 15>));
        EXPECT_EQ(tinfo.alignment, alignof(sugoi::ArrayComponent<V, 15>));
    }
}