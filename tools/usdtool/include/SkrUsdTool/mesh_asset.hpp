#pragma once
#include "SkrUsdTool/module.configure.h"
#include "SkrToolCore/asset/importer.hpp"
#include "platform/configure.h"
#ifndef __meta__
#include "SkrUsdTool/mesh_asset.generated.h"
#endif

namespace skd sreflect
{
namespace asset sreflect
{
sreflect_struct("guid" : "01A1037A-22A1-46C9-8461-90B443796FDC", "serialize" : "json")
SKRUSDTOOL_API SUSDMeshImporter final : public SImporter
{
    // the SDF path of Mesh asset
    skr::string path;
    void* Import(skr_io_ram_service_t*, SCookContext* context) override { SKR_UNIMPLEMENTED_FUNCTION(); return nullptr; }
    void Destroy(void *) override { SKR_UNIMPLEMENTED_FUNCTION(); }
}
sregister_importer();
} // namespace asset
} // namespace skd