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
    CGpuDescriptorSetId desc_set_;
    // considering some status like wireframe_mode, fetch pipeline with RenderBlackboard::GetRenderPipeline
    // when recording drawcalls may be better
    AsyncRenderPipeline* async_ppl_;
    // set by aux thread callback
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
    // On AuxThread and not stuck the calling thread
    void AsyncCreateRenderPipelines(RenderContext* context, struct RenderAuxThread* aux_thread);
    // On AuxThread and not stuck the calling thread
    void AsyncCreateGPUMemory(RenderContext* context, struct RenderAuxThread* aux_thread);
    bool AsyncUploadReady();
    void Upload(RenderContext* context, struct RenderAuxThread* aux_thread);
    void Destroy(struct RenderAuxThread* aux_thread = nullptr);

    eastl::vector<RenderNode> nodes_;
    uint32_t root_node_index_;
    eastl::vector<RenderMesh> meshes_;
    eastl::vector_map<eastl::string, RenderMaterial> materials_;

    std::atomic_bool bufs_creation_ready_ = false;
    std::atomic_uint32_t bufs_creation_counter_ = 0;
    std::atomic_bool bufs_upload_started_ = false;
    std::atomic_bool bufs_upload_ready_ = false;

    RenderContext* context_ = nullptr;
    CGpuSemaphoreId gpu_geometry_semaphore = nullptr;
    CGpuFenceId gpu_geometry_fence = nullptr;

    AsyncRenderBuffer* vertex_buffers_;
    uint32_t vertex_buffer_count_ = 0;
    AsyncRenderBuffer index_buffer_;
    uint32_t index_stride_;
    CGpuBufferId staging_buffer_;

protected:
    int32_t loadNode(struct cgltf_node* src, int32_t parent_idx);
    int32_t loadMesh(struct cgltf_mesh* src);
    int32_t loadMaterial(struct cgltf_material* src);

    struct cgltf_data* gltf_data_ = nullptr;
    eastl::vector_map<struct cgltf_buffer_view*, uint32_t> viewVBIdxMap = {};
};
