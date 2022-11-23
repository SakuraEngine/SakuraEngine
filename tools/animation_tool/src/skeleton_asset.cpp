#include "SkrAnimTool/skeleton_asset.h"
#include "SkrAnim/ozz/skeleton.h"
#include "SkrAnimTool/ozz/raw_skeleton.h"
#include "SkrAnimTool/ozz/skeleton_builder.h"
#include "tools/import2ozz_skel.h"
#include "SkrAnim/ozz/base/memory/unique_ptr.h"
#include "SkrAnim/ozz/base/io/stream.h"
#include "SkrAnim/ozz/base/io/archive.h"
#include "utils/log.hpp"
#include "simdjson.h"

namespace skd::asset
{
bool SSkelCooker::Cook(SCookContext *ctx)
{
    using namespace ozz::animation::offline;
    //-----load config
    simdjson::ondemand::parser parser;
    auto doc = parser.iterate(ctx->GetAssetRecord()->meta);
    ozz::Endianness endianness = ozz::GetNativeEndianness();
    auto _endianness = doc["endianness"];
    if(_endianness.error() == simdjson::error_code::SUCCESS)
    {
        auto __endianness = _endianness.value_unsafe().get_string();
        if(__endianness.error() == simdjson::error_code::SUCCESS)
        {
            if(__endianness.value_unsafe() == "big")
            {
                endianness = ozz::Endianness::kBigEndian;
            }
            else if(__endianness.value_unsafe() == "little")
            {
                endianness = ozz::Endianness::kLittleEndian;
            }
        }
    }
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
    ozz::unique_ptr<ozz::animation::Skeleton> skeleton;
    {
        SkeletonBuilder builder;
        skeleton = builder(*rawSkeleton);
        if (!skeleton) {
            SKR_LOG_ERROR("Failed to build skeleton for asset %s.", ctx->GetAssetRecord()->path.c_str());
            return false;
        }
    }
    //-----fetch runtime dependencies
    // no runtime dependencies
    //------write resource object
    const auto outputPath = ctx->GetOutputPath();
    ozz::io::File stream(outputPath.u8string().c_str(), "wb");
    if (!stream.opened()) {
        SKR_LOG_ERROR("Failed to open output file %s.", outputPath.u8string().c_str());
        return false;
    }
    ozz::io::OArchive ozzArchive(&stream, endianness);
    ozzArchive << *skeleton;
    return true;
}
}