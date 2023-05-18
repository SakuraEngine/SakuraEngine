#include "gtest/gtest.h"
#include <memory>
#include "ecs/dual.h"
#include "guid.hpp" //for guid
#include "misc/make_zeroed.hpp"

using test = int;
dual_type_index_t type_test;
dual_type_index_t type_test_arr;
dual_type_index_t type_test2;
dual_type_index_t type_test2_arr;
using ref = dual_entity_t;
dual_type_index_t type_ref;
dual_type_index_t type_ref_arr;
using managed = std::shared_ptr<int>;
dual_type_index_t type_managed;
dual_type_index_t type_managed_arr;
using pinned = int*;
dual_type_index_t type_pinned;
dual_type_index_t type_pinned_arr;

class APITest : public ::testing::Test
{
public:
    void SetUp() override
    {
        storage = dualS_create();
        {
            dual_chunk_view_t view;
            dual_entity_type_t entityType;
            entityType.type = { &type_test, 1 };
            entityType.meta = { nullptr, 0 };
            auto callback = [&](dual_chunk_view_t* inView) {
                view = *inView;
                *(test*)dualV_get_owned_rw(&view, type_test) = 123;
            };
            dualS_allocate_type(storage, &entityType, 1, DUAL_LAMBDA(callback));

            e1 = dualV_get_entities(&view)[0];
        }
    }

    void TearDown() override
    {
        dualS_release(storage);
    }

    dual_storage_t* storage;
    dual_entity_t e1;
};

template <class T>
void zero(T& t)
{
    std::memset(&t, 0, sizeof(T));
}

TEST_F(APITest, allocate_one)
{
    EXPECT_TRUE(dualS_exist(storage, e1));
}

TEST_F(APITest, allocate_100000)
{
    dual_chunk_view_t view;
    dual_entity_type_t entityType;
    entityType.type = { &type_test, 1 };
    entityType.meta = { nullptr, 0 };
    auto callback = [&](dual_chunk_view_t* inView) {
        view = *inView;
        auto t = (test*)dualV_get_owned_rw(&view, type_test);
        std::fill(t, t + inView->count, 123);
    };
    dualS_allocate_type(storage, &entityType, 100000, DUAL_LAMBDA(callback));

    auto e2 = dualV_get_entities(&view)[0];
    (void)e2;
    auto data = (const test*)dualV_get_owned_ro(&view, type_test);
    EXPECT_EQ(data[0], 123);
    EXPECT_EQ(data[10], 123);
}

TEST_F(APITest, instantiate_single)
{
    dual_chunk_view_t view;
    auto callback = [&](dual_chunk_view_t* inView) { view = *inView; };
    dualS_instantiate(storage, e1, 10, DUAL_LAMBDA(callback));

    auto data = (const test*)dualV_get_owned_ro(&view, type_test);
    EXPECT_EQ(data[0], 123);
    EXPECT_EQ(data[5], 123);
    EXPECT_EQ(data[9], 123);
}

TEST_F(APITest, instantiate_group)
{
    dual_entity_t e2;
    {
        dual_chunk_view_t view;
        dual_entity_type_t entityType;
        entityType.type = { &type_ref, 1 };
        entityType.meta = { nullptr, 0 };
        auto callback = [&](dual_chunk_view_t* inView) { view = *inView; };
        dualS_allocate_type(storage, &entityType, 1, DUAL_LAMBDA(callback));
        *(ref*)dualV_get_owned_rw(&view, type_ref) = e1;
        e2 = dualV_get_entities(&view)[0];
    }
    {
        dual_chunk_view_t view[2];
        int n = 0;
        auto callback = [&](dual_chunk_view_t* inView) { view[n++] = *inView; };
        dual_entity_t group[] = { e1, e2 };
        dualS_instantiate_entities(storage, group, 2, 10, DUAL_LAMBDA(callback));
        auto ents = dualV_get_entities(&view[0]);
        auto refs = (const ref*)dualV_get_owned_ro(&view[1], type_ref);
        EXPECT_TRUE(std::equal(ents, ents + 10, refs));
    }
}

TEST_F(APITest, destroy_entity)
{
    EXPECT_TRUE(dualS_exist(storage, e1));
    dual_chunk_view_t view;
    dualS_access(storage, e1, &view);
    dualS_destroy(storage, &view);
    EXPECT_FALSE(dualS_exist(storage, e1));
    {
        dual_entity_type_t entityType;
        entityType.type = { &type_test, 1 };
        entityType.meta = { nullptr, 0 };
        auto callback = [&](dual_chunk_view_t* inView) { view = *inView; };
        dualS_allocate_type(storage, &entityType, 1, DUAL_LAMBDA(callback));
        auto e2 = dualV_get_entities(&view)[0];
        EXPECT_TRUE(dualS_exist(storage, e2));
        EXPECT_FALSE(dualS_exist(storage, e1));
    }
}

TEST_F(APITest, add_component)
{
    dual_chunk_view_t view;
    dualS_access(storage, e1, &view);
    EXPECT_EQ(dualV_get_owned_ro(&view, type_test2), nullptr);
    dual_delta_type_t deltaType;
    zero(deltaType);
    deltaType.added = { { &type_test2, 1 } };
    dualS_cast_view_delta(storage, &view, &deltaType, nullptr, nullptr);
    dualS_access(storage, e1, &view);
    EXPECT_NE(dualV_get_owned_ro(&view, type_test2), nullptr);
}

TEST_F(APITest, remove_component)
{
    dual_chunk_view_t view;
    dualS_access(storage, e1, &view);
    EXPECT_NE(dualV_get_owned_ro(&view, type_test), nullptr);
    dual_delta_type_t deltaType;
    zero(deltaType);
    deltaType.removed = { { &type_test, 1 } };
    dualS_cast_view_delta(storage, &view, &deltaType, nullptr, nullptr);
    dualS_access(storage, e1, &view);
    EXPECT_EQ(dualV_get_owned_ro(&view, type_test), nullptr);
}

TEST_F(APITest, batch)
{
    dual_chunk_view_t view[2];
    {
        dual_entity_type_t entityType;
        entityType.type = { &type_test, 1 };
        entityType.meta = { nullptr, 0 };
        auto callback = [&](dual_chunk_view_t* inView) {
            view[0] = *inView;
        };
        dualS_allocate_type(storage, &entityType, 10, DUAL_LAMBDA(callback));
    }
    {
        dual_entity_type_t entityType;
        entityType.type = { &type_test2, 1 };
        entityType.meta = { nullptr, 0 };
        auto callback = [&](dual_chunk_view_t* inView) {
            view[1] = *inView;
        };
        dualS_allocate_type(storage, &entityType, 10, DUAL_LAMBDA(callback));
    }
    std::vector<dual_entity_t> es;
    es.resize(20);
    std::memcpy(es.data(), dualV_get_entities(&view[0]), 10 * sizeof(dual_entity_t));
    std::memcpy(es.data() + 10, dualV_get_entities(&view[1]), 10 * sizeof(dual_entity_t));
    int i = 0;
    auto callback2 = [&](dual_chunk_view_t* inView) {
        EXPECT_EQ(view[i].chunk, inView->chunk);
        EXPECT_EQ(view[i].start, inView->start);
        EXPECT_EQ(view[i].count, inView->count);
        ++i;
    };
    dualS_batch(storage, es.data(), 20, DUAL_LAMBDA(callback2));
}

TEST_F(APITest, filter)
{
    dual_filter_t filter;
    zero(filter);
    dual_meta_filter_t meta;
    zero(meta);
    dual_chunk_view_t view;
    auto callback = [&](dual_chunk_view_t* inView) {
        view = *inView;
    };
    dualS_query(storage, &filter, &meta, DUAL_LAMBDA(callback));
    EXPECT_EQ(*dualV_get_entities(&view), e1);
}

TEST_F(APITest, query)
{
    auto query = dualQ_from_literal(storage, "[in]test");

    dual_chunk_view_t view;
    auto callback = [&](dual_chunk_view_t* inView) {
        view = *inView;
    };
    dualQ_get_views(query, DUAL_LAMBDA(callback));
    EXPECT_EQ(*dualV_get_entities(&view), e1);
}

void register_test_component()
{
    using namespace guid_parse::literals;
    {
        dual_type_description_t desc = make_zeroed<dual_type_description_t>();
        desc.name = u8"test";
        desc.size = sizeof(test);
        desc.entityFieldsCount = 0;
        desc.entityFields = 0;
        desc.guid = u8"{3A44728E-66C2-40F9-A3C1-0920A727A94A}"_guid;
        desc.callback = {};
        desc.flags = 0;
        desc.elementSize = 0;
        desc.alignment = alignof(test);
        type_test = dualT_register_type(&desc);
        desc.elementSize = desc.size;
        desc.size = desc.size * 10;
        desc.name = u8"test_arr";
        type_test_arr = dualT_register_type(&desc);
    }

    {

        dual_type_description_t desc = make_zeroed<dual_type_description_t>();
        desc.name = u8"test2";
        desc.size = sizeof(test);
        desc.entityFieldsCount = 0;
        desc.entityFields = 0;
        desc.guid = u8"{36B139D5-0492-4EC7-AF72-B440665F2307}"_guid;
        desc.callback = {};
        desc.flags = 0;
        desc.elementSize = 0;
        desc.alignment = alignof(test);
        type_test2 = dualT_register_type(&desc);
        desc.elementSize = desc.size;
        desc.size = desc.size * 10;
        desc.name = u8"test_arr";
        type_test2_arr = dualT_register_type(&desc);
    }
}
auto register_ref_component()
{
    using namespace guid_parse::literals;
    dual_type_description_t desc = make_zeroed<dual_type_description_t>();
    desc.name = u8"ref";
    desc.size = sizeof(ref);
    desc.entityFieldsCount = 1;
    intptr_t fields[1] = { 0 };
    desc.entityFields = (intptr_t)fields;
    desc.guid = u8"{4BEC235F-63DF-4A49-8F5E-5431890F61DD}"_guid;
    desc.callback = {};
    desc.flags = 0;
    desc.elementSize = 0;
    desc.alignment = alignof(ref);
    type_ref = dualT_register_type(&desc);
    desc.elementSize = desc.size;
    desc.size = desc.size * 10;
    desc.name = u8"ref_arr";
    type_ref_arr = dualT_register_type(&desc);
}
auto register_managed_component()
{
    using namespace guid_parse::literals;
    dual_type_description_t desc = make_zeroed<dual_type_description_t>();
    desc.name = u8"managed";
    desc.size = sizeof(managed);
    desc.entityFieldsCount = 0;
    desc.entityFields = 0;
    desc.guid = u8"{BA2D8E6A-9841-474F-9A6D-3E43BF0A7A9C}"_guid;
    desc.flags = 0;
    desc.elementSize = 0;
    desc.alignment = alignof(managed);
    desc.callback = {
        +[](dual_chunk_t* chunk, EIndex index, char* data) { new (data) managed; },
        +[](dual_chunk_t* chunk, EIndex index, char* dst, dual_chunk_t* schunk, EIndex sindex, const char* src) { *(managed*)dst = *(const managed*)src; },
        +[](dual_chunk_t* chunk, EIndex index, char* data) { ((managed*)data)->~managed(); },
        +[](dual_chunk_t* chunk, EIndex index, char* dst, dual_chunk_t* schunk, EIndex sindex, char* src) { *(managed*)dst = std::move(*(managed*)src); },
        +[](dual_chunk_t* chunk, EIndex index, char* data, EIndex count, skr_binary_writer_t* v) {
        },
        +[](dual_chunk_t* chunk, EIndex index, char* data, EIndex count, skr_binary_reader_t* v)
        {
            new (data) managed;
        },
        nullptr
    };
    type_managed = dualT_register_type(&desc);
    desc.elementSize = desc.size;
    desc.size = desc.size * 10;
    desc.name = u8"managed_arr";
    type_managed_arr = dualT_register_type(&desc);
}
auto register_pinned_component()
{
    using namespace guid_parse::literals;
    dual_type_description_t desc = make_zeroed<dual_type_description_t>();
    desc.name = u8"pinned";
    desc.size = sizeof(pinned);
    desc.entityFieldsCount = 0;
    desc.entityFields = 0;
    desc.guid = u8"{E6127548-981D-46F5-BF33-8EC31CACACFD}"_guid;
    desc.callback = {};
    desc.flags = DTF_PIN;
    desc.elementSize = 0;
    desc.alignment = alignof(pinned);
    type_pinned = dualT_register_type(&desc);
    desc.elementSize = desc.size;
    desc.size = desc.size * 10;
    desc.name = u8"pinned_arr";
    type_pinned_arr = dualT_register_type(&desc);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    register_test_component();
    register_ref_component();
    register_managed_component();
    register_pinned_component();
    auto result = RUN_ALL_TESTS();
    dual_shutdown();
    return result;
}