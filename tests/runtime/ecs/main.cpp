#include "guid.hpp" //for guid
#include "SkrRT/platform/crash.h"
#include "SkrRT/ecs/sugoi.h"
#include "SkrRT/misc/make_zeroed.hpp"
#include "SkrRT/misc/log.h"

#include "SkrTestFramework/framework.hpp"

#include <memory>
#include <algorithm>

using TestComp = int;
sugoi_type_index_t type_test;
sugoi_type_index_t type_test_arr;
sugoi_type_index_t type_test2;
sugoi_type_index_t type_test3;
sugoi_type_index_t type_test2_arr;
using ref = sugoi_entity_t;
sugoi_type_index_t type_ref;
sugoi_type_index_t type_ref_arr;
using managed = std::shared_ptr<int>;
sugoi_type_index_t type_managed;
sugoi_type_index_t type_managed_arr;
using pinned = int*;
sugoi_type_index_t type_pinned;
sugoi_type_index_t type_pinned_arr;

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
        ::sugoi_shutdown();

        ::skr_log_finalize_async_worker();
        ::skr_finalize_crash_handler();
    }
} init;

class ECSTest
{
public:
    ECSTest() SKR_NOEXCEPT
    {
        storage = sugoiS_create();
        {
            sugoi_chunk_view_t view;
            sugoi_entity_type_t entityType;
            entityType.type = { &type_test, 1 };
            entityType.meta = { nullptr, 0 };
            auto callback = [&](sugoi_chunk_view_t* inView) {
                view = *inView;
                *(TestComp*)sugoiV_get_owned_rw(&view, type_test) = 123;
            };
            sugoiS_allocate_type(storage, &entityType, 1, SUGOI_LAMBDA(callback));

            e1 = sugoiV_get_entities(&view)[0];
        }
    }

    ~ECSTest() SKR_NOEXCEPT
    {
        sugoiS_release(storage);
    }

    sugoi_storage_t* storage;
    sugoi_entity_t e1;
};

template <class T>
void zero(T& t)
{
    std::memset(&t, 0, sizeof(T));
}

TEST_CASE_METHOD(ECSTest, "allocate_one")
{
    REQUIRE(sugoiS_exist(storage, e1));
}

TEST_CASE_METHOD(ECSTest, "allocate_100000")
{
    sugoi_chunk_view_t view;
    sugoi_entity_type_t entityType;
    entityType.type = { &type_test, 1 };
    entityType.meta = { nullptr, 0 };
    auto callback = [&](sugoi_chunk_view_t* inView) {
        view = *inView;
        auto t = (TestComp*)sugoiV_get_owned_rw(&view, type_test);
        std::fill(t, t + inView->count, 123);
    };
    sugoiS_allocate_type(storage, &entityType, 100000, SUGOI_LAMBDA(callback));

    auto e2 = sugoiV_get_entities(&view)[0];
    (void)e2;
    auto data = (const TestComp*)sugoiV_get_owned_ro(&view, type_test);
    EXPECT_EQ(data[0], 123);
    EXPECT_EQ(data[10], 123);
}

TEST_CASE_METHOD(ECSTest, "instantiate_single")
{
    sugoi_chunk_view_t view;
    auto callback = [&](sugoi_chunk_view_t* inView) { view = *inView; };
    sugoiS_instantiate(storage, e1, 10, SUGOI_LAMBDA(callback));

    auto data = (const TestComp*)sugoiV_get_owned_ro(&view, type_test);
    EXPECT_EQ(data[0], 123);
    EXPECT_EQ(data[5], 123);
    EXPECT_EQ(data[9], 123);
}

TEST_CASE_METHOD(ECSTest, "instantiate_group")
{
    sugoi_entity_t e2;
    {
        sugoi_chunk_view_t view;
        sugoi_entity_type_t entityType;
        entityType.type = { &type_ref, 1 };
        entityType.meta = { nullptr, 0 };
        auto callback = [&](sugoi_chunk_view_t* inView) { view = *inView; };
        sugoiS_allocate_type(storage, &entityType, 1, SUGOI_LAMBDA(callback));
        *(ref*)sugoiV_get_owned_rw(&view, type_ref) = e1;
        e2 = sugoiV_get_entities(&view)[0];
    }
    {
        sugoi_chunk_view_t view[2];
        int n = 0;
        auto callback = [&](sugoi_chunk_view_t* inView) { view[n++] = *inView; };
        sugoi_entity_t group[] = { e1, e2 };
        sugoiS_instantiate_entities(storage, group, 2, 10, SUGOI_LAMBDA(callback));
        auto ents = sugoiV_get_entities(&view[0]);
        auto refs = (const ref*)sugoiV_get_owned_ro(&view[1], type_ref);
        REQUIRE(std::equal(ents, ents + 10, refs));
    }
}

