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

constexpr int CHILDREN_COUNT = 10;
static const auto parentTranslation = skr_float3_t{ 1.f, 2.f, 3.f };
static const auto parentRotation = skr_rotator_f_t{ 0.f, 0.f, 0.f };
static const auto parentScale = skr_float3_t{ 1.f, 2.f, 3.f };
const skr::TransformF parentTransform{
    skr::QuatF(parentRotation),
    parentTranslation,
    parentScale
};

static const auto childTranslation = skr_float3_t{ 1.f, 2.f, 3.f };
static const auto childRotation = skr_rotator_f_t{ 10.f, 20.f, 30.f };
static const auto childScale = skr_float3_t{ 1.f, 2.f, 3.f };
static const skr::TransformF childTransformRelative{
    skr::QuatF(childRotation),
    childTranslation,
    childScale
};
static const skr::TransformF childTransform = parentTransform * childTransformRelative;

struct TransformTests
{
    TransformTests()
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

    ~TransformTests() SKR_NOEXCEPT
    {
        sugoiJ_unbind_storage(storage);
        ::sugoiS_release(storage);
        scheduler.unbind();
        ::skr_log_finalize_async_worker();
    }

    void spawnEntities()
    {
        SkrZoneScopedN("TestTransformUpdate");

        sugoi::EntitySpawner<skr::scene::RootComponent, SKR_SCENE_COMPONENTS> root_spawner;
        {
            SkrZoneScopedN("InitializeParentEntities");
            root_spawner(storage, 1, [&](auto& view) {
                auto translations = sugoi::get_owned<skr::scene::PositionComponent>(view.view);
                auto rots = sugoi::get_owned<skr::scene::RotationComponent>(view.view);
                auto scales = sugoi::get_owned<skr::scene::ScaleComponent>(view.view);

                translations[0].value = parentTranslation;
                rots[0].euler = parentRotation;
                scales[0].value = parentScale;
                parent = sugoiV_get_entities(view.view)[0]; // set as parent entity
                SKR_LOG_INFO(u8"Parent entity created: %llu", parent);
            });
        }
        sugoi::EntitySpawner<SKR_SCENE_COMPONENTS> children_spawner;
        {
            SkrZoneScopedN("InitializeChildEntities");
            children.reserve(CHILDREN_COUNT);
            children_spawner(storage, CHILDREN_COUNT, [&](auto& view) {
                auto translations = sugoi::get_owned<skr::scene::PositionComponent>(view.view);
                auto rots = sugoi::get_owned<skr::scene::RotationComponent>(view.view);
                auto scales = sugoi::get_owned<skr::scene::ScaleComponent>(view.view);
                auto parents = sugoi::get_owned<skr::scene::ParentComponent>(view.view);
                for (uint32_t i = 0; i < view.count(); ++i)
                {
                    translations[i].value = childTranslation;
                    rots[i].euler = childRotation;
                    scales[i].value = childScale;
                    parents[i].entity = this->parent;

                    children.add(sugoiV_get_entities(view.view)[i]);
                }
            });
        }

        // do attach
        auto setupAttachQuery = storage->new_query()
                                    .ReadWriteAny<skr::scene::ChildrenComponent>()
                                    .ReadAll<skr::scene::RootComponent>()
                                    .commit()
                                    .value();
        SKR_DEFER({ storage->destroy_query(setupAttachQuery); });
        {
            SkrZoneScopedN("AttachEntities");
            storage->query(setupAttachQuery, +[](void* userdata, sugoi_chunk_view_t* view) -> void {
                        auto _this = (TransformTests*)userdata;
                        auto pChildren = (skr::scene::ChildrenArray*)sugoi::get_owned<skr::scene::ChildrenComponent>(view);
                        for (uint32_t i = 0; i < _this->children.size(); ++i)
                        {
                            pChildren->emplace_back(skr::scene::ChildrenComponent{ _this->children[i] });
                        } }, this);
        }
    }

    skr::TransformSystem* transform_system;
    sugoi_storage_t* storage = nullptr;
    skr::task::scheduler_t scheduler;

    sugoi_entity_t parent = SUGOI_NULL_ENTITY;
    skr::Vector<sugoi_entity_t> children;
};

TEST_CASE_METHOD(TransformTests, "TransformUpdate")
{
    SkrZoneScopedN("TestTransformUpdate");
    transform_system->update();

    auto checkQuery = storage->new_query()
                          .ReadAny<skr::scene::ParentComponent, skr::scene::ChildrenComponent>()
                          .ReadAll<skr::scene::PositionComponent, skr::scene::RotationComponent, skr::scene::ScaleComponent>()
                          .ReadAll<skr::scene::TransformComponent>()
                          .commit()
                          .value();
    SKR_DEFER({ storage->destroy_query(checkQuery); });

    storage->getScheduler()
        ->schedule_ecs_job(
            checkQuery, 1, +[](void* userdata, sugoi_query_t* query, sugoi_chunk_view_t* view, sugoi_type_index_t* localTypes, EIndex entityIndex) -> void {
                SkrZoneScopedN("TransformTestBody");
                auto _this = (TransformTests*)userdata;
                const auto es = sugoiV_get_entities(view);
                auto transforms = sugoi::get_owned<const skr::scene::TransformComponent>(view);

                // batchly check
                for (uint32_t i = 0; i < view->count; i++)
                {
                    const auto Transfrom = transforms[i].value;
                    if (es[i] == _this->parent)
                    {
                        CHECK(skr::all(nearly_equal(parentTransform.position, Transfrom.position)));
                        CHECK(skr::all(nearly_equal(parentTransform.scale, Transfrom.scale)));
                        CHECK(skr::all(nearly_equal(parentTransform.rotation, Transfrom.rotation)));
                    }
                    else
                    {
                        CHECK(skr::all(nearly_equal(childTransform.position, Transfrom.position)));
                        CHECK(skr::all(nearly_equal(childTransform.scale, Transfrom.scale)));
                        CHECK(skr::all(nearly_equal(childTransform.rotation, Transfrom.rotation)));
                    }
                }
            },
            this,
            nullptr,
            nullptr,
            nullptr)
        .wait(true);
};
