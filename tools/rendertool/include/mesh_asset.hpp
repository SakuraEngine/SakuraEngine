#pragma once

#include "rendertool_configure.h"
#include "asset/importer.hpp"
#include "platform/configure.h"

namespace game sreflect
{
namespace asset sreflect
{
using namespace skd::asset;
struct sreflect sattr(
"guid" : "D72E2056-3C12-402A-A8B8-148CB8EAB922",
"serialize" : "json",
"importer" : "3b8ca511-33d1-4db4-b805-00eea6a8d5e1"
)
RENDERTOOL_API SGltfMeshImporter final : public SImporter
{
    using SImporter::SImporter;
    void* Import(skr::io::RAMService*, const SAssetRecord* record) override { return nullptr; }
};

struct sreflect sattr(
"cooker" : "3b8ca511-33d1-4db4-b805-00eea6a8d5e1"
)
RENDERTOOL_API SMeshCooker final : public SCooker
{
    bool Cook(SCookContext * ctx) override { return false; }
    uint32_t Version() override { return 0; }
};
} // namespace sreflect
} // namespace sreflect