TEST_CASE_METHOD(ECSTest, "destroy_entity")
{
    REQUIRE(sugoiS_exist(storage, e1));
    sugoi_chunk_view_t view;
    sugoiS_access(storage, e1, &view);
    sugoiS_destroy(storage, &view);
    EXPECT_FALSE(sugoiS_exist(storage, e1));
    {
        sugoi_entity_type_t entityType;
        entityType.type = { &type_test, 1 };
        entityType.meta = { nullptr, 0 };
        auto callback = [&](sugoi_chunk_view_t* inView) { view = *inView; };
        sugoiS_allocate_type(storage, &entityType, 1, SUGOI_LAMBDA(callback));
        auto e2 = sugoiV_get_entities(&view)[0];
        REQUIRE(sugoiS_exist(storage, e2));
        EXPECT_FALSE(sugoiS_exist(storage, e1));
    }
}

TEST_CASE_METHOD(ECSTest, "add_component")
{
    sugoi_chunk_view_t view;
    sugoiS_access(storage, e1, &view);
    EXPECT_EQ(sugoiV_get_owned_ro(&view, type_test2), nullptr);
    sugoi_delta_type_t deltaType;
    zero(deltaType);
    deltaType.added = { { &type_test2, 1 } };
    sugoiS_cast_view_delta(storage, &view, &deltaType, nullptr, nullptr);
    sugoiS_access(storage, e1, &view);
    EXPECT_NE(sugoiV_get_owned_ro(&view, type_test2), nullptr);
}

TEST_CASE_METHOD(ECSTest, "remove_component")
{
    sugoi_chunk_view_t view;
    sugoiS_access(storage, e1, &view);
    EXPECT_NE(sugoiV_get_owned_ro(&view, type_test), nullptr);
    sugoi_delta_type_t deltaType;
    zero(deltaType);
    deltaType.removed = { { &type_test, 1 } };
    sugoiS_cast_view_delta(storage, &view, &deltaType, nullptr, nullptr);
    sugoiS_access(storage, e1, &view);
    EXPECT_EQ(sugoiV_get_owned_ro(&view, type_test), nullptr);
}

TEST_CASE_METHOD(ECSTest, "pin")
{
    sugoi_entity_t e2;
    sugoi_chunk_view_t view;
    {
        sugoi_entity_type_t entityType;
        sugoi_type_index_t types[] = { type_test, type_pinned };
        entityType.type = { types, 2 };
        entityType.meta = { nullptr, 0 };
        auto callback = [&](sugoi_chunk_view_t* inView) {
            view = *inView;
            *(TestComp*)sugoiV_get_owned_rw(&view, type_test) = 123;
        };
        sugoiS_allocate_type(storage, &entityType, 1, SUGOI_LAMBDA(callback));

        e2 = sugoiV_get_entities(&view)[0];
    }
    
    sugoiS_destroy(storage, &view);
    {
        // entity with pinned component will only be marked as dead and keep existing
        REQUIRE(sugoiS_exist(storage, e2));
        sugoi_chunk_view_t view2;
        sugoiS_access(storage, e2, &view2);
        EXPECT_EQ(*(TestComp*)sugoiV_get_owned_ro(&view2, type_test), 123);
        //only explicit query can access dead entity
        {
            auto query = sugoiQ_from_literal(storage, u8"[in]pinned, [has]dead");
            sugoi_chunk_view_t view3;
            auto callback = [&](sugoi_chunk_view_t* inView) {
                view3 = *inView;
            };
            sugoiQ_get_views(query, SUGOI_LAMBDA(callback));
            EXPECT_EQ(*sugoiV_get_entities(&view3), e2);
        }
        {
            auto query2 = sugoiQ_from_literal(storage, u8"[in]pinned");
            sugoi_chunk_view_t view4 = { 0 };
            auto callback = [&](sugoi_chunk_view_t* inView) {
                view4 = *inView;
            };
            sugoiQ_get_views(query2, SUGOI_LAMBDA(callback));
            EXPECT_EQ(view4.chunk, nullptr);
        }
        //when all pinned components are removed from a dead entity, entity will be destroyed
        sugoi_delta_type_t deltaType;
        zero(deltaType);
        deltaType.removed = { { &type_pinned, 1 } };
        sugoiS_cast_view_delta(storage, &view2, &deltaType, nullptr, nullptr);
        EXPECT_FALSE(sugoiS_exist(storage, e2));
    }
}

