#include "cpp_style.hpp"
#include "SkrCore/log.h"
#include "SkrRuntime/ecs/world.hpp"
#include <chrono>

class MaskComponentTest 
{
public:
    skr::ecs::ECSWorld world;
    sugoi_storage_t* storage = nullptr;
    
    MaskComponentTest() SKR_NOEXCEPT
    {
        world.initialize();
        storage = world.get_storage();
    }
    
    ~MaskComponentTest() SKR_NOEXCEPT
    {
        world.finalize();
    }
    
    sugoi_entity_t create_test_entity()
    {
        struct Spawner
        {
            void build(skr::ecs::ArchetypeBuilder& builder)
            {
                builder.add_component<IntComponent>()
                    .add_component<FloatComponent>()
                    .add_component<sugoi::mask_comp_t>();
            }
            void run(skr::ecs::TaskContext& ctx)
            {
                auto entities = ctx.entities();
                result = (uint32_t)entities[0];
                
                // Initialize component data using readwrite access
                auto compInt = ctx.components<IntComponent>();
                auto compFloat = ctx.components<FloatComponent>();
                auto masks = ctx.components<sugoi::mask_comp_t>();

                compInt[0].v = 42;
                compFloat[0].v = 3.14f;
                
                // Initialize mask to enable all components
                masks[0].value = 0xFFFFFFFF; // All components enabled initially
            }
            uint32_t result = SUGOI_NULL_ENTITY;
        } spawner;
        world.create_entities(spawner, 1);
        return spawner.result;
    }
    
    std::vector<sugoi_entity_t> create_multiple_entities(uint32_t count)
    {
        struct Spawner
        {
            void build(skr::ecs::ArchetypeBuilder& builder)
            {
                builder.add_component<IntComponent>()
                    .add_component<FloatComponent>()
                    .add_component<sugoi::mask_comp_t>();
            }
            void run(skr::ecs::TaskContext& ctx)
            {
                auto entity_list = ctx.entities();
                for (uint32_t i = 0; i < ctx.size(); ++i) {
                    entities.push_back((sugoi_entity_t)entity_list[i]);
                }
                
                // Initialize component data using readwrite access
                auto compInt = ctx.components<IntComponent>();
                auto compFloat = ctx.components<FloatComponent>();
                auto masks = ctx.components<sugoi::mask_comp_t>();

                
                for (uint32_t i = 0; i < ctx.size(); ++i) {
                    compInt[i].v = static_cast<int>(i);
                    compFloat[i].v = static_cast<float>(i * 2);
                    masks[i].value = 0xFFFFFFFF; // All enabled
                }
            }
            std::vector<sugoi_entity_t> entities;
        } spawner;
        world.create_entities(spawner, count);
        return spawner.entities;
    }
};

TEST_CASE_METHOD(MaskComponentTest, "BasicMaskDisabling")
{
    auto entity = create_test_entity();
    
    // Create type sets for individual components
    sugoi_type_index_t typeInt = sugoi_id_of<IntComponent>::get();
    sugoi_type_index_t typeFloat = sugoi_id_of<FloatComponent>::get();
    sugoi_type_set_t typeSetInt = {&typeInt, 1};
    sugoi_type_set_t typeSetFloat = {&typeFloat, 1};
    
    // Initially all components should be enabled
    EXPECT_TRUE(sugoiS_components_enabled(storage, entity, &typeSetInt));
    EXPECT_TRUE(sugoiS_components_enabled(storage, entity, &typeSetFloat));
    
    // Access entity to disable IntComponent
    sugoi_chunk_view_t view;
    sugoiS_access(storage, entity, &view);
    
    // Disable IntComponent
    sugoiS_disable_components(&view, &typeSetInt);
    
    // Check that IntComponent is now disabled but FloatComponent is still enabled
    EXPECT_FALSE(sugoiS_components_enabled(storage, entity, &typeSetInt));
    EXPECT_TRUE(sugoiS_components_enabled(storage, entity, &typeSetFloat));
    
    // Re-enable IntComponent
    sugoiS_enable_components(&view, &typeSetInt);
    
    EXPECT_TRUE(sugoiS_components_enabled(storage, entity, &typeSetInt));
    EXPECT_TRUE(sugoiS_components_enabled(storage, entity, &typeSetFloat));
}

TEST_CASE_METHOD(MaskComponentTest, "QueryWithMaskFiltering")
{
    // Create multiple entities with components and mask
    auto entities = create_multiple_entities(10);
    
    // Disable IntComponent in first 5 entities
    sugoi_type_index_t typeInt = sugoi_id_of<IntComponent>::get();
    sugoi_type_set_t typeSetInt = {&typeInt, 1};
    
    for (int i = 0; i < 5; ++i) {
        sugoi_chunk_view_t view;
        sugoiS_access(storage, entities[i], &view);
        sugoiS_disable_components(&view, &typeSetInt);
    }
    
    // Query for entities with IntComponent enabled
    auto query = sugoiQ_from_literal(storage, u8"[in]IntComponent");
    
    size_t count = 0;
    sugoiQ_get_views(query, [](void* u, sugoi_chunk_view_t* view) {
        auto* counter = static_cast<size_t*>(u);
        *counter += view->count;
    }, &count);
    
    // Should find 5 entities (IntComponent enabled)
    EXPECT_EQ(count, 5);
}

