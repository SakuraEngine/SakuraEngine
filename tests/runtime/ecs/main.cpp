#include "guid.hpp" //for guid
#include "SkrRT/platform/crash.h"
#include "SkrRT/ecs/dual.h"
#include "SkrRT/misc/make_zeroed.hpp"
#include "SkrRT/misc/log.h"

#include "SkrTestFramework/framework.hpp"

#include <memory>
#include <algorithm>

using TestComp = int;
dual_type_index_t type_test;
dual_type_index_t type_test_arr;
dual_type_index_t type_test2;
dual_type_index_t type_test3;
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

void register_test_component();
void register_ref_component();
void register_managed_component();
void register_pinned_component();

static struct ProcInitializer
{
    ProcInitializer()
    {
        ::skr_log_set_level(SKR_LOG_LEVEL_WARN);
        ::skr_initialize_crash_handler();
        ::skr_log_initialize_async_worker();

        ::register_test_component();
        ::register_ref_component();
        ::register_managed_component();
        ::register_pinned_component();
    }
    ~ProcInitializer()
    {
        ::dual_shutdown();

        ::skr_log_finalize_async_worker();
        ::skr_finalize_crash_handler();
    }
} init;

class ECSTest
{
public:
    ECSTest() SKR_NOEXCEPT
    {
        storage = dualS_create();
        {
            dual_chunk_view_t view;
            dual_entity_type_t entityType;
            entityType.type = { &type_test, 1 };
            entityType.meta = { nullptr, 0 };
            auto callback = [&](dual_chunk_view_t* inView) {
                view = *inView;
                *(TestComp*)dualV_get_owned_rw(&view, type_test) = 123;
            };
            dualS_allocate_type(storage, &entityType, 1, DUAL_LAMBDA(callback));

            e1 = dualV_get_entities(&view)[0];
        }
    }

    ~ECSTest() SKR_NOEXCEPT
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

TEST_CASE_METHOD(ECSTest, "allocate_one")
{
    REQUIRE(dualS_exist(storage, e1));
}

TEST_CASE_METHOD(ECSTest, "allocate_100000")
{
    dual_chunk_view_t view;
    dual_entity_type_t entityType;
    entityType.type = { &type_test, 1 };
    entityType.meta = { nullptr, 0 };
    auto callback = [&](dual_chunk_view_t* inView) {
        view = *inView;
        auto t = (TestComp*)dualV_get_owned_rw(&view, type_test);
        std::fill(t, t + inView->count, 123);
    };
    dualS_allocate_type(storage, &entityType, 100000, DUAL_LAMBDA(callback));

    auto e2 = dualV_get_entities(&view)[0];
    (void)e2;
    auto data = (const TestComp*)dualV_get_owned_ro(&view, type_test);
    EXPECT_EQ(data[0], 123);
    EXPECT_EQ(data[10], 123);
}

TEST_CASE_METHOD(ECSTest, "instantiate_single")
{
    dual_chunk_view_t view;
    auto callback = [&](dual_chunk_view_t* inView) { view = *inView; };
    dualS_instantiate(storage, e1, 10, DUAL_LAMBDA(callback));

    auto data = (const TestComp*)dualV_get_owned_ro(&view, type_test);
    EXPECT_EQ(data[0], 123);
    EXPECT_EQ(data[5], 123);
    EXPECT_EQ(data[9], 123);
}

TEST_CASE_METHOD(ECSTest, "instantiate_group")
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
        REQUIRE(std::equal(ents, ents + 10, refs));
    }
}

TEST_CASE_METHOD(ECSTest, "destroy_entity")
{
    REQUIRE(dualS_exist(storage, e1));
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
        REQUIRE(dualS_exist(storage, e2));
        EXPECT_FALSE(dualS_exist(storage, e1));
    }
}

TEST_CASE_METHOD(ECSTest, "add_component")
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

TEST_CASE_METHOD(ECSTest, "remove_component")
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

TEST_CASE_METHOD(ECSTest, "pin")
{
    dual_entity_t e2;
    dual_chunk_view_t view;
    {
        dual_entity_type_t entityType;
        dual_type_index_t types[] = { type_test, type_pinned };
        entityType.type = { types, 2 };
        entityType.meta = { nullptr, 0 };
        auto callback = [&](dual_chunk_view_t* inView) {
            view = *inView;
            *(TestComp*)dualV_get_owned_rw(&view, type_test) = 123;
        };
        dualS_allocate_type(storage, &entityType, 1, DUAL_LAMBDA(callback));

        e2 = dualV_get_entities(&view)[0];
    }
    
    dualS_destroy(storage, &view);
    {
        // entity with pinned component will only be marked as dead and keep existing
        REQUIRE(dualS_exist(storage, e2));
        dual_chunk_view_t view2;
        dualS_access(storage, e2, &view2);
        EXPECT_EQ(*(TestComp*)dualV_get_owned_ro(&view2, type_test), 123);
        //only explicit query can access dead entity
        {
            auto query = dualQ_from_literal(storage, "[in]pinned, [has]dead");
            dual_chunk_view_t view3;
            auto callback = [&](dual_chunk_view_t* inView) {
                view3 = *inView;
            };
            dualQ_get_views(query, DUAL_LAMBDA(callback));
            EXPECT_EQ(*dualV_get_entities(&view3), e2);
        }
        {
            auto query2 = dualQ_from_literal(storage, "[in]pinned");
            dual_chunk_view_t view4 = { 0 };
            auto callback = [&](dual_chunk_view_t* inView) {
                view4 = *inView;
            };
            dualQ_get_views(query2, DUAL_LAMBDA(callback));
            EXPECT_EQ(view4.chunk, nullptr);
        }
        //when all pinned components are removed from a dead entity, entity will be destroyed
        dual_delta_type_t deltaType;
        zero(deltaType);
        deltaType.removed = { { &type_pinned, 1 } };
        dualS_cast_view_delta(storage, &view2, &deltaType, nullptr, nullptr);
        EXPECT_FALSE(dualS_exist(storage, e2));
    }
}

