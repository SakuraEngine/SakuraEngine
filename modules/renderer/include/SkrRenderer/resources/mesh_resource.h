#pragma once
#include "SkrRenderer/module.configure.h"
#include "SkrRenderer/fwd_types.h"

#include <containers/string.hpp>
#include <containers/vector.hpp>

#ifndef __meta__
    #include "SkrRenderer/resources/mesh_resource.generated.h"
#endif

sreflect_enum("guid" : "01f05eb7-6d5d-46d8-945e-ce1259d22c8f")
sattr("rtti" : true, "serialize" : ["bin", "json"])
ESkrVertexAttribute SKR_IF_CPP(: uint32_t)
{
    SKR_VERT_ATTRIB_NONE,
    SKR_VERT_ATTRIB_POSITION,
    SKR_VERT_ATTRIB_NORMAL,
    SKR_VERT_ATTRIB_TANGENT,
    SKR_VERT_ATTRIB_TEXCOORD,
    SKR_VERT_ATTRIB_COLOR,
    SKR_VERT_ATTRIB_JOINTS,
    SKR_VERT_ATTRIB_WEIGHTS,
    SKR_VERT_ATTRIB_CUSTOM,
    SKR_VERT_ATTRIB_SIZE,
    SKR_VERT_ATTRIB_MAX_ENUM_BIT = UINT32_MAX,
};
typedef enum ESkrVertexAttribute ESkrVertexAttribute;

sreflect_struct("guid" : "3f01f94e-bd88-44a0-95e8-94ff74d18fca")
sattr("rtti" : true, "serialize" : "bin")
skr_vertex_buffer_entry_t
{
    ESkrVertexAttribute attribute;
    uint32_t attribute_index;
    uint32_t buffer_index;
    uint32_t stride;
    uint32_t offset;
};
typedef struct skr_vertex_buffer_entry_t skr_vertex_buffer_entry_t;

sreflect_struct("guid" : "6ac5f946-dd65-4710-8725-ab4273fe13e6")
sattr("rtti" : true, "serialize" : "bin")
skr_index_buffer_entry_t
{
    uint32_t buffer_index;
    uint32_t index_offset;
    uint32_t first_index;
    uint32_t index_count;
    uint32_t stride;
};
typedef struct skr_index_buffer_entry_t skr_index_buffer_entry_t;

sreflect_struct("guid" : "03104e51-c998-410b-9d3c-d76535933440")
sattr("rtti" : true, "serialize" : "bin")
skr_mesh_buffer_t
{
    uint32_t index;
    uint64_t byte_length;
    bool used_with_index;
    bool used_with_vertex;
    sattr("transient": true, "no-rtti" : true)
    skr_blob_t bin;
};

#ifdef __cplusplus
#include "resource/resource_factory.h"

namespace skr sreflect
{
namespace renderer sreflect
{
using EVertexAttribute = ESkrVertexAttribute;
using VertexBufferEntry = skr_vertex_buffer_entry_t;
using IndexBufferEntry = skr_index_buffer_entry_t;
using MeshBuffer = skr_mesh_buffer_t;

sreflect_struct("guid" : "b0b69898-166f-49de-a675-7b04405b98b1")
sattr("rtti" : true, "serialize" : "bin")
MeshPrimitive 
{
    skr_vertex_layout_id vertex_layout_id;
    uint32_t material_index;
    skr::vector<VertexBufferEntry> vertex_buffers;
    IndexBufferEntry index_buffer;
    uint32_t vertex_count;
};

sreflect_struct("guid" : "d3b04ea5-415d-44d5-995a-5c77c64fe1de")
sattr("rtti" : true, "serialize" : "bin")
MeshSection 
{
    int32_t parent_index;
    skr_float3_t translation;
    skr_float3_t scale;
    skr_float4_t rotation;
    skr::vector<uint32_t> primive_indices;
};

sreflect_struct("guid" : "3b8ca511-33d1-4db4-b805-00eea6a8d5e1") 
sattr("rtti" : true, "serialize" : "bin")
MeshResource
{
    SKR_RENDERER_API ~MeshResource() SKR_NOEXCEPT;
    
    skr::string name;
    skr::vector<MeshSection> sections;
    skr::vector<MeshPrimitive> primitives;
    skr::vector<MeshBuffer> bins;

    using material_handle_t = skr::resource::TResourceHandle<skr_material_resource_t>;
    skr::vector<material_handle_t> materials;
    
    bool install_to_vram SKR_IF_CPP(= true);
    bool install_to_ram SKR_IF_CPP(= true); // TODO: configure this in asset

    sattr("no-rtti": true,"transient": true)
    skr_render_mesh_id render_mesh SKR_IF_CPP(= nullptr);
};

struct SKR_RENDERER_API SMeshFactory : public resource::SResourceFactory {
    virtual ~SMeshFactory() = default;

    struct Root {
        skr_vfs_t* vfs = nullptr;
        const char8_t* dstorage_root;
        skr_io_ram_service_t* ram_service = nullptr;
        skr_io_vram_service_t* vram_service = nullptr;
        SRenderDeviceId render_device = nullptr;
    };

    float AsyncSerdeLoadFactor() override { return 2.5f; }
    [[nodiscard]] static SMeshFactory* Create(const Root& root);
    static void Destroy(SMeshFactory* factory); 
};
} // namespace renderer
} // namespace skr
#endif

SKR_RENDERER_EXTERN_C SKR_RENDERER_API void 
skr_mesh_resource_free(skr_mesh_resource_id mesh_resource);

SKR_RENDERER_EXTERN_C SKR_RENDERER_API void 
skr_mesh_resource_register_vertex_layout(skr_vertex_layout_id id, const char8_t* name, const struct CGPUVertexLayout* in_vertex_layout);

SKR_RENDERER_EXTERN_C SKR_RENDERER_API const char* 
skr_mesh_resource_query_vertex_layout(skr_vertex_layout_id id, struct CGPUVertexLayout* out_vertex_layout);