#pragma once
#include "SkrBase/config.h" // IWYU pragma: export
#include "SkrBase/types.h"  // IWYU pragma: export
#include "SkrBase/meta.h"   // IWYU pragma: export
#include "SkrRuntime/ecs/component.hpp" // IWYU pragma: export

SKR_DECLARE_TYPE_ID_FWD(skr::io, IRAMService, skr_io_ram_service);
SKR_DECLARE_TYPE_ID_FWD(skr::io, IVRAMService, skr_io_vram_service);
namespace skr
{
enum class EShaderOptionType : uint32_t;
struct RenderDevice;
} // namespace skr

namespace skr
{
template <class T>
struct AsyncResource;
}

namespace skr
{
struct ShaderOptionInstance;
struct ShaderOptionTemplate;
struct ShaderOptionsResource;
struct MultiShaderResource;
struct ShaderOptionSequence;
struct ShaderCollectionResource;
struct ShaderCollectionJSON;
struct MaterialProperty;
struct MaterialValue;
struct MaterialPass;
struct MaterialTypeResource;
struct MaterialOverrides;
struct MaterialShaderVariant;
struct MaterialValueBool;
struct MaterialValueFloat;
struct MaterialValueDouble;
struct MaterialValueFloat2;
struct MaterialValueFloat3;
struct MaterialValueFloat4;
struct MaterialValueTexture;
struct MaterialValueSampler;
struct MaterialResource;
struct MeshPrimitive;
struct MeshResource;
struct MeshSection;
struct RenderMesh;
struct PrimitiveCommand;

struct StableShaderHash;
struct PlatformShaderHash;
struct PlatformShaderIdentifier;
struct ShaderMap;
typedef skr_guid_t VertexLayoutId;
} // namespace skr

typedef struct skr::RenderDevice SRenderDevice;
class SkrRendererModule;

typedef SRenderDevice* SRenderDeviceId;

SKR_DECLARE_TYPE_ID_FWD(skr, PSOMapKey, skr_pso_map_key);

typedef struct skr_pso_map_t* skr_pso_map_id;
typedef struct skr_pso_map_root_t skr_pso_map_root_t;

using skr_shader_resource_handle_t = skr::AsyncResource<skr::MultiShaderResource>;
using skr_material_type_handle_t = skr::AsyncResource<skr::MaterialTypeResource>;