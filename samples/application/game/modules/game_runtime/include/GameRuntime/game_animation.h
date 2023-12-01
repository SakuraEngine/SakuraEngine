#pragma once
#include "GameRuntime/module.configure.h"
#include "SkrAnim/resources/animation_resource.hpp"
#include "SkrAnim/resources/skeleton_resource.hpp"
#include "SkrAnim/resources/skin_resource.hpp"
#include "SkrAnim/ozz/base/maths/soa_transform.h"
#include "SkrAnim/ozz/sampling_job.h"
#ifndef __meta__
#include "GameRuntime/game_animation.generated.h" // IWYU pragma: export
#endif

namespace skr::anim
{
    struct AnimResource;
    struct AnimComponent;
}

namespace game sreflect
{  
    sreflect_managed_component("guid" : "E06E11F7-6F3A-4BFF-93D8-37310EF0FB87", "scriptable" : true)
    anim_state_t
    {
        SKR_RESOURCE_FIELD(skr::anim::AnimResource, animation_resource);
        sattr("native" : true)
        skr::vector<ozz::math::SoaTransform> local_transforms;
        sattr("native" : true)
        ozz::animation::SamplingJob::Context sampling_context;
        float currtime = 0.f;
    };

    GAME_RUNTIME_API void InitializeAnimState(anim_state_t* state, skr::anim::SkeletonResource* skeleton);
    GAME_RUNTIME_API void UpdateAnimState(anim_state_t* state, skr::anim::SkeletonResource* skeleton, float dt, skr::anim::AnimComponent* output);
}