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
"serialize" : "json",
"importer" : "3b8ca511-33d1-4db4-b805-00eea6a8d5e1"
)
USDTOOL_API SUSDMeshImporter final : public SImporter
{
    // the SDF path of Mesh asset
    eastl::string path;
    void* Import(skr::io::RAMService*, const SAssetRecord* record) override { return nullptr; }
};
} // namespace sreflect
} // namespace sreflect