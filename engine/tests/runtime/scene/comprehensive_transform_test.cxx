#include "SkrBase/types.h"
#include "SkrBase/math.h"
#include "rtm/qvvf.h"
#include "SkrOS/thread.h"
#include "SkrCore/log.h"
#include "SkrContainers/vector.hpp"
#include "SkrRuntime/sugoi/storage.hpp"
#include "SkrRuntime/sugoi/job.hpp"
#include "SkrSceneCore/transform_system.h"

#include "SkrTestFramework/framework.hpp"

// Test configuration - complex tree structure
constexpr int ROOT_COUNT = 3;           // 3 root entities
constexpr int LEVEL_1_PER_ROOT = 256;   // 4 children per root
constexpr int LEVEL_2_PER_LEVEL1 = 8;   // 3 children per level-1 entity
constexpr int LEVEL_3_PER_LEVEL2 = 256; // 2 children per level-2 entity

// Define expected transforms for each level
static const auto rootTranslation1 = skr_float3_t{ 10.0f, 5.0f, 0.0f };
static const auto rootRotation1 = skr_rotator_f_t{ 0.0f, 30.0f, 0.0f };
static const auto rootScale1 = skr_float3_t{ 1.1f, 1.1f, 1.1f };
const skr::TransformF rootTransform1{
    skr::QuatF(rootRotation1),
    rootTranslation1,
    rootScale1
};

static const auto rootTranslation2 = skr_float3_t{ 20.0f, 10.0f, 0.0f };
static const auto rootRotation2 = skr_rotator_f_t{ 0.0f, 60.0f, 0.0f };
static const auto rootScale2 = skr_float3_t{ 1.2f, 1.2f, 1.2f };
const skr::TransformF rootTransform2{
    skr::QuatF(rootRotation2),
    rootTranslation2,
    rootScale2
};

static const auto rootTranslation3 = skr_float3_t{ 30.0f, 15.0f, 0.0f };
static const auto rootRotation3 = skr_rotator_f_t{ 0.0f, 90.0f, 0.0f };
static const auto rootScale3 = skr_float3_t{ 1.3f, 1.3f, 1.3f };
const skr::TransformF rootTransform3{
    skr::QuatF(rootRotation3),
    rootTranslation3,
    rootScale3
};

static const auto level1Translation = skr_float3_t{ 2.0f, 3.0f, 1.0f };
static const auto level1Rotation = skr_rotator_f_t{ 15.0f, 0.0f, 10.0f };
static const auto level1Scale = skr_float3_t{ 0.95f, 0.95f, 0.95f };
const skr::TransformF level1TransformLocal{
    skr::QuatF(level1Rotation),
    level1Translation,
    level1Scale
};

static const auto level2Translation = skr_float3_t{ 1.5f, 2.0f, 2.5f };
static const auto level2Rotation = skr_rotator_f_t{ 0.0f, 20.0f, 5.0f };
static const auto level2Scale = skr_float3_t{ 0.9f, 0.9f, 0.9f };
const skr::TransformF level2TransformLocal{
    skr::QuatF(level2Rotation),
    level2Translation,
    level2Scale
};

static const auto level3Translation = skr_float3_t{ 0.5f, 1.0f, 1.5f };
static const auto level3Rotation = skr_rotator_f_t{ 45.0f, 25.0f, 0.0f };
static const auto level3Scale = skr_float3_t{ 0.7f, 0.7f, 0.7f };
const skr::TransformF level3TransformLocal{
    skr::QuatF(level3Rotation),
    level3Translation,
    level3Scale
};

struct ComplexTransformTest
{
    struct SetupParams
    {
        int start_index;
        int count;
        const skr::Vector<sugoi_entity_t>* children_list;
    };
    SetupParams setupParams;

    ComplexTransformTest()
    {
        ::skr_log_set_level(SKR_LOG_LEVEL_WARN);
        ::skr_log_initialize_async_worker();
        storage = sugoiS_create();
        transform_system = skr::TransformSystem::Create(storage);

        spawnEntities();

        scheduler.initialize(skr::task::scheudler_config_t());
        scheduler.bind();
        sugoiJ_bind_storage(storage);

        // Calculate expected world transforms
        calculateExpectedTransforms();
    }

    ~ComplexTransformTest()
    {
        sugoiJ_unbind_storage(storage);
        skr::TransformSystem::Destroy(transform_system);
        sugoiS_release(storage);
        scheduler.unbind();
        ::skr_log_finalize_async_worker();
    }