TEST_CASE_METHOD(ECSTest, "manage")
{
    std::weak_ptr<int> observer;
    sugoi_entity_t e2;
    {
        sugoi_chunk_view_t view;
        sugoi_entity_type_t entityType;
        entityType.type = { &type_managed, 1 };
        entityType.meta = { nullptr, 0 };
        auto callback = [&](sugoi_chunk_view_t* inView) {
            view = *inView;
            auto& d = *(std::shared_ptr<int>*)sugoiV_get_owned_rw(&view, type_managed);
            d.reset(new int(1));
            observer = d;
        };
        sugoiS_allocate_type(storage, &entityType, 1, SUGOI_LAMBDA(callback));
        e2 = sugoiV_get_entities(&view)[0];

        EXPECT_EQ(observer.use_count(), 1);
        EXPECT_EQ(*observer.lock(), 1);
        sugoiS_instantiate(storage, e2, 3, nullptr, nullptr);
        EXPECT_EQ(observer.use_count(), 4);

        sugoiS_destroy(storage, &view);
        EXPECT_EQ(observer.use_count(), 3);
    }
}

TEST_CASE_METHOD(ECSTest, "ref")
{
    sugoi_entity_t e2, e3, e4;
    {
        sugoi_chunk_view_t view;
        sugoi_entity_type_t entityType;
        entityType.type = { &type_ref, 1 };
        entityType.meta = { nullptr, 0 };
        auto callback = [&](sugoi_chunk_view_t* inView) {
            view = *inView;
            auto& d = *(sugoi_entity_t*)sugoiV_get_owned_rw(&view, type_ref);
            d = e1;
        };
        sugoiS_allocate_type(storage, &entityType, 1, SUGOI_LAMBDA(callback));
        e2 = sugoiV_get_entities(&view)[0];
        auto callback2 = [&](sugoi_chunk_view_t* inView) {
            view = *inView;
            auto d = (sugoi_entity_t*)sugoiV_get_owned_rw(&view, type_ref);
            if(d)
            {
                e4 = *d;
            }
            else 
            {
                e3 = sugoiV_get_entities(&view)[0];
            }
        };
        sugoi_entity_t ents[] = { e1, e2 };
        sugoiS_instantiate_entities(storage, ents, 2, 1, SUGOI_LAMBDA(callback2));
        auto totalCount = sugoiS_count(storage, false, false);
        EXPECT_EQ(totalCount, 4);
        EXPECT_EQ(e3, e4);
    }
}

TEST_CASE_METHOD(ECSTest, "batch")
{
    sugoi_chunk_view_t view[2];
    {
        sugoi_entity_type_t entityType;
        entityType.type = { &type_test, 1 };
        entityType.meta = { nullptr, 0 };
        auto callback = [&](sugoi_chunk_view_t* inView) {
            view[0] = *inView;
        };
        sugoiS_allocate_type(storage, &entityType, 10, SUGOI_LAMBDA(callback));
    }
    {
        sugoi_entity_type_t entityType;
        entityType.type = { &type_test2, 1 };
        entityType.meta = { nullptr, 0 };
        auto callback = [&](sugoi_chunk_view_t* inView) {
            view[1] = *inView;
        };
        sugoiS_allocate_type(storage, &entityType, 10, SUGOI_LAMBDA(callback));
    }
    std::vector<sugoi_entity_t> es;
    es.resize(20);
    std::memcpy(es.data(), sugoiV_get_entities(&view[0]), 10 * sizeof(sugoi_entity_t));
    std::memcpy(es.data() + 10, sugoiV_get_entities(&view[1]), 10 * sizeof(sugoi_entity_t));
    int i = 0;
    auto callback2 = [&](sugoi_chunk_view_t* inView) {
        EXPECT_EQ(view[i].chunk, inView->chunk);
        EXPECT_EQ(view[i].start, inView->start);
        EXPECT_EQ(view[i].count, inView->count);
        ++i;
    };
    sugoiS_batch(storage, es.data(), 20, SUGOI_LAMBDA(callback2));
}

