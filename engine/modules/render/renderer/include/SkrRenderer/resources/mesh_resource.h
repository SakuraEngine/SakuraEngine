#pragma once
#include "SkrBase/meta.h"
#include "SkrGraphics/api.h"
#include "SkrCore/blob.hpp"
#include "SkrContainers/string.hpp"
#include "SkrContainers/vector.hpp"
#include "SkrContainers/map.hpp"
#include "SkrRuntime/resource/resource_factory.hpp"
#include "SkrRenderer/graphics/gpu_table.hpp"
#include "SkrRenderer/resources/mesh_resource.generated.h" // IWYU pragma: export

namespace skr
{
enum class [[sattr(
    guid = "01f05eb7-6d5d-46d8-945e-ce1259d22c8f"
    serde = @enable
)]] EVertexAttribute : uint32_t
{
    NONE,
    POSITION,
    NORMAL,
    TANGENT,
    TEXCOORD,
    COLOR,
    JOINTS,
    WEIGHTS,
    CUSTOM,
    SIZE,
    MAX_ENUM_BIT = UINT32_MAX,
};

struct [[sattr(guid = "3f01f94e-bd88-44a0-95e8-94ff74d18fca" serde = @enable)]]
VertexBufferEntry
{
    EVertexAttribute attribute;
    uint32_t attribute_index;
    uint32_t buffer_index;
    uint32_t vertex_count;
    uint32_t stride;
    uint32_t offset;
};

struct [[sattr(guid = "6ac5f946-dd65-4710-8725-ab4273fe13e6" serde = @enable)]]
IndexBufferEntry
{
    uint32_t buffer_index;
    uint32_t index_offset;
    uint32_t first_index;
    uint32_t index_count;
    uint32_t stride;
};

struct [[sattr(guid = "03104e51-c998-410b-9d3c-d76535933440" serde = @enable)]]
MeshBuffer
{
    uint32_t index;
    uint64_t byte_length;
    bool used_with_index;
    bool used_with_vertex;
    [[sattr(serde = @disable)]]
    skr::RC<skr::IBlob> blob = nullptr;
};

struct [[sattr(guid = "cd2d43a7-1e0e-4951-bf87-7d693fd26227" serde = @enable)]]
MeshPrimitive
{
    skr::Vector<VertexBufferEntry> vertex_buffers;
    IndexBufferEntry index_buffer;
    uint32_t vertex_count;
    uint32_t material_index;
};

struct [[sattr(guid = "d3b04ea5-415d-44d5-995a-5c77c64fe1de" serde = @enable)]]
MeshSection
{
    int32_t parent_index;
    skr::float3 translation;
    skr::float3 scale;
    skr::float4 rotation;
    skr::Vector<uint32_t> primitive_indices;
};

struct [[sattr(guid = "3b8ca511-33d1-4db4-b805-00eea6a8d5e1" serde = @enable)]]
SKR_RENDERER_API MeshResource
{
    ~MeshResource() SKR_NOEXCEPT;

    skr::String name;
    skr::Vector<MeshSection> sections;
    skr::Vector<MeshPrimitive> primitives;
    skr::Vector<MeshBuffer> bins;

    skr::Vector<skr::AsyncResource<MaterialResource>> materials;

    bool install_to_vram SKR_IF_CPP(= true);
    bool install_to_ram SKR_IF_CPP(= false);

    [[sattr(serde = @disable)]]
    RenderMesh* render_mesh SKR_IF_CPP(= nullptr);
};

struct [[secs_component, sattr(
    guid = "c66ab7ef-bde9-4e0f-8023-a2d99ba5134c"
    serde = @enable
)]] SKR_RENDERER_API MeshComponent
{
public:
    SKR_GENERATE_BODY(MeshComponent);

    void SetMeshResource(skr::AsyncResource<skr::MeshResource> mesh);
    skr::AsyncResource<skr::MeshResource>& GetMeshResource();
    const skr::AsyncResource<skr::MeshResource>& GetMeshResource() const;

    void SetOverrideMaterial(uint32_t slot, skr::AsyncResource<MaterialResource> mat);
    skr::AsyncResource<MaterialResource> GetOverrideMaterial(uint32_t slot);

private:
    skr::AsyncResource<skr::MeshResource> mesh_resource;
    skr::InlineMap<uint32_t, skr::AsyncResource<MaterialResource>, 1> override_materials;
};

struct SKR_RENDERER_API MeshFactory : public ResourceFactory
{
    virtual ~MeshFactory() = default;

    struct Root
    {
        skr_vfs_t* vfs = nullptr;
        const char8_t* dstorage_root;
        io::IRAMService* ram_service = nullptr;
        io::IVRAMService* vram_service = nullptr;
        SRenderDeviceId render_device = nullptr;
        skr::RC<gpu::TableManager> table_manager = nullptr;
    };

    virtual skr::RG::BufferHandle UpdateGPUTable(skr::RG::RenderGraph* graph) = 0;
    virtual CGPUDescriptorBufferId descriptor_buffer() = 0;
    virtual skr::RC<gpu::TableInstance> primitive_table() = 0;

    float AsyncSerdeLoadFactor() override { return 2.5f; }
    [[nodiscard]] static MeshFactory* Create(const Root& root);
    static void Destroy(MeshFactory* factory);
};

} // namespace skr

SKR_EXTERN_C SKR_RENDERER_API void
skr_mesh_resource_free(skr::MeshResource* mesh_resource);

SKR_EXTERN_C SKR_RENDERER_API void
skr_mesh_resource_register_vertex_layout(skr::VertexLayoutId id, const char8_t* name, const struct CGPUVertexLayout* in_vertex_layout);

SKR_EXTERN_C SKR_RENDERER_API const char*
skr_mesh_resource_query_vertex_layout(skr::VertexLayoutId id, struct CGPUVertexLayout* out_vertex_layout);