    void spawnEntities()
    {
        SkrZoneScopedN("SpawnComplexHierarchy");

        // Create 3 root entities
        spawnRootEntities();

        // Create level 1 entities (children of roots)
        spawnLevel1Entities();

        // Create level 2 entities (grandchildren)
        spawnLevel2Entities();

        // Create level 3 entities (great-grandchildren)
        spawnLevel3Entities();

        // Setup parent-child relationships
        setupHierarchy();

        SKR_LOG_INFO(u8"Created complex hierarchy: %d roots, %d level1, %d level2, %d level3",
            ROOT_COUNT,
            ROOT_COUNT * LEVEL_1_PER_ROOT,
            ROOT_COUNT * LEVEL_1_PER_ROOT * LEVEL_2_PER_LEVEL1,
            ROOT_COUNT * LEVEL_1_PER_ROOT * LEVEL_2_PER_LEVEL1 * LEVEL_3_PER_LEVEL2);
    }

    void spawnRootEntities()
    {
        SkrZoneScopedN("SpawnRootEntities");

        sugoi::EntitySpawner<skr::scene::RootComponent, SKR_SCENE_COMPONENTS> root_spawner;
        root_spawner(storage, ROOT_COUNT, [&](auto& view) {
            auto translations = sugoi::get_owned<skr::scene::PositionComponent>(view.view);
            auto rotations = sugoi::get_owned<skr::scene::RotationComponent>(view.view);
            auto scales = sugoi::get_owned<skr::scene::ScaleComponent>(view.view);
            auto entities = sugoiV_get_entities(view.view);

            for (uint32_t i = 0; i < view.count(); ++i)
            {
                if (i == 0)
                {
                    translations[i].value = rootTranslation1;
                    rotations[i].euler = rootRotation1;
                    scales[i].value = rootScale1;
                }
                else if (i == 1)
                {
                    translations[i].value = rootTranslation2;
                    rotations[i].euler = rootRotation2;
                    scales[i].value = rootScale2;
                }
                else
                {
                    translations[i].value = rootTranslation3;
                    rotations[i].euler = rootRotation3;
                    scales[i].value = rootScale3;
                }

                root_entities.add(entities[i]);
                SKR_LOG_INFO(u8"Root entity %d: %llu", i, entities[i]);
            }
        });
    }

    void spawnLevel1Entities()
    {
        SkrZoneScopedN("SpawnLevel1Entities");

        const int total_level1 = ROOT_COUNT * LEVEL_1_PER_ROOT;
        sugoi::EntitySpawner<SKR_SCENE_COMPONENTS> level1_spawner;

        level1_spawner(storage, total_level1, [&](auto& view) {
            auto translations = sugoi::get_owned<skr::scene::PositionComponent>(view.view);
            auto rotations = sugoi::get_owned<skr::scene::RotationComponent>(view.view);
            auto scales = sugoi::get_owned<skr::scene::ScaleComponent>(view.view);
            auto parents = sugoi::get_owned<skr::scene::ParentComponent>(view.view);
            auto entities = sugoiV_get_entities(view.view);

            for (uint32_t i = 0; i < view.count(); ++i)
            {
                int root_index = i / LEVEL_1_PER_ROOT;

                translations[i].value = level1Translation;
                rotations[i].euler = level1Rotation;
                scales[i].value = level1Scale;
                parents[i].entity = root_entities[root_index];

                level1_entities.add(entities[i]);
            }
        });
    }

    void spawnLevel2Entities()
    {
        SkrZoneScopedN("SpawnLevel2Entities");

        const int total_level2 = ROOT_COUNT * LEVEL_1_PER_ROOT * LEVEL_2_PER_LEVEL1;
        sugoi::EntitySpawner<SKR_SCENE_COMPONENTS> level2_spawner;

        level2_spawner(storage, total_level2, [&](auto& view) {
            auto translations = sugoi::get_owned<skr::scene::PositionComponent>(view.view);
            auto rotations = sugoi::get_owned<skr::scene::RotationComponent>(view.view);
            auto scales = sugoi::get_owned<skr::scene::ScaleComponent>(view.view);
            auto parents = sugoi::get_owned<skr::scene::ParentComponent>(view.view);
            auto entities = sugoiV_get_entities(view.view);

            for (uint32_t i = 0; i < view.count(); ++i)
            {
                int level1_index = i / LEVEL_2_PER_LEVEL1;

                translations[i].value = level2Translation;
                rotations[i].euler = level2Rotation;
                scales[i].value = level2Scale;
                parents[i].entity = level1_entities[level1_index];

                level2_entities.add(entities[i]);
            }
        });
    }