TEST_CASE_METHOD(MaskComponentTest, "QueryStressTest")
{
    const uint32_t NUM_ENTITIES = 100000;
    const uint32_t NUM_QUERIES = 1000;
    
    SKR_LOG_INFO(u8"Starting Query Stress Test with %u entities and %u queries", NUM_ENTITIES, NUM_QUERIES);
    
    // Create a large number of entities for stress testing
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Create entities in batches for better performance
    std::vector<sugoi_entity_t> entities;
    entities.reserve(NUM_ENTITIES);
    
    const uint32_t BATCH_SIZE = 10000;
    for (uint32_t batch = 0; batch < NUM_ENTITIES; batch += BATCH_SIZE) {
        uint32_t batch_count = std::min(BATCH_SIZE, NUM_ENTITIES - batch);
        auto batch_entities = create_multiple_entities(batch_count);
        entities.insert(entities.end(), batch_entities.begin(), batch_entities.end());
    }
    
    auto creation_time = std::chrono::high_resolution_clock::now();
    auto creation_duration = std::chrono::duration_cast<std::chrono::milliseconds>(creation_time - start_time);
    SKR_LOG_INFO(u8"Created %u entities in %lld ms", NUM_ENTITIES, creation_duration.count());
    
    // Disable components in a pattern for variety
    sugoi_type_index_t typeInt = sugoi_id_of<IntComponent>::get();
    sugoi_type_index_t typeFloat = sugoi_id_of<FloatComponent>::get();
    sugoi_type_set_t typeSetInt = {&typeInt, 1};
    sugoi_type_set_t typeSetFloat = {&typeFloat, 1};
    
    // Disable IntComponent in every 3rd entity, FloatComponent in every 5th entity
    for (uint32_t i = 0; i < entities.size(); ++i) {
        sugoi_chunk_view_t view;
        sugoiS_access(storage, entities[i], &view);
        
        if (i % 5 == 0) {
            sugoiS_disable_components(&view, &typeSetFloat);
        }
        
        if (i % 3 == 0) {
            sugoiS_disable_components(&view, &typeSetInt);
        }
    }
    
    auto disable_time = std::chrono::high_resolution_clock::now();
    auto disable_duration = std::chrono::duration_cast<std::chrono::milliseconds>(disable_time - creation_time);
    SKR_LOG_INFO(u8"Applied mask patterns in %lld ms", disable_duration.count());
    
    // Create different types of queries
    auto queryInt = sugoiQ_from_literal(storage, u8"[in]IntComponent");
    auto queryFloat = sugoiQ_from_literal(storage, u8"[in]FloatComponent");
    auto queryBoth = sugoiQ_from_literal(storage, u8"[in]IntComponent, [in]FloatComponent");
    
    // Perform stress test with multiple queries
    auto query_start = std::chrono::high_resolution_clock::now();
    
    size_t totalIntCount = 0, totalFloatCount = 0, totalBothCount = 0;
    
    for (uint32_t i = 0; i < NUM_QUERIES; ++i) {
        // Query for IntComponent entities
        size_t intCount = 0;
        sugoiQ_get_views(queryInt, [](void* u, sugoi_chunk_view_t* view) {
            auto* counter = static_cast<size_t*>(u);
            *counter += view->count;
        }, &intCount);
        totalIntCount += intCount;
        
        // Query for FloatComponent entities
        size_t floatCount = 0;
        sugoiQ_get_views(queryFloat, [](void* u, sugoi_chunk_view_t* view) {
            auto* counter = static_cast<size_t*>(u);
            *counter += view->count;
        }, &floatCount);
        totalFloatCount += floatCount;
        
        // Query for entities with both components
        size_t bothCount = 0;
        sugoiQ_get_views(queryBoth, [](void* u, sugoi_chunk_view_t* view) {
            auto* counter = static_cast<size_t*>(u);
            *counter += view->count;
        }, &bothCount);
        totalBothCount += bothCount;
    }
    
    auto query_end = std::chrono::high_resolution_clock::now();
    auto query_duration = std::chrono::duration_cast<std::chrono::milliseconds>(query_end - query_start);
    
    SKR_LOG_INFO(u8"Performed %u queries in %lld ms (%.2f queries/ms)", 
                 NUM_QUERIES * 3, query_duration.count(), 
                 (NUM_QUERIES * 3.0) / query_duration.count());
    
    // Verify query results make sense
    // IntComponent should be disabled in 1/3 of entities, so ~2/3 should be found
    // FloatComponent should be disabled in 1/5 of entities, so ~4/5 should be found
    size_t expectedIntPerQuery = (NUM_ENTITIES * 2) / 3;  // Approximate
    size_t expectedFloatPerQuery = (NUM_ENTITIES * 4) / 5;  // Approximate
    
    size_t avgIntCount = totalIntCount / NUM_QUERIES;
    size_t avgFloatCount = totalFloatCount / NUM_QUERIES;
    size_t avgBothCount = totalBothCount / NUM_QUERIES;
    
    SKR_LOG_INFO(u8"Average counts - Int: %zu (expected ~%zu), Float: %zu (expected ~%zu), Both: %zu", 
                 avgIntCount, expectedIntPerQuery, avgFloatCount, expectedFloatPerQuery, avgBothCount);
    
    // Verify the counts are reasonable (within 20% of expected)
    EXPECT_TRUE(avgFloatCount > expectedFloatPerQuery * 8 / 10);
    EXPECT_TRUE(avgFloatCount < expectedFloatPerQuery * 12 / 10);
    EXPECT_TRUE(avgIntCount > expectedIntPerQuery * 8 / 10);
    EXPECT_TRUE(avgIntCount < expectedIntPerQuery * 12 / 10);
    
    // Both count should be less than either individual count
    EXPECT_TRUE(avgBothCount < avgIntCount);
    EXPECT_TRUE(avgBothCount < avgFloatCount);
    
    auto total_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(total_time - start_time);
    SKR_LOG_INFO(u8"Query stress test completed in %lld ms total", total_duration.count());
}