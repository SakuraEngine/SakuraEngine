#pragma once
#include "GameRuntime/module.configure.h"
#include "SkrAnim/resources/animation_resource.h"
#include "SkrAnim/resources/skeleton_resource.h"
#include "SkrAnim/resources/skin_resource.h"
#include "SkrAnim/ozz/base/maths/soa_transform.h"
#include "SkrAnim/ozz/sampling_job.h"
#ifndef __meta__
#include "GameRuntime/game_animation.generated.h"
#endif

struct skr_render_anim_comp_t;

namespace game sreflect
{  
    sreflect_struct("guid" : "E06E11F7-6F3A-4BFF-93D8-37310EF0FB87")
    sattr("component" : 
    {
        "custom" : "::dual::managed_component"
    })
    sattr("scriptable" : true)
    anim_state_t
    {
        SKR_RESOURCE_FIELD(skr_anim_resource_t, animation_resource);
        sattr("native" : true)
        eastl::vector<ozz::math::SoaTransform> local_transforms;
        sattr("native" : true)
        ozz::animation::SamplingJob::Context sampling_context;
        float currtime = 0.f;
    };

    GAME_RUNTIME_API void InitializeAnimState(anim_state_t* state, skr_skeleton_resource_t* skeleton);
    GAME_RUNTIME_API void UpdateAnimState(anim_state_t* state, skr_skeleton_resource_t* skeleton, float dt, skr_render_anim_comp_t* output);
}