    void spawnLevel3Entities()
    {
        SkrZoneScopedN("SpawnLevel3Entities");

        const int total_level3 = ROOT_COUNT * LEVEL_1_PER_ROOT * LEVEL_2_PER_LEVEL1 * LEVEL_3_PER_LEVEL2;
        sugoi::EntitySpawner<SKR_SCENE_COMPONENTS> level3_spawner;

        level3_spawner(storage, total_level3, [&](auto& view) {
            auto translations = sugoi::get_owned<skr::scene::PositionComponent>(view.view);
            auto rotations = sugoi::get_owned<skr::scene::RotationComponent>(view.view);
            auto scales = sugoi::get_owned<skr::scene::ScaleComponent>(view.view);
            auto parents = sugoi::get_owned<skr::scene::ParentComponent>(view.view);
            auto entities = sugoiV_get_entities(view.view);

            for (uint32_t i = 0; i < view.count(); ++i)
            {
                int level2_index = i / LEVEL_3_PER_LEVEL2;

                translations[i].value = level3Translation;
                rotations[i].euler = level3Rotation;
                scales[i].value = level3Scale;
                parents[i].entity = level2_entities[level2_index];

                level3_entities.add(entities[i]);
            }
        });
    }

    void setupHierarchy()
    {
        SkrZoneScopedN("SetupHierarchy");

        // Setup children for root entities
        setupChildrenForRoots();

        // Setup children for level 1 entities
        setupChildrenForLevel1();

        // Setup children for level 2 entities
        setupChildrenForLevel2();
    }

    void setupChildrenForRoots()
    {
        auto setupQuery = storage->new_query()
                              .ReadWriteAny<skr::scene::ChildrenComponent>()
                              .ReadAll<skr::scene::RootComponent>()
                              .commit()
                              .value();
        SKR_DEFER({ storage->destroy_query(setupQuery); });

        storage->query(setupQuery, +[](void* userdata, sugoi_chunk_view_t* view) -> void {
            auto _this = (ComplexTransformTest*)userdata;
            auto entities = sugoiV_get_entities(view);
            auto children_arrays = (skr::scene::ChildrenArray*)sugoi::get_owned<skr::scene::ChildrenComponent>(view);
            
            for (uint32_t i = 0; i < view->count; ++i) {
                // Find which root this is
                for (int j = 0; j < ROOT_COUNT; ++j) {
                    if (entities[i] == _this->root_entities[j]) {
                        // Add level 1 children for this root
                        for (int k = 0; k < LEVEL_1_PER_ROOT; ++k) {
                            int child_index = j * LEVEL_1_PER_ROOT + k;
                            if (child_index < _this->level1_entities.size()) {
                                children_arrays[i].emplace_back(skr::scene::ChildrenComponent{ _this->level1_entities[child_index] });
                            }
                        }
                        break;
                    }
                }
            } }, this);
    }

    void setupChildrenForLevel1()
    {
        auto setupQuery = storage->new_query()
                              .ReadWriteAny<skr::scene::ChildrenComponent>()
                              .ReadAll<skr::scene::ParentComponent>()
                              .commit()
                              .value();
        SKR_DEFER({ storage->destroy_query(setupQuery); });

        storage->query(setupQuery, +[](void* userdata, sugoi_chunk_view_t* view) -> void {
            auto _this = (ComplexTransformTest*)userdata;
            auto entities = sugoiV_get_entities(view);
            auto children_arrays = (skr::scene::ChildrenArray*)sugoi::get_owned<skr::scene::ChildrenComponent>(view);
            
            for (uint32_t i = 0; i < view->count; ++i) {
                // Find which level 1 entity this is
                for (int j = 0; j < _this->level1_entities.size(); ++j) {
                    if (entities[i] == _this->level1_entities[j]) {
                        // Add level 2 children for this level 1 entity
                        for (int k = 0; k < LEVEL_2_PER_LEVEL1; ++k) {
                            int child_index = j * LEVEL_2_PER_LEVEL1 + k;
                            if (child_index < _this->level2_entities.size()) {
                                children_arrays[i].emplace_back(skr::scene::ChildrenComponent{ _this->level2_entities[child_index] });
                            }
                        }
                        break;
                    }
                }
            } }, this);
    }

