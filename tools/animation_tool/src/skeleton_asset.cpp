#include "SkrToolCore/asset/cook_system.hpp"
#include "SkrAnimTool/skeleton_asset.h"
#include "SkrAnim/ozz/skeleton.h"
#include "SkrAnimTool/ozz/raw_skeleton.h"
#include "SkrAnimTool/ozz/skeleton_builder.h"
#include "tools/import2ozz_utils.h"
#include "SkrAnim/ozz/base/memory/unique_ptr.h"
#include "SkrAnim/ozz/base/io/stream.h"
#include "SkrAnim/ozz/base/io/archive.h"
#include "SkrRT/misc/log.hpp"
#include "SkrRT/serde/json/reader.h"
#include "SkrAnim/resources/skeleton_resource.h"

namespace skd::asset
{
bool SSkelCooker::Cook(SCookContext *ctx)
{
    SkrZoneScopedNS("SSkelCooker::Cook", 4);

    using namespace ozz::animation::offline;
    //-----load config
    simdjson::ondemand::parser parser;
    //-----import resource object
    RawSkeleton* rawSkeleton = (RawSkeleton*)ctx->Import<RawSkeleton>();
    if (!rawSkeleton)
    {
        return false;
    }
    SKR_DEFER({ctx->Destroy(rawSkeleton);});
    if(!ValidateJointNamesUniqueness(*rawSkeleton))
    {
        return false;
    }
    //-----emit dependencies
    // no static dependencies
    //-----cook resource
    skr::anim::SkeletonResource resource;
    {
        SkeletonBuilder builder;
        ozz::unique_ptr<ozz::animation::Skeleton> skeleton = builder(*rawSkeleton);
        if (!skeleton) {
            SKR_LOG_ERROR(u8"Failed to build skeleton for asset %s.", ctx->GetAssetRecord()->path.c_str());
            return false;
        }
        resource.skeleton = std::move(*skeleton);
    }
    //-----fetch runtime dependencies
    // no runtime dependencies
    //-----save resource
    ctx->Save(resource);
    return true;
}
}