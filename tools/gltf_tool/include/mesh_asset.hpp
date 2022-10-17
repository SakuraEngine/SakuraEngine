#pragma once
#include "GLTFTool/module.configure.h"
#include "asset/importer.hpp"
#include "platform/configure.h"
#include "cgltf/cgltf.h"
#if !defined(__meta__) && defined(__cplusplus)
    #include "GLTFTool/json_reader.generated.h"
#endif

namespace skd sreflect
{
namespace asset sreflect
{
struct sreflect sattr(
"guid" : "D72E2056-3C12-402A-A8B8-148CB8EAB922",
"serialize" : "json",
"importer" : "3b8ca511-33d1-4db4-b805-00eea6a8d5e1"
)
GLTFTOOL_API SGltfMeshImporter final : public SImporter
{
    skr_guid_t placeHolder;
    void* Import(skr::io::RAMService*, const SAssetRecord* record) override;
};

struct sreflect sattr(
"cooker" : "3b8ca511-33d1-4db4-b805-00eea6a8d5e1"
)
GLTFTOOL_API SMeshCooker final : public SCooker
{
    bool Cook(SCookContext * ctx) override;
    uint32_t Version() override;
};
} // namespace sreflect
} // namespace sreflect