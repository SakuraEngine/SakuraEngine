#pragma once
#include "SkrRenderer/module.configure.h"
#include "utils/types.h"
#ifndef __meta__
    #include "SkrRenderer/resources/mesh_resource.generated.h"
#endif

typedef skr_guid_t skr_vertex_layout_id;

sreflect_struct("guid" : "3f01f94e-bd88-44a0-95e8-94ff74d18fca")
sattr("serialize" : "bin")
skr_vertex_buffer_entry_t
{
    uint32_t buffer_index;
    uint32_t stride;
    uint32_t offset;
};
typedef struct skr_vertex_buffer_entry_t skr_vertex_buffer_entry_t;

sreflect_struct("guid" : "6ac5f946-dd65-4710-8725-ab4273fe13e6")
sattr("serialize" : "bin")
skr_index_buffer_entry_t
{
    uint32_t buffer_index;
    uint32_t index_offset;
    uint32_t first_index;
    uint32_t index_count;
    uint32_t stride;
};
typedef struct skr_index_buffer_entry_t skr_index_buffer_entry_t;

#ifdef __cplusplus
#include <EASTL/vector.h>
#include <EASTL/string.h>

sreflect_struct("guid" : "b0b69898-166f-49de-a675-7b04405b98b1")
sattr("serialize" : "bin")
skr_mesh_primitive_t 
{
    skr_vertex_layout_id vertex_layout_id;
    skr_guid_t material_inst;
    eastl::vector<skr_vertex_buffer_entry_t> vertex_buffers;
    struct skr_index_buffer_entry_t index_buffer;
};

sreflect_struct("guid" : "d3b04ea5-415d-44d5-995a-5c77c64fe1de")
sattr("serialize" : "bin")
skr_mesh_section_t 
{
    int32_t parent_index;
    skr_float3_t translation;
    skr_float3_t scale;
    skr_float4_t rotation;
    eastl::vector<uint32_t> primive_indices;
};

sreflect_struct("guid" : "03104e51-c998-410b-9d3c-d76535933440")
sattr("serialize" : "bin")
skr_mesh_buffer_t
{
    uint32_t index;
    uint64_t byte_length;
    bool used_with_index;
    bool used_with_vertex;
    sattr("transient": true)
    skr_blob_t bin;
    // TODO: keep this?
    eastl::string uri; // gltf buffer uri
};

sreflect_struct("guid" : "3b8ca511-33d1-4db4-b805-00eea6a8d5e1") 
sattr("serialize" : "bin")
skr_mesh_resource_t
{
    eastl::string name;
    eastl::vector<skr_mesh_section_t> sections;
    eastl::vector<skr_mesh_primitive_t> primitives;
    eastl::vector<skr_mesh_buffer_t> bins;
    sattr("transient": true)
    void* gltf_data;
};
#endif
typedef struct skr_mesh_buffer_t skr_mesh_buffer_t;
typedef struct skr_mesh_primitive_t skr_mesh_primitive_t;
typedef struct skr_mesh_section_t skr_mesh_section_t;
typedef struct skr_mesh_resource_t skr_mesh_resource_t;
typedef struct skr_mesh_resource_t* skr_mesh_resource_id;

#ifdef __cplusplus
#include "resource/resource_factory.h"

namespace skr sreflect
{
namespace resource sreflect
{
// - dstorage & bc: dstorage
// - dstorage & bc & zlib: dstorage with custom decompress queue
// - bc & zlib: [TODO] ram service & decompress service & upload
//    - upload with copy queue
//    - upload with gfx queue
struct SKR_RENDERER_API SMeshFactory : public SResourceFactory {
    virtual ~SMeshFactory() = default;

    struct Root {
        uint32_t __nothing__;
    };

    [[nodiscard]] static SMeshFactory* Create(const Root& root);
    static void Destroy(SMeshFactory* factory); 
};
} // namespace resource
} // namespace skr
#endif

#include "utils/io.h"


SKR_RENDERER_EXTERN_C SKR_RENDERER_API void 
skr_mesh_resource_free(skr_mesh_resource_id mesh_resource);

SKR_RENDERER_EXTERN_C SKR_RENDERER_API void 
skr_mesh_resource_register_vertex_layout(skr_vertex_layout_id id, const char* name, const struct CGPUVertexLayout* in_vertex_layout);

SKR_RENDERER_EXTERN_C SKR_RENDERER_API const char* 
skr_mesh_resource_query_vertex_layout(skr_vertex_layout_id id, struct CGPUVertexLayout* out_vertex_layout);


// : RAW GLTF METHODS :

typedef void (*skr_async_gltf_io_callback_t)(struct skr_gltf_ram_io_request_t* request, void* data);
typedef struct skr_gltf_ram_io_request_t {
    struct skr_vfs_t* vfs_override;
    skr_vertex_layout_id shuffle_layout SKR_IF_CPP( = {});
    bool load_bin_to_memory;
    skr_async_io_request_t ioRequest;
    SAtomic32 gltf_status;
    skr_mesh_resource_id mesh_resource;
    skr_async_gltf_io_callback_t callback;
    void* callback_data;
#ifdef __cplusplus
    bool is_ready() const SKR_NOEXCEPT
    {
        return get_status() == SKR_ASYNC_IO_STATUS_OK;
    }
    SkrAsyncIOStatus get_status() const SKR_NOEXCEPT
    {
        return (SkrAsyncIOStatus)skr_atomic32_load_acquire(&gltf_status);
    }
#endif
} skr_gltf_ram_io_request_t;

#ifndef SKR_SERIALIZE_GURAD
SKR_RENDERER_EXTERN_C SKR_RENDERER_API void 
skr_mesh_resource_create_from_gltf(skr_io_ram_service_t* ioService, const char* path, skr_gltf_ram_io_request_t* request);
#endif