    void setupChildrenForLevel2()
    {
        auto setupQuery = storage->new_query()
                              .ReadWriteAny<skr::scene::ChildrenComponent>()
                              .ReadAll<skr::scene::ParentComponent>()
                              .commit()
                              .value();
        SKR_DEFER({ storage->destroy_query(setupQuery); });

        storage->query(setupQuery, +[](void* userdata, sugoi_chunk_view_t* view) -> void {
            auto _this = (ComplexTransformTest*)userdata;
            auto entities = sugoiV_get_entities(view);
            auto children_arrays = (skr::scene::ChildrenArray*)sugoi::get_owned<skr::scene::ChildrenComponent>(view);
            
            for (uint32_t i = 0; i < view->count; ++i) {
                // Find which level 2 entity this is
                for (int j = 0; j < _this->level2_entities.size(); ++j) {
                    if (entities[i] == _this->level2_entities[j]) {
                        // Add level 3 children for this level 2 entity
                        for (int k = 0; k < LEVEL_3_PER_LEVEL2; ++k) {
                            int child_index = j * LEVEL_3_PER_LEVEL2 + k;
                            if (child_index < _this->level3_entities.size()) {
                                children_arrays[i].emplace_back(skr::scene::ChildrenComponent{ _this->level3_entities[child_index] });
                            }
                        }
                        break;
                    }
                }
            } }, this);
    }

    void calculateExpectedTransforms()
    {
        SkrZoneScopedN("CalculateExpectedTransforms");

        // Root transforms are already set
        expectedRootTransforms.add(rootTransform1);
        expectedRootTransforms.add(rootTransform2);
        expectedRootTransforms.add(rootTransform3);

        // Calculate level 1 expected transforms
        for (int i = 0; i < ROOT_COUNT; ++i)
        {
            skr::TransformF expected = expectedRootTransforms[i] * level1TransformLocal;
            for (int j = 0; j < LEVEL_1_PER_ROOT; ++j)
            {
                expectedLevel1Transforms.add(expected);
            }
        }

        // Calculate level 2 expected transforms
        for (int i = 0; i < expectedLevel1Transforms.size(); ++i)
        {
            skr::TransformF expected = expectedLevel1Transforms[i] * level2TransformLocal;
            for (int j = 0; j < LEVEL_2_PER_LEVEL1; ++j)
            {
                expectedLevel2Transforms.add(expected);
            }
        }

        // Calculate level 3 expected transforms
        for (int i = 0; i < expectedLevel2Transforms.size(); ++i)
        {
            skr::TransformF expected = expectedLevel2Transforms[i] * level3TransformLocal;
            for (int j = 0; j < LEVEL_3_PER_LEVEL2; ++j)
            {
                expectedLevel3Transforms.add(expected);
            }
        }
    }

    skr::TransformSystem* transform_system;
    sugoi_storage_t* storage = nullptr;
    skr::task::scheduler_t scheduler;

    skr::Vector<sugoi_entity_t> root_entities;
    skr::Vector<sugoi_entity_t> level1_entities;
    skr::Vector<sugoi_entity_t> level2_entities;
    skr::Vector<sugoi_entity_t> level3_entities;

    skr::Vector<skr::TransformF> expectedRootTransforms;
    skr::Vector<skr::TransformF> expectedLevel1Transforms;
    skr::Vector<skr::TransformF> expectedLevel2Transforms;
    skr::Vector<skr::TransformF> expectedLevel3Transforms;
};

