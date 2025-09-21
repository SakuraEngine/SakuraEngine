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

// Simple 3-level hierarchy test
constexpr int SIMPLE_CHILDREN_COUNT = 2;

static const auto simpleParentTranslation = skr_float3_t{ 10.0f, 0.0f, 0.0f };
static const auto simpleParentRotation = skr_rotator_f_t{ 0.0f, 45.0f, 0.0f };
static const auto simpleParentScale = skr_float3_t{ 2.0f, 2.0f, 2.0f };
const skr::TransformF simpleParentTransform{
    skr::QuatF(simpleParentRotation),
    simpleParentTranslation,
    simpleParentScale
};

static const auto simpleChildTranslation = skr_float3_t{ 5.0f, 0.0f, 0.0f };
static const auto simpleChildRotation = skr_rotator_f_t{ 0.0f, 0.0f, 0.0f };
static const auto simpleChildScale = skr_float3_t{ 1.0f, 1.0f, 1.0f };
const skr::TransformF simpleChildTransformLocal{
    skr::QuatF(simpleChildRotation),
    simpleChildTranslation,
    simpleChildScale
};

// Expected world transform for child: parent * child_local
const skr::TransformF simpleChildTransformWorld = simpleParentTransform * simpleChildTransformLocal;

struct SimpleTransformTest
{
    SimpleTransformTest()
    {
        ::skr_log_set_level(SKR_LOG_LEVEL_WARN);
        ::skr_log_initialize_async_worker();
        storage = sugoiS_create();
        transform_system = skr::TransformSystem::Create(storage);

        spawnEntities();

        scheduler.initialize(skr::task::scheudler_config_t());
        scheduler.bind();
        sugoiJ_bind_storage(storage);
    }

    ~SimpleTransformTest()
    {
        sugoiJ_unbind_storage(storage);
        skr::TransformSystem::Destroy(transform_system);
        sugoiS_release(storage);
        scheduler.unbind();
        ::skr_log_finalize_async_worker();
    }

    void spawnEntities()
    {
        SkrZoneScopedN("SpawnSimpleHierarchy");

        // Create 1 root entity
        sugoi::EntitySpawner<skr::scene::RootComponent, SKR_SCENE_COMPONENTS> root_spawner;
        root_spawner(storage, 1, [&](auto& view) {
            auto translations = sugoi::get_owned<skr::scene::PositionComponent>(view.view);
            auto rotations = sugoi::get_owned<skr::scene::RotationComponent>(view.view);
            auto scales = sugoi::get_owned<skr::scene::ScaleComponent>(view.view);

            translations[0].value = simpleParentTranslation;
            rotations[0].euler = simpleParentRotation;
            scales[0].value = simpleParentScale;
            parent = sugoiV_get_entities(view.view)[0];
            SKR_LOG_INFO(u8"Simple parent entity created: %llu", parent);
        });

        // Create 2 child entities
        sugoi::EntitySpawner<SKR_SCENE_COMPONENTS> children_spawner;
        children.reserve(SIMPLE_CHILDREN_COUNT);
        children_spawner(storage, SIMPLE_CHILDREN_COUNT, [&](auto& view) {
            auto translations = sugoi::get_owned<skr::scene::PositionComponent>(view.view);
            auto rotations = sugoi::get_owned<skr::scene::RotationComponent>(view.view);
            auto scales = sugoi::get_owned<skr::scene::ScaleComponent>(view.view);
            auto parents = sugoi::get_owned<skr::scene::ParentComponent>(view.view);

            for (uint32_t i = 0; i < view.count(); ++i)
            {
                translations[i].value = simpleChildTranslation;
                rotations[i].euler = simpleChildRotation;
                scales[i].value = simpleChildScale;
                parents[i].entity = this->parent;

                children.add(sugoiV_get_entities(view.view)[i]);
                SKR_LOG_INFO(u8"Simple child entity %d created: %llu", i, sugoiV_get_entities(view.view)[i]);
            }
        });

        // Setup parent-child relationship
        auto setupAttachQuery = storage->new_query()
                                    .ReadWriteAny<skr::scene::ChildrenComponent>()
                                    .ReadAll<skr::scene::RootComponent>()
                                    .commit()
                                    .value();
        SKR_DEFER({ storage->destroy_query(setupAttachQuery); });

        storage->query(setupAttachQuery, +[](void* userdata, sugoi_chunk_view_t* view) -> void {
            auto _this = (SimpleTransformTest*)userdata;
            auto pChildren = (skr::scene::ChildrenArray*)sugoi::get_owned<skr::scene::ChildrenComponent>(view);
            for (uint32_t i = 0; i < _this->children.size(); ++i) {
                pChildren->emplace_back(skr::scene::ChildrenComponent{ _this->children[i] });
            } }, this);

        SKR_LOG_INFO(u8"Simple hierarchy setup complete: 1 parent, %d children", SIMPLE_CHILDREN_COUNT);
    }

    skr::TransformSystem* transform_system;
    sugoi_storage_t* storage = nullptr;
    skr::task::scheduler_t scheduler;

    sugoi_entity_t parent = SUGOI_NULL_ENTITY;
    skr::Vector<sugoi_entity_t> children;
};

TEST_CASE_METHOD(SimpleTransformTest, "Simple Transform Hierarchy")
{
    SkrZoneScopedN("SimpleTransformHierarchyTest");

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

    storage->getScheduler()
        ->schedule_ecs_job(
            checkQuery, 1, +[](void* userdata, sugoi_query_t* query, sugoi_chunk_view_t* view, sugoi_type_index_t* localTypes, EIndex entityIndex) -> void {
                SkrZoneScopedN("SimpleTransformTestBody");
                auto _this = (SimpleTransformTest*)userdata;
                const auto entities = sugoiV_get_entities(view);
                auto transforms = sugoi::get_owned<const skr::scene::TransformComponent>(view);

                for (uint32_t i = 0; i < view->count; i++)
                {
                    const auto& actual_transform = transforms[i].value;

                    if (entities[i] == _this->parent)
                    {
                        // Check parent transform
                        CHECK(skr::all(nearly_equal(simpleParentTransform.position, actual_transform.position)));
                        CHECK(skr::all(nearly_equal(simpleParentTransform.scale, actual_transform.scale)));
                        CHECK(skr::all(nearly_equal(simpleParentTransform.rotation, actual_transform.rotation)));
                        SKR_LOG_INFO(u8"Parent transform verified");
                    }
                    else
                    {
                        // Check if this is a child
                        bool is_child = false;
                        for (const auto& child : _this->children)
                        {
                            if (entities[i] == child)
                            {
                                is_child = true;
                                break;
                            }
                        }

                        if (is_child)
                        {
                            // Check child world transform
                            CHECK(skr::all(nearly_equal(simpleChildTransformWorld.position, actual_transform.position)));
                            CHECK(skr::all(nearly_equal(simpleChildTransformWorld.scale, actual_transform.scale)));
                            CHECK(skr::all(nearly_equal(simpleChildTransformWorld.rotation, actual_transform.rotation)));
                            SKR_LOG_INFO(u8"Child transform verified");
                        }
                    }
                }
            },
            this,
            nullptr,
            nullptr,
            nullptr)
        .wait(true);

    SKR_LOG_INFO(u8"Simple transform verification completed");
}