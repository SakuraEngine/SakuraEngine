#include "SkrToolCore/cook_system/cook_system.hpp"
#include "SkrAnimTool/animation_asset.h"
#include "SkrAnim/ozz/animation.h"
#include "SkrAnimTool/ozz/animation_optimizer.h"
#include "SkrAnimTool/ozz/additive_animation_builder.h"
#include "SkrAnimTool/ozz/animation_builder.h"
#include "SkrAnimTool/ozz/raw_animation.h"

#include "tools/import2ozz_utils.h"

#include "SkrAnim/ozz/base/containers/vector.h"
#include "SkrAnim/ozz/base/memory/unique_ptr.h"
#include "SkrAnim/resources/skeleton_resource.hpp"
#include "SkrAnim/resources/animation_resource.hpp"

#include "SkrProfile/profile.h"

namespace skd::asset
{
bool AnimCooker::Cook(CookContext* ctx)
{
    using namespace skr;
    using namespace ozz::animation::offline;
    SkrZoneScopedNS("AnimCooker::Cook", 4);

    //-----load config
    auto anim_asset_file = ctx->GetAssetMetaFile();
    auto& anim_asset = *anim_asset_file->GetMetadata<AnimAsset>();
    //-----emit static dependencies
    if (anim_asset.skeletonAsset.get_serialized().is_zero())
    {
        SKR_LOG_ERROR(u8"Failed to cook animation asset %s. No skeleton asset specified.", ctx->GetAssetMetaFile()->GetURI().string().c_str());
        return false;
    }
    auto idx = ctx->AddStaticDependency(anim_asset.skeletonAsset.get_serialized(), true);

    if (ctx->GetStaticDependency(idx).get_status() == SKR_LOADING_STATUS_ERROR)
        return false;
    SkeletonResource* skeletonResource = (SkeletonResource*)ctx->GetStaticDependency(idx).get_ptr();
    auto& skeleton = skeletonResource->skeleton;
    //-----import resource object
    RawAnimation* rawAnimation = (RawAnimation*)ctx->Import<RawAnimation>();
    if (!rawAnimation) return false;
    SKR_DEFER({ ctx->Destroy(rawAnimation); });
    //-----emit dependencies
    // no static dependencies
    //-----cook resource
    if (anim_asset.optimize)
    {
        AnimationOptimizer optimizer;
        AnimationOptimizer::Setting optanim_asset;
        optanim_asset.tolerance = anim_asset.tolerance;
        optanim_asset.distance = anim_asset.distance;
        optimizer.setting = optanim_asset;
        for (int i = 0; i < anim_asset.override.size(); ++i)
        {
            bool found = false;
            auto& override = anim_asset.override[i];
            for (int j = 0; j < skeleton.num_joints(); ++j)
            {
                const char* joint_name = skeleton.joint_names()[j];
                if (ozz::strmatch(joint_name, override.name.c_str_raw()))
                {
                    found = true;

                    SKR_LOG_TRACE(u8"Found joint \"%s\" matching pattern \"%s\" for joint optimization setting override.", joint_name, override.name.c_str());

                    const AnimationOptimizer::JointsSetting::value_type entry(j, optanim_asset);
                    const bool newly =
                        optimizer.joints_setting_override.insert(entry).second;
                    if (!newly)
                    {
                        SKR_LOG_TRACE(u8"Redundant optimization setting for pattern \"%s\".", override.name.c_str());
                    }
                }
            }

            optanim_asset = optimizer.setting;

            if (!found)
            {
                SKR_LOG_INFO(u8"No joint matching pattern \"%s\" for joint optimization setting override.", override.name.c_str());
            }
        }
        RawAnimation rawOptimizedAnimation;
        if (!optimizer(*rawAnimation, skeleton, &rawOptimizedAnimation))
        {
            SKR_LOG_ERROR(u8"Failed to optimize animation.");
            return false;
        }

        // Displays optimization statistics.
        DisplaysOptimizationstatistics(*rawAnimation, rawOptimizedAnimation);

        // Brings data back to the raw animation.
        *rawAnimation = std::move(rawOptimizedAnimation);
    }
    if (anim_asset.additive)
    {
        AdditiveAnimationBuilder additiveBuilder;
        RawAnimation rawAdditive;
        bool succeeded = false;

        if (anim_asset.additiveReference == AnimAdditiveReference::skeleton)
        {
            const ozz::vector<ozz::math::Transform> transforms =
                SkeletonRestPoseSoAToAoS(skeleton);
            succeeded =
                additiveBuilder(*rawAnimation, ozz::make_span(transforms), &rawAdditive);
        }
        else
        {
            succeeded = additiveBuilder(*rawAnimation, &rawAdditive);
        }

        if (!succeeded)
        {
            SKR_LOG_ERROR(u8"Failed to build additive animation.");
            return false;
        }

        // Now use additive animation.
        *rawAnimation = std::move(rawAdditive);
    }

    AnimationBuilder builder;
    ozz::unique_ptr<ozz::animation::Animation> animation = builder(*rawAnimation);
    if (!animation)
    {
        SKR_LOG_ERROR(u8"Failed to build animation.");
        return false;
    }
    AnimResource resource;
    resource.animation = std::move(*animation);
    //-----emit runtime dependencies
    // no runtime dependencies
    //------write resource object
    ctx->Save(resource);

    return true;
}
} // namespace skd::asset