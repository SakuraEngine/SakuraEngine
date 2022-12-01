#include "GameRuntime/game_animation.h"
#include "SkrAnim/ozz/local_to_model_job.h"
#include "SkrAnim/components/skin_component.h"
#include "utils/log.h"
#include "SkrTweak/module.h"
#include "SkrInspector/inspect_value.h"

namespace game
{
    void InitializeAnimState(anim_state_t *state, skr_skeleton_resource_t* skeleton)
    {
        state->sampling_context.Resize(skeleton->skeleton.num_joints());
        state->local_transforms.resize(skeleton->skeleton.num_soa_joints());
    }

    void UpdateAnimState(anim_state_t *state, skr_skeleton_resource_t* skeleton, float dt, skr_render_anim_comp_t *output)
    {
        auto anim = state->animation_resource.get_resolved();
        if(!anim) return;
        
        float newTime = SKR_INSPECT(state->currtime) + SKR_TWEAK(1.f) * dt;
        if(newTime > anim->animation.duration())
            newTime = std::fmodf(newTime, anim->animation.duration());
        ozz::animation::SamplingJob sampling_job;
        sampling_job.animation = &anim->animation;
        sampling_job.context = &state->sampling_context;
        sampling_job.ratio = newTime / anim->animation.duration();
        sampling_job.output = ozz::span{state->local_transforms.data(), state->local_transforms.size()};
        if (!sampling_job.Run()) {
            SKR_LOG_ERROR("Failed to sample animation %s.", anim->animation.name());
            return;
        }
        
        // Converts from local space to model space matrices.
        ozz::animation::LocalToModelJob ltm_job;
        ltm_job.skeleton = &skeleton->skeleton;
        ltm_job.input = ozz::span{state->local_transforms.data(), state->local_transforms.size()};
        output->joint_matrices.resize(skeleton->skeleton.num_joints());
        ltm_job.output = ozz::span{output->joint_matrices.data(), output->joint_matrices.size()};
        if (!ltm_job.Run()) {
            SKR_LOG_ERROR("Failed to convert local space to model space %s.", anim->animation.name());
            return;
        }
        state->currtime = newTime;
    }
}