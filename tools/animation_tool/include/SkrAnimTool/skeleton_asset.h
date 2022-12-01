#pragma once
#include "SkrAnimTool/module.configure.h"
#include "SkrToolCore/asset/importer.hpp"
#ifndef __meta__
    #include "SkrAnimTool/skeleton_asset.generated.h"
#endif

namespace ozz::animation::offline
{
struct RawSkeleton;
struct RawAnimation;
} // namespace ozz::animation::offline

namespace skd sreflect
{
namespace asset sreflect
{
using RawSkeleton = ozz::animation::offline::RawSkeleton;
sreflect_struct("guid"
                : "1719ab02-7a48-45db-b101-949155f92cad")
sattr("serialize" : "json")
SKR_ANIMTOOL_API SSkelGltfImporter : public skd::asset::SImporter
{
    // bool skeleton;
    // bool marker;
    // bool camera;
    // bool geometry;
    // bool light;
    // bool null;
    // bool any;
    sattr("no-default" : true)
    skr::string assetPath;
    virtual ~SSkelGltfImporter() = default;
    virtual void* Import(skr_io_ram_service_t*, SCookContext * context) override;
    virtual void Destroy(void*) override;
    static uint32_t Version() { return kDevelopmentVersion; }
}
sregister_importer();

struct sreflect SKR_ANIMTOOL_API SSkelCooker final : public SCooker
{
    bool Cook(SCookContext * ctx) override;
    uint32_t Version() override { return kDevelopmentVersion; }
}
sregister_cooker("1876BF35-E4DC-450B-B9D4-09259397F4BA");
} // namespace asset sreflect
} // namespace skd sreflect