#pragma once
#include "render-context.hpp"
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
};

struct RenderPrimitive {
    uint32_t first_index_;
    uint32_t index_count_;
    eastl::shared_ptr<RenderMaterial> material_;
};

struct RenderMesh {
    friend class RenderScene;
    eastl::string name_;
    eastl::vector<RenderPrimitive> primitives_;

protected:
    int32_t loadPrimitive(struct cgltf_primitive* src, uint32_t& index_cursor);
};

class RenderScene
{
public:
    void Initialize(const char8_t* path);
    void Upload(RenderContext* context, bool keep_gltf_data = false);
    void Destroy();

    struct cgltf_data* gltf_data_ = nullptr;
    eastl::vector<RenderNode> nodes_;
    uint32_t root_node_index_;
    eastl::vector<RenderMesh> meshes_;

    std::atomic_bool load_ready_ = false;
    std::atomic_bool gpu_memory_ready = false;
    CGpuSemaphoreId gpu_geometry_semaphore;
    CGpuFenceId gpu_geometry_fence;

    eastl::vector<CGpuBufferId> vertex_buffers_;
    CGpuBufferId index_buffer_;
    CGpuBufferId staging_buffer_;

protected:
    int32_t loadNode(struct cgltf_node* src, int32_t parent_idx);
    int32_t loadMesh(struct cgltf_mesh* src);
};
