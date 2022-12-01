#pragma once
#include "SkrAnimTool/module.configure.h"
#include "SkrToolCore/asset/importer.hpp"
#ifndef __meta__
    #include "SkrAnimTool/animation_asset.generated.h"
#endif
struct skr_skeleton_resource_t;
namespace ozz::animation::offline
{
struct RawSkeleton;
struct RawAnimation;
} // namespace ozz::animation::offline

namespace skd sreflect
{
namespace asset sreflect
{
using RawAnimation = ozz::animation::offline::RawAnimation;

sreflect_struct("guid"
                : "37d07586-0901-480a-8dcd-1f1f8220569c")
sattr("serialize" : "json")
SKR_ANIMTOOL_API SAnimGltfImporter : public skd::asset::SImporter
{
    sattr("no-default" : true)
    skr::string assetPath;
    sattr("no-default" : true)
    eastl::string animationName;
    float samplingRate = 30.f;
    virtual ~SAnimGltfImporter() = default;
    virtual void* Import(skr_io_ram_service_t*, SCookContext * context) override;
    virtual void Destroy(void*) override;
    static uint32_t Version() { return kDevelopmentVersion; }
}
sregister_importer();

sreflect_enum_class("guid" : "544116F5-EBE9-4837-AB88-4743435F39EF")
sattr("serialize" : "json")
SAnimAdditiveReference : uint32_t {
    animation, 
    skeleton
};

sreflect_struct("guid" : "9B780FFE-FA11-4BA9-B410-B5D5B2849E64")
sattr("serialize" : "json")
SAnimOptimizationOverride
{
    /*
    {
        "name" : "*", //  Joint name. Wildcard characters '*' and '?' are supported
        "tolerance" : 0.001, //  The maximum error that an optimization is allowed to generate on a whole joint hierarchy.
        "distance" : 0.1 //  The distance (from the joint) at which error is measured. This allows to emulate effect on skinning.
    }
    */
    eastl::string name = "*";
    float tolerance = 0.001f;
    float distance = 0.1f;
};

sreflect_struct("guid" : "13873706-F7EE-4386-B7F0-B4E313864624")
sattr("serialize" : "json")
SAnimCookSettings
{
    /*
        "additive" : false, //  Creates a delta animation that can be used for additive blending.
        "additive_reference" : "animation", //  Select reference pose to use to build additive/delta animation. Can be "animation" to use the 1st animation keyframe as reference, or "skeleton" to use skeleton rest pose.
        "sampling_rate" : 0, //  Selects animation sampling rate in hertz. Set a value <= 0 to use imported scene default frame rate.
        "optimize" : true, //  Activates keyframes reduction optimization.
        "tolerance" : 0.001, //  The maximum error that an optimization is allowed to generate on a whole joint hierarchy.
        "distance" : 0.1, //  The distance (from the joint) at which error is measured. This allows to emulate effect on skinning.
        //  Per joint optimization setting override
        "override" : 
        [
            {
            "name" : "*", //  Joint name. Wildcard characters '*' and '?' are supported
            "tolerance" : 0.001, //  The maximum error that an optimization is allowed to generate on a whole joint hierarchy.
            "distance" : 0.1 //  The distance (from the joint) at which error is measured. This allows to emulate effect on skinning.
            }
        ]
    */
    resource::TResourceHandle<skr_skeleton_resource_t> skeletonAsset;
    bool additive = false; //  Creates a delta animation that can be used for additive blending.
    SAnimAdditiveReference additiveReference = SAnimAdditiveReference::animation; //  Select reference pose to use to build additive/delta animation. Can be "animation" to use the 1st animation keyframe as reference, or "skeleton" to use skeleton rest pose.
    float samplingRate = 0.f; //  Selects animation sampling rate in hertz. Set a value <= 0 to use imported scene default frame rate.
    bool optimize = true; //  Activates keyframes reduction optimization.
    float tolerance = 0.001f; //  The maximum error that an optimization is allowed to generate on a whole joint hierarchy.
    float distance = 0.1f; //  The distance (from the joint) at which error is measured. This allows to emulate effect on skinning.
    eastl::vector<SAnimOptimizationOverride> override; //  Per joint optimization setting override
};

struct sreflect SKR_ANIMTOOL_API SAnimCooker : public skd::asset::SCooker
{
    bool Cook(SCookContext * ctx) override;
    uint32_t Version() override { return kDevelopmentVersion; }
}
sregister_cooker("5D6DC46B-8696-4DD8-ADE4-C27D07CEDCCD");
} // namespace asset sreflect
} // namespace skd sreflect