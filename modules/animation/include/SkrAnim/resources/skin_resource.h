#pragma once
#include "SkrRenderer/resources/mesh_resource.h"
#include "SkrAnim/resources/skeleton_resource.h"
#include "SkrAnim/ozz/base/maths/simd_math.h"
#ifndef __meta__
    #include "SkrAnim/resources/skin_resource.generated.h"
#endif

sreflect_struct("guid": "C387FD0E-83BE-4617-9A79-589862F3F941") 
sattr("blob" : true)
skr_skin_bin_t
{
    gsl::span<eastl::string_view> joint_remaps;
    gsl::span<skr_float4x4_t> inverse_bind_poses;
};

sreflect_struct("guid" : "332C6133-7222-4B88-9B2F-E4336A46DF2C")
sattr("rtti" : true)
sattr("serialize" : "bin")
skr_skin_resource_t
{
    skr::resource::TResourceHandle<skr_skeleton_resource_t> skeleton;
    sattr("no-rtti" : true)
    skr_blob_arena_t arena;
    sattr("arena" : "arena")
    sattr("no-rtti" : true)
    skr_skin_bin_t bin;
};

namespace skr::resource
{
struct SKR_ANIM_API SSkinFactory : public SResourceFactory {
    virtual ~SSkinFactory() = default;

    struct Root {
        skr_vfs_t* vfs = nullptr;
        skr::filesystem::path dstorage_root;
        skr_io_ram_service_t* ram_service = nullptr;
        skr::io::VRAMService* vram_service = nullptr;
        SRenderDeviceId render_device = nullptr;
    };

    float AsyncSerdeLoadFactor() override { return 2.5f; }
    [[nodiscard]] static SSkinFactory* Create(const Root& root);
    static void Destroy(SSkinFactory* factory);
};
} // namespace skr::resource