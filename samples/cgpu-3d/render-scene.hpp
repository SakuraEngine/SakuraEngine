#pragma once
#include "render-context.hpp"
#include "render-resources.hpp"
#include "math/vectormath.hpp"
#include <atomic>
#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <EASTL/shared_ptr.h>

struct RenderNode {
    RenderNode* parent_;
    eastl::vector<RenderNode*> children_;
    int32_t index_;
    struct RenderMesh* mesh_ = nullptr;
    sakura::math::Vector3f translation_ = {};
    sakura::math::Vector3f scale_ = { 1.f, 1.f, 1.f };
    sakura::math::Quaternion rotation_ = {};
};

struct RenderMaterial {
    eastl::string base_color_uri_;
    uint32_t id_in_scene_;
};

struct RenderPrimitive {
    uint32_t index_offset_;
    uint32_t first_index_;
    uint32_t index_count_;
    uint32_t vertex_layout_id_;
    uint32_t material_id_;
    CGpuRenderPipelineId pipeline_;
    CGpuDescriptorSetId desc_set_;
    eastl::vector<CGpuBufferId> vertex_buffers_;
    eastl::vector<uint32_t> vertex_strides_;
    eastl::vector<uint32_t> vertex_offsets_;
};

struct RenderMesh {
    friend class RenderScene;
    eastl::string name_;
    eastl::vector<RenderPrimitive> primitives_;
    class RenderScene* scene_;

protected:
    int32_t loadPrimitive(struct cgltf_primitive* src, uint32_t& index_cursor);
};

class RenderScene
{
public:
    void Initialize(const char8_t* path);
    void Upload(RenderContext* context, struct RenderAuxThread* aux_thread, bool keep_gltf_data = false);
    void Destroy(struct RenderAuxThread* aux_thread);

    struct cgltf_data* gltf_data_ = nullptr;
    eastl::vector<RenderNode> nodes_;
    uint32_t root_node_index_;
    eastl::vector<RenderMesh> meshes_;
    eastl::vector_map<eastl::string, RenderMaterial> materials_;

    std::atomic_bool load_ready_ = false;
    std::atomic_bool gpu_memory_ready = false;

    RenderContext* context_;
    CGpuSemaphoreId gpu_geometry_semaphore;
    CGpuFenceId gpu_geometry_fence;

    RenderBuffer* vertex_buffers_;
    uint32_t vertex_buffer_count_ = 0;
    RenderBuffer index_buffer_;
    uint32_t index_stride_;
    CGpuBufferId staging_buffer_;

protected:
    int32_t loadNode(struct cgltf_node* src, int32_t parent_idx);
    int32_t loadMesh(struct cgltf_mesh* src);
    int32_t loadMaterial(struct cgltf_material* src);
};