TEST_CASE_METHOD(ComplexTransformTest, "Complex Transform Hierarchy Update")
{
    SkrZoneScopedN("ComplexTransformHierarchyTest");

    // Update the transform system
    transform_system->update();

    // Verify results
    auto checkQuery = storage->new_query()
                          .ReadAny<skr::scene::ParentComponent, skr::scene::ChildrenComponent, skr::scene::RootComponent>()
                          .ReadAll<skr::scene::PositionComponent, skr::scene::RotationComponent, skr::scene::ScaleComponent>()
                          .ReadAll<skr::scene::TransformComponent>()
                          .commit()
                          .value();
    SKR_DEFER({ storage->destroy_query(checkQuery); });

    int verified_count = 0;
    int correct_count = 0;

    storage->getScheduler()
        ->schedule_ecs_job(
            checkQuery, 1, +[](void* userdata, sugoi_query_t* query, sugoi_chunk_view_t* view, sugoi_type_index_t* localTypes, EIndex entityIndex) -> void {
                SkrZoneScopedN("ComplexTransformTestBody");
                auto _this = (ComplexTransformTest*)userdata;
                const auto entities = sugoiV_get_entities(view);
                auto transforms = sugoi::get_owned<const skr::scene::TransformComponent>(view);

                for (uint32_t i = 0; i < view->count; i++)
                {
                    const auto& actual_transform = transforms[i].value;
                    skr::TransformF expected_transform;
                    bool found_expected = false;

                    // Find which level this entity belongs to and get expected transform
                    for (int j = 0; j < _this->root_entities.size(); ++j)
                    {
                        if (entities[i] == _this->root_entities[j])
                        {
                            expected_transform = _this->expectedRootTransforms[j];
                            found_expected = true;
                            break;
                        }
                    }

                    if (!found_expected)
                    {
                        for (int j = 0; j < _this->level1_entities.size(); ++j)
                        {
                            if (entities[i] == _this->level1_entities[j])
                            {
                                expected_transform = _this->expectedLevel1Transforms[j];
                                found_expected = true;
                                break;
                            }
                        }
                    }

                    if (!found_expected)
                    {
                        for (int j = 0; j < _this->level2_entities.size(); ++j)
                        {
                            if (entities[i] == _this->level2_entities[j])
                            {
                                expected_transform = _this->expectedLevel2Transforms[j];
                                found_expected = true;
                                break;
                            }
                        }
                    }

                    if (!found_expected)
                    {
                        for (int j = 0; j < _this->level3_entities.size(); ++j)
                        {
                            if (entities[i] == _this->level3_entities[j])
                            {
                                expected_transform = _this->expectedLevel3Transforms[j];
                                found_expected = true;
                                break;
                            }
                        }
                    }

                    if (found_expected)
                    {
                        CHECK(skr::all(nearly_equal(expected_transform.position, actual_transform.position)));
                        CHECK(skr::all(nearly_equal(expected_transform.scale, actual_transform.scale)));
                        CHECK(skr::all(nearly_equal(expected_transform.rotation, actual_transform.rotation)));
                    }
                    else
                    {
                        SKR_LOG_WARN(u8"Entity %llu not found in expected transform lists", entities[i]);
                    }
                }
            },
            this,
            nullptr,
            nullptr,
            nullptr)
        .wait(true);

    SKR_LOG_INFO(u8"Complex transform verification completed");
}

// Test edge cases with missing components
TEST_CASE_METHOD(ComplexTransformTest, "Transform Missing Components")
{
    SkrZoneScopedN("TransformMissingComponents");

    // Create entity with only TransformComponent and RootComponent (missing Translation/Rotation/Scale)
    sugoi::EntitySpawner<skr::scene::RootComponent, skr::scene::TransformComponent> minimal_spawner;
    sugoi_entity_t minimal_entity = SUGOI_NULL_ENTITY;

    minimal_spawner(storage, 1, [&](auto& view) {
        minimal_entity = sugoiV_get_entities(view.view)[0];
    });

    // Update transform system
    transform_system->update();

    // Verify minimal entity has identity transform
    auto minimal_query = storage->new_query()
                             .ReadAll<skr::scene::TransformComponent>()
                             .ReadAll<skr::scene::RootComponent>()
                             .commit()
                             .value();
    SKR_DEFER({ storage->destroy_query(minimal_query); });

    bool minimal_verified = false;
    struct
    {
        sugoi_entity_t entity;
        bool* verified;
    } test_data = { minimal_entity, &minimal_verified };

    storage->query(minimal_query, +[](void* userdata, sugoi_chunk_view_t* view) -> void {
        auto data = (decltype(test_data)*)userdata;
        auto entities = sugoiV_get_entities(view);
        auto transforms = sugoi::get_owned<const skr::scene::TransformComponent>(view);
        
        for (uint32_t i = 0; i < view->count; i++) {
            if (entities[i] == data->entity) {
                const auto& transform = transforms[i].value;
                
                // Should have identity transform (defaults)
                CHECK(skr::all(nearly_equal(transform.position, skr_float3_t{0,0,0})));
                CHECK(skr::all(nearly_equal(transform.scale, skr_float3_t{1,1,1})));
                // For quaternion identity, check if it's close to (0,0,0,1) or (0,0,0,-1)
                skr::QuatF identity = skr::QuatF::Identity();
                CHECK(skr::all(nearly_equal(transform.rotation, identity)));
                
                *(data->verified) = true;
                break;
            }
        } }, &test_data);

    CHECK(minimal_verified);
}