TEST_CASE_METHOD(ECSTest, "filter")
{
    sugoi_filter_t filter;
    zero(filter);
    sugoi_meta_filter_t meta;
    zero(meta);
    sugoi_chunk_view_t view;
    auto callback = [&](sugoi_chunk_view_t* inView) {
        view = *inView;
    };
    sugoiS_query(storage, &filter, &meta, SUGOI_LAMBDA(callback));
    EXPECT_EQ(*sugoiV_get_entities(&view), e1);
}

TEST_CASE_METHOD(ECSTest, "query")
{
    auto query = sugoiQ_from_literal(storage, u8"[in]test");

    sugoi_chunk_view_t view;
    auto callback = [&](sugoi_chunk_view_t* inView) {
        view = *inView;
    };
    sugoiQ_get_views(query, SUGOI_LAMBDA(callback));
    EXPECT_EQ(*sugoiV_get_entities(&view), e1);
}

TEST_CASE_METHOD(ECSTest, "query_overload")
{
    [[maybe_unused]] sugoi_entity_t e2, e3;
    {
        sugoi_chunk_view_t view;
        sugoi_entity_type_t entityType;
        sugoi_type_index_t type[2] = { type_test, type_test2 };
        std::sort(type, type + 2);
        entityType.type = { type, 2 };
        entityType.meta = { nullptr, 0 };
        auto callback = [&](sugoi_chunk_view_t* inView) {
            view = *inView;
            *(TestComp*)sugoiV_get_owned_rw(&view, type_test) = 123;
        };
        sugoiS_allocate_type(storage, &entityType, 1, SUGOI_LAMBDA(callback));

        e2 = sugoiV_get_entities(&view)[0];
    }
    {
        sugoi_chunk_view_t view;
        sugoi_entity_type_t entityType;
        sugoi_type_index_t type[3] = { type_test, type_test2, type_test3 };
        std::sort(type, type + 3);
        entityType.type = { type, 3 };
        entityType.meta = { nullptr, 0 };
        auto callback = [&](sugoi_chunk_view_t* inView) {
            view = *inView;
            *(TestComp*)sugoiV_get_owned_rw(&view, type_test) = 123;
        };
        sugoiS_allocate_type(storage, &entityType, 1, SUGOI_LAMBDA(callback));

        e3 = sugoiV_get_entities(&view)[0];
    }
    sugoiQ_make_alias(storage, u8"test", u8"test:a");
    auto query1 = sugoiQ_from_literal(storage, u8"[inout]test:a");
    auto query2 = sugoiQ_from_literal(storage, u8"[inout]test:a, [in]test2");
    SKR_UNUSED auto query3 = sugoiQ_from_literal(storage, u8"[inout]test:a, [in]test3");

    {
        sugoi_chunk_view_t view;
        auto callback = [&](sugoi_chunk_view_t* inView) {
            EXPECT_EQ(inView->count, 1);
            view = *inView;
        };
        sugoiQ_get_views(query1, SUGOI_LAMBDA(callback));
        EXPECT_EQ(*sugoiV_get_entities(&view), e1);
    }
    {
        sugoi_chunk_view_t view;
        auto callback = [&](sugoi_chunk_view_t* inView) {
            EXPECT_EQ(inView->count, 1);
            view = *inView;
        };
        sugoiQ_get_views(query2, SUGOI_LAMBDA(callback));
        EXPECT_EQ(*sugoiV_get_entities(&view), e2);
    }
}

