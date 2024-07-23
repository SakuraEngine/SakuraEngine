#include "SkrBase/types.h"
#include "SkrBase/math/vector.h"
#include "SkrBase/math/quat.h"
#include "SkrBase/math/rtm/qvvf.h"
#include "SkrOS/thread.h"
#include "SkrCore/log.h"
#include "SkrContainers/vector.hpp"
#include "SkrRT/ecs/type_builder.hpp"
#include "SkrRT/ecs/storage.hpp"
#include "SkrRT/ecs/job.hpp"
#include "SkrScene/scene.h"
#include "SkrSerde/json_serde.hpp"

#include "SkrTestFramework/framework.hpp"

skr_transform_t make_qvv(skr_rotator_t r, skr_float3_t t, skr_float3_t s, const skr_transform_t* parent = nullptr)
{
    skr_transform_t result;
    const auto translation = skr::math::load(t);
    const auto scale = skr::math::load(s);
    const auto quat = skr::math::load(r);
    auto qvv = rtm::qvv_set(quat, translation, scale);
    if (parent)
    {
        const auto parentQVV = rtm::qvv_set(
            skr::math::load(parent->rotation), 
            skr::math::load(parent->translation), 
            skr::math::load(parent->scale)
        );
        qvv = rtm::qvv_mul(qvv, parentQVV);
    }
    skr::math::store(qvv.translation, result.translation);
    skr::math::store(qvv.rotation, result.rotation);
    skr::math::store(qvv.scale, result.scale);
    return result;
}

static constexpr auto parentTranslation = skr_float3_t{ 1.f, 2.f, 3.f };
static constexpr auto parentRotation = skr_rotator_t{ 0.f, 0.f, 0.f };
static constexpr auto parentScale = skr_float3_t{ 1.f, 2.f, 3.f };
const auto parentTransform = make_qvv(parentRotation, parentTranslation, parentScale);

static constexpr auto childTranslation = skr_float3_t{ 1.f, 2.f, 3.f };
static constexpr auto childRotation = skr_rotator_t{ 0.f, 0.f, 0.f };
static constexpr auto childScale = skr_float3_t{ 1.f, 2.f, 3.f };
const auto childTransform = make_qvv(childRotation, childTranslation, childScale, &parentTransform);

struct TransformTests {
    TransformTests()
    {
        skr_thread_sleep(3000);

        storage = sugoiS_create();
        skr_transform_system_setup(storage, &transform_system);

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
        skr_thread_sleep(3000);
    }

    void spawnEntities()
    {
        SkrZoneScopedN("TestTransformUpdate");

        sugoi::EntitySpawner<SKR_SCENE_COMPONENTS> root_spawner(transform_system.root_meta);
        {
            SkrZoneScopedN("InitializeParentEntities");
            root_spawner(storage, 1, 
                [&](auto& view){
                    auto translations = sugoi::get_owned<skr_translation_comp_t>(view.view);
                    auto rots = sugoi::get_owned<skr_rotation_comp_t>(view.view);
                    auto scales = sugoi::get_owned<skr_scale_comp_t>(view.view);

                    translations[0].value = parentTranslation;
                    rots[0].euler = parentRotation;
                    scales[0].value = parentScale;
                    parent = sugoiV_get_entities(view.view)[0];
                });
        }
        sugoi::EntitySpawner<SKR_SCENE_COMPONENTS> children_spawner;
        {
            SkrZoneScopedN("InitializeChildEntities");
            children.reserve(1024);
            children_spawner(storage, 1024, 
                [&](auto& view){
                    auto translations = sugoi::get_owned<skr_translation_comp_t>(view.view);
                    auto rots = sugoi::get_owned<skr_rotation_comp_t>(view.view);
                    auto scales = sugoi::get_owned<skr_scale_comp_t>(view.view);
                    auto parents = sugoi::get_owned<skr_parent_comp_t>(view.view);
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
            .ReadWriteAny<skr_child_comp_t>()
            .WithMetaEntity(transform_system.root_meta)
            .commit().value();
        SKR_DEFER({ storage->destroy_query(setupAttachQuery); });
        {
            SkrZoneScopedN("AttachEntities");
            storage->query(setupAttachQuery, 
            +[](void* userdata, sugoi_chunk_view_t* view) -> void {
                auto _this = (TransformTests*)userdata;
                auto pChildren = (skr_children_t*)sugoi::get_owned<skr_child_comp_t>(view);
                for (uint32_t i = 0; i < _this->children.size(); ++i)
                {
                    pChildren->emplace_back(skr_child_comp_t{ _this->children[i] });
                }
            }, this);
        }
    }

    skr_transform_system_t transform_system;
    sugoi_storage_t* storage = nullptr;
    skr::task::scheduler_t scheduler;

    sugoi_entity_t parent = SUGOI_NULL_ENTITY;
    skr::Vector<sugoi_entity_t> children;
};

TEST_CASE_METHOD(TransformTests, "update")
{
    SkrZoneScopedN("TestTransformUpdate");
    skr_transform_system_update(&transform_system);

    auto checkQuery = storage->new_query()
        .ReadAny<skr_parent_comp_t, skr_child_comp_t>()
        .ReadAll<skr_translation_comp_t, skr_rotation_comp_t, skr_scale_comp_t>()
        .ReadAll<skr_transform_comp_t>()
        .commit().value();
    SKR_DEFER({ storage->destroy_query(checkQuery); });

    storage->getScheduler()
        ->schedule_ecs_job(checkQuery, 1, 
        +[](void* userdata, sugoi_query_t* query, sugoi_chunk_view_t* view, sugoi_type_index_t* localTypes, EIndex entityIndex) -> void {
            SkrZoneScopedN("TransformTestBody");
            auto _this = (TransformTests*)userdata;
            const auto es = sugoiV_get_entities(view);
            auto transforms = sugoi::get_owned<const skr_transform_comp_t>(view);
            for (uint32_t i = 0; i < view->count; i++)
            {
                const auto Transfrom = transforms[i].value;
                if (es[i] == _this->parent)
                {
                    EXPECT_EQ(parentTransform.translation, Transfrom.translation);
                    EXPECT_EQ(parentTransform.scale, Transfrom.scale);
                    EXPECT_EQ(parentTransform.rotation, Transfrom.rotation);
                }
                else
                {
                    EXPECT_EQ(childTransform.translation, Transfrom.translation);
                    EXPECT_EQ(childTransform.scale, Transfrom.scale);
                    EXPECT_EQ(childTransform.rotation, Transfrom.rotation);
                }
            }
        }, this, nullptr, nullptr, nullptr)
    .wait(true);
}
