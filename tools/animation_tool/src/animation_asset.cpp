#include "SkrAnimTool/animation_asset.h"
#include "SkrAnim/ozz/animation.h"
#include "SkrAnimTool/ozz/raw_animation.h"
#include "SkrAnimTool/ozz/animation_optimizer.h"
#include "SkrAnimTool/ozz/additive_animation_builder.h"
#include "SkrAnimTool/ozz/animation_builder.h"
#include "tools/import2ozz_utils.h"
#include "SkrAnim/ozz/base/containers/vector.h"
#include "SkrAnim/ozz/base/memory/unique_ptr.h"
#include "SkrAnim/ozz/base/io/stream.h"
#include "SkrAnim/ozz/base/io/archive.h"
#include "utils/log.hpp"
#include "simdjson.h"
#include "SkrAnim/resources/skeleton_resource.h"
#include "SkrAnim/resources/animation_resource.h"

namespace skd::asset
{
bool SAnimCooker::Cook(SCookContext *ctx)
{
    using namespace ozz::animation::offline;
    //-----load config
    auto settings = ctx->Config<SAnimCookSettings>();
    //-----emit static dependencies
    if(settings.skeletonAsset.get_serialized() == skr_guid_t{})
    {
        SKR_LOG_ERROR("Failed to cook animation asset %s. No skeleton asset specified.", ctx->GetAssetRecord()->path.c_str());
        return false;
    }
    auto idx = ctx->AddStaticDependency(settings.skeletonAsset.get_serialized(), true);
    if(ctx->GetStaticDependency(idx).get_status() == SKR_LOADING_STATUS_ERROR)
        return false;
    skr_skeleton_resource_t* skeletonResource = (skr_skeleton_resource_t*)ctx->GetStaticDependency(idx).get_ptr();
    auto& skeleton = skeletonResource->skeleton;
    //-----import resource object
    RawAnimation* rawAnimation = (RawAnimation*)ctx->Import<RawAnimation>();
    if (!rawAnimation)
    {
        return false;
    }
    SKR_DEFER({ctx->Destroy(rawAnimation);});
    //-----emit dependencies
    // no static dependencies
    //-----cook resource
    if(settings.optimize)
    {
        AnimationOptimizer optimizer;
        AnimationOptimizer::Setting optSettings;
        optSettings.tolerance = settings.tolerance;
        optSettings.distance = settings.distance;
        optimizer.setting = optSettings;
        for (int i = 0; i < settings.override.size(); ++i)
        {
            bool found = false;
            auto& override = settings.override[i];
            for (int j = 0; j < skeleton.num_joints(); ++j) {
                const char* joint_name = skeleton.joint_names()[j];
                if (ozz::strmatch(joint_name, override.name.c_str())) {
                    found = true;

                    SKR_LOG_TRACE("Found joint \"%s\" matching pattern \"%s\" for joint optimization setting override.", joint_name, override.name.c_str());

                    const AnimationOptimizer::JointsSetting::value_type entry(j, optSettings);
                    const bool newly =
                        optimizer.joints_setting_override.insert(entry).second;
                    if (!newly) {
                        SKR_LOG_TRACE("Redundant optimization setting for pattern \"%s\".", override.name.c_str());
                    }
                }
            }
            
            optSettings = optimizer.setting;
            
            if (!found) {
                SKR_LOG_INFO("No joint matching pattern \"%s\" for joint optimization setting override.", override.name.c_str());
            }
        }
        RawAnimation rawOptimizedAnimation;
        if (!optimizer(*rawAnimation, skeleton, &rawOptimizedAnimation))
        {
            SKR_LOG_ERROR("Failed to optimize animation.");
            return false;
        }

        // Displays optimization statistics.
        DisplaysOptimizationstatistics(*rawAnimation, rawOptimizedAnimation);

        // Brings data back to the raw animation.
        *rawAnimation = std::move(rawOptimizedAnimation);
    }
    if(settings.additive)
    {
        AdditiveAnimationBuilder additiveBuilder;
        RawAnimation rawAdditive;
        bool succeeded = false;

        if (settings.additiveReference == SAnimAdditiveReference::skeleton) {
            const ozz::vector<ozz::math::Transform> transforms =
                SkeletonRestPoseSoAToAoS(skeleton);
            succeeded =
                additiveBuilder(*rawAnimation, ozz::make_span(transforms), &rawAdditive);
        } else {
            succeeded = additiveBuilder(*rawAnimation, &rawAdditive);
        }

        if (!succeeded) {
            SKR_LOG_ERROR("Failed to build additive animation.");
            return false;
        }

        // Now use additive animation.
        *rawAnimation = std::move(rawAdditive);
    }
    
    AnimationBuilder builder;
    ozz::unique_ptr<ozz::animation::Animation> animation = builder(*rawAnimation);
    if (!animation) {
        SKR_LOG_ERROR("Failed to build animation.");
        return false;
    }
    skr_anim_resource_t resource;
    resource.animation = std::move(*animation);
    //-----emit runtime dependencies
    // no runtime dependencies
    //------write resource object
    ctx->Save(resource);
    return true;
}
}