void register_test_component()
{
    using namespace guid_parse::literals;
    {
        sugoi_type_description_t desc = make_zeroed<sugoi_type_description_t>();
        desc.name = u8"test";
        desc.size = sizeof(TestComp);
        desc.entityFieldsCount = 0;
        desc.entityFields = 0;
        desc.guid = u8"{3A44728E-66C2-40F9-A3C1-0920A727A94A}"_guid;
        desc.callback = {};
        desc.flags = 0;
        desc.elementSize = 0;
        desc.alignment = alignof(TestComp);
        type_test = sugoiT_register_type(&desc);
        desc.elementSize = desc.size;
        desc.size = desc.size * 10;
        desc.guid = u8"{82D170AE-6D06-406C-A451-F670103D7894}"_guid;
        desc.name = u8"test_arr";
        type_test_arr = sugoiT_register_type(&desc);
    }

    {

        sugoi_type_description_t desc = make_zeroed<sugoi_type_description_t>();
        desc.name = u8"test2";
        desc.size = sizeof(TestComp);
        desc.entityFieldsCount = 0;
        desc.entityFields = 0;
        desc.guid = u8"{36B139D5-0492-4EC7-AF72-B440665F2307}"_guid;
        desc.callback = {};
        desc.flags = 0;
        desc.elementSize = 0;
        desc.alignment = alignof(TestComp);
        type_test2 = sugoiT_register_type(&desc);
        desc.guid = u8"{FD70F4BD-52FB-4911-8930-41A80C482DBF}"_guid;
        desc.elementSize = desc.size;
        desc.size = desc.size * 10;
        desc.name = u8"test_arr";
        type_test2_arr = sugoiT_register_type(&desc);
    }

    {

        sugoi_type_description_t desc = make_zeroed<sugoi_type_description_t>();
        desc.name = u8"test3";
        desc.size = sizeof(TestComp);
        desc.entityFieldsCount = 0;
        desc.entityFields = 0;
        desc.guid = u8"{3E5A2C5F-7FBE-486C-BCCE-8E6D8933B2C5}"_guid;
        desc.callback = {};
        desc.flags = 0;
        desc.elementSize = 0;
        desc.alignment = alignof(TestComp);
        type_test3 = sugoiT_register_type(&desc);
    }
}

void register_ref_component()
{
    using namespace guid_parse::literals;
    sugoi_type_description_t desc = make_zeroed<sugoi_type_description_t>();
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
    type_ref = sugoiT_register_type(&desc);
    desc.guid = u8"{F84BB5DE-DB21-4CEF-8DF1-8CD7AEEE766F}"_guid;
    desc.elementSize = desc.size;
    desc.size = desc.size * 10;
    desc.name = u8"ref_arr";
    type_ref_arr = sugoiT_register_type(&desc);
}

void register_managed_component()
{
    using namespace guid_parse::literals;
    sugoi_type_description_t desc = make_zeroed<sugoi_type_description_t>();
    desc.name = u8"managed";
    desc.size = sizeof(managed);
    desc.entityFieldsCount = 0;
    desc.entityFields = 0;
    desc.guid = u8"{BA2D8E6A-9841-474F-9A6D-3E43BF0A7A9C}"_guid;
    desc.flags = 0;
    desc.elementSize = 0;
    desc.alignment = alignof(managed);
    desc.callback = {
        +[](sugoi_chunk_t* chunk, EIndex index, char* data) { new (data) managed; },
        +[](sugoi_chunk_t* chunk, EIndex index, char* dst, sugoi_chunk_t* schunk, EIndex sindex, const char* src) { new (dst) managed(*(managed*)src); },
        +[](sugoi_chunk_t* chunk, EIndex index, char* data) { ((managed*)data)->~managed(); },
        +[](sugoi_chunk_t* chunk, EIndex index, char* dst, sugoi_chunk_t* schunk, EIndex sindex, char* src) { new (dst) managed(std::move(*(managed*)src)); },
        +[](sugoi_chunk_t* chunk, EIndex index, char* data, EIndex count, skr_binary_writer_t* v) {
        },
        +[](sugoi_chunk_t* chunk, EIndex index, char* data, EIndex count, skr_binary_reader_t* v)
        {
        },
        nullptr
    };
    type_managed = sugoiT_register_type(&desc);
    desc.guid = u8"{BCF22A9B-908C-4F84-89B4-C7BD803FC6C2}"_guid;
    desc.elementSize = desc.size;
    desc.size = desc.size * 10;
    desc.name = u8"managed_arr";
    type_managed_arr = sugoiT_register_type(&desc);
}

void register_pinned_component()
{
    using namespace guid_parse::literals;
    sugoi_type_description_t desc = make_zeroed<sugoi_type_description_t>();
    desc.name = u8"pinned";
    desc.size = sizeof(pinned);
    desc.entityFieldsCount = 0;
    desc.entityFields = 0;
    desc.guid = u8"{E6127548-981D-46F5-BF33-8EC31CACACFD}"_guid;
    desc.callback = {};
    desc.flags = SUGOI_TYPE_FLAG_PIN;
    desc.elementSize = 0;
    desc.alignment = alignof(pinned);
    type_pinned = sugoiT_register_type(&desc);
    desc.guid = u8"{673605E0-C52C-40F5-A3F0-736409617D15}"_guid;
    desc.elementSize = desc.size;
    desc.size = desc.size * 10;
    desc.name = u8"pinned_arr";
    type_pinned_arr = sugoiT_register_type(&desc);
}