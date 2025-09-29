#pragma once
#include "SkrToolCore/cook_system/asset_meta.hpp"
#include "SkrMeshCore/mesh_encoder.hpp" // IWYU pragma: export
#include "SkrMeshCore/mesh_asset.generated.h" // IWYU pragma: export

namespace skr
{

struct [[sattr(
    guid = "9A2C9CBF-517D-4197-BDE3-E40D85D88320" serde = @enable
)]] MeshAsset : public skr::AssetMetadata
{
    skr::GUID vertexType;
    skr::Vector<skr::GUID> materials;
    bool install_to_ram = false;
    bool install_to_vram = true;
};

} // namespace skr