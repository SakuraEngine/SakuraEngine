#pragma once
#include "usdtool_configure.h"
#include "asset/importer.hpp"
#include "platform/configure.h"

namespace skd sreflect
{
namespace asset sreflect
{
struct sreflect sattr(
"guid" : "01A1037A-22A1-46C9-8461-90B443796FDC",
"serialize" : "json"
)
USDTOOL_API SUSDMeshImporter final : public SImporter
{
    // the SDF path of Mesh asset
    eastl::string path;
    void* Import(skr::io::RAMService*, SCookContext* context) override { SKR_UNIMPLEMENTED_FUNCTION(); return nullptr; }
    void Destroy(void *) override { SKR_UNIMPLEMENTED_FUNCTION(); }
}
sregister_importer();
} // namespace sreflect
} // namespace sreflect