TEST_CASE_METHOD(ECSTest, "manage")
{
    std::weak_ptr<int> observer;
    dual_entity_t e2;
    {
        dual_chunk_view_t view;
        dual_entity_type_t entityType;
        entityType.type = { &type_managed, 1 };
        entityType.meta = { nullptr, 0 };
        auto callback = [&](dual_chunk_view_t* inView) {
            view = *inView;
            auto& d = *(std::shared_ptr<int>*)dualV_get_owned_rw(&view, type_managed);
            d.reset(new int(1));
            observer = d;
        };
        dualS_allocate_type(storage, &entityType, 1, DUAL_LAMBDA(callback));
        e2 = dualV_get_entities(&view)[0];

        EXPECT_EQ(observer.use_count(), 1);
        EXPECT_EQ(*observer.lock(), 1);
        dualS_instantiate(storage, e2, 3, nullptr, nullptr);
        EXPECT_EQ(observer.use_count(), 4);

        dualS_destroy(storage, &view);
        EXPECT_EQ(observer.use_count(), 3);
    }
}

TEST_CASE_METHOD(ECSTest, "ref")
{
    dual_entity_t e2, e3, e4;
    {
        dual_chunk_view_t view;
        dual_entity_type_t entityType;
        entityType.type = { &type_ref, 1 };
        entityType.meta = { nullptr, 0 };
        auto callback = [&](dual_chunk_view_t* inView) {
            view = *inView;
            auto& d = *(dual_entity_t*)dualV_get_owned_rw(&view, type_ref);
            d = e1;
        };
        dualS_allocate_type(storage, &entityType, 1, DUAL_LAMBDA(callback));
        e2 = dualV_get_entities(&view)[0];
        auto callback2 = [&](dual_chunk_view_t* inView) {
            view = *inView;
            auto d = (dual_entity_t*)dualV_get_owned_rw(&view, type_ref);
            if(d)
            {
                e4 = *d;
            }
            else 
            {
                e3 = dualV_get_entities(&view)[0];
            }
        };
        dual_entity_t ents[] = { e1, e2 };
        dualS_instantiate_entities(storage, ents, 2, 1, DUAL_LAMBDA(callback2));
        auto totalCount = dualS_count(storage, false, false);
        EXPECT_EQ(totalCount, 4);
        EXPECT_EQ(e3, e4);
    }
}

TEST_CASE_METHOD(ECSTest, "batch")
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

TEST_CASE_METHOD(ECSTest, "filter")
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

TEST_CASE_METHOD(ECSTest, "query")
{
    auto query = dualQ_from_literal(storage, "[in]test");

    dual_chunk_view_t view;
    auto callback = [&](dual_chunk_view_t* inView) {
        view = *inView;
    };
    dualQ_get_views(query, DUAL_LAMBDA(callback));
    EXPECT_EQ(*dualV_get_entities(&view), e1);
}

TEST_CASE_METHOD(ECSTest, "query_overload")
{
    [[maybe_unused]] dual_entity_t e2, e3;
    {
        dual_chunk_view_t view;
        dual_entity_type_t entityType;
        dual_type_index_t type[2] = { type_test, type_test2 };
        std::sort(type, type + 2);
        entityType.type = { type, 2 };
        entityType.meta = { nullptr, 0 };
        auto callback = [&](dual_chunk_view_t* inView) {
            view = *inView;
            *(TestComp*)dualV_get_owned_rw(&view, type_test) = 123;
        };
        dualS_allocate_type(storage, &entityType, 1, DUAL_LAMBDA(callback));

        e2 = dualV_get_entities(&view)[0];
    }
    {
        dual_chunk_view_t view;
        dual_entity_type_t entityType;
        dual_type_index_t type[3] = { type_test, type_test2, type_test3 };
        std::sort(type, type + 3);
        entityType.type = { type, 3 };
        entityType.meta = { nullptr, 0 };
        auto callback = [&](dual_chunk_view_t* inView) {
            view = *inView;
            *(TestComp*)dualV_get_owned_rw(&view, type_test) = 123;
        };
        dualS_allocate_type(storage, &entityType, 1, DUAL_LAMBDA(callback));

        e3 = dualV_get_entities(&view)[0];
    }
    dualQ_make_alias(storage, "test", "test:a");
    auto query1 = dualQ_from_literal(storage, "[inout]test:a");
    auto query2 = dualQ_from_literal(storage, "[inout]test:a,[in]test2");
    SKR_UNUSED auto query3 = dualQ_from_literal(storage, "[inout]test:a,[in]test3");

    {
        dual_chunk_view_t view;
        auto callback = [&](dual_chunk_view_t* inView) {
            EXPECT_EQ(inView->count, 1);
            view = *inView;
        };
        dualQ_get_views(query1, DUAL_LAMBDA(callback));
        EXPECT_EQ(*dualV_get_entities(&view), e1);
    }
    {
        dual_chunk_view_t view;
        auto callback = [&](dual_chunk_view_t* inView) {
            EXPECT_EQ(inView->count, 1);
            view = *inView;
        };
        dualQ_get_views(query2, DUAL_LAMBDA(callback));
        EXPECT_EQ(*dualV_get_entities(&view), e2);
    }
}

void register_test_component()
{
    using namespace guid_parse::literals;
    {
        dual_type_description_t desc = make_zeroed<dual_type_description_t>();
        desc.name = u8"test";
        desc.size = sizeof(TestComp);
        desc.entityFieldsCount = 0;
        desc.entityFields = 0;
        desc.guid = u8"{3A44728E-66C2-40F9-A3C1-0920A727A94A}"_guid;
        desc.callback = {};
        desc.flags = 0;
        desc.elementSize = 0;
        desc.alignment = alignof(TestComp);
        type_test = dualT_register_type(&desc);
        desc.elementSize = desc.size;
        desc.size = desc.size * 10;
        desc.guid = u8"{82D170AE-6D06-406C-A451-F670103D7894}"_guid;
        desc.name = u8"test_arr";
        type_test_arr = dualT_register_type(&desc);
    }

    {

        dual_type_description_t desc = make_zeroed<dual_type_description_t>();
        desc.name = u8"test2";
        desc.size = sizeof(TestComp);
        desc.entityFieldsCount = 0;
        desc.entityFields = 0;
        desc.guid = u8"{36B139D5-0492-4EC7-AF72-B440665F2307}"_guid;
        desc.callback = {};
        desc.flags = 0;
        desc.elementSize = 0;
        desc.alignment = alignof(TestComp);
        type_test2 = dualT_register_type(&desc);
        desc.guid = u8"{FD70F4BD-52FB-4911-8930-41A80C482DBF}"_guid;
        desc.elementSize = desc.size;
        desc.size = desc.size * 10;
        desc.name = u8"test_arr";
        type_test2_arr = dualT_register_type(&desc);
    }

    {

        dual_type_description_t desc = make_zeroed<dual_type_description_t>();
        desc.name = u8"test3";
        desc.size = sizeof(TestComp);
        desc.entityFieldsCount = 0;
        desc.entityFields = 0;
        desc.guid = u8"{3E5A2C5F-7FBE-486C-BCCE-8E6D8933B2C5}"_guid;
        desc.callback = {};
        desc.flags = 0;
        desc.elementSize = 0;
        desc.alignment = alignof(TestComp);
        type_test3 = dualT_register_type(&desc);
    }
}

void register_ref_component()
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
    desc.guid = u8"{F84BB5DE-DB21-4CEF-8DF1-8CD7AEEE766F}"_guid;
    desc.elementSize = desc.size;
    desc.size = desc.size * 10;
    desc.name = u8"ref_arr";
    type_ref_arr = dualT_register_type(&desc);
}

void register_managed_component()
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
        +[](dual_chunk_t* chunk, EIndex index, char* dst, dual_chunk_t* schunk, EIndex sindex, const char* src) { new (dst) managed(*(managed*)src); },
        +[](dual_chunk_t* chunk, EIndex index, char* data) { ((managed*)data)->~managed(); },
        +[](dual_chunk_t* chunk, EIndex index, char* dst, dual_chunk_t* schunk, EIndex sindex, char* src) { new (dst) managed(std::move(*(managed*)src)); },
        +[](dual_chunk_t* chunk, EIndex index, char* data, EIndex count, skr_binary_writer_t* v) {
        },
        +[](dual_chunk_t* chunk, EIndex index, char* data, EIndex count, skr_binary_reader_t* v)
        {
        },
        nullptr
    };
    type_managed = dualT_register_type(&desc);
    desc.guid = u8"{BCF22A9B-908C-4F84-89B4-C7BD803FC6C2}"_guid;
    desc.elementSize = desc.size;
    desc.size = desc.size * 10;
    desc.name = u8"managed_arr";
    type_managed_arr = dualT_register_type(&desc);
}

void register_pinned_component()
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
    desc.guid = u8"{673605E0-C52C-40F5-A3F0-736409617D15}"_guid;
    desc.elementSize = desc.size;
    desc.size = desc.size * 10;
    desc.name = u8"pinned_arr";
    type_pinned_arr = dualT_register_type(&desc);
}