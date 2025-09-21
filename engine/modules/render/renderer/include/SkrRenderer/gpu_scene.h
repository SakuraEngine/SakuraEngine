#pragma once
#include "SkrCore/id_range_allocator.hpp"
#include "SkrContainersDef/map.hpp"
#include "SkrGraphics/raytracing.h"
#include "SkrRuntime/ecs/world.hpp"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderGraph/frame_resource.hpp"
#include "SkrRenderer/render_device.h"
#include "SkrRenderer/graphics/gpu_table.hpp"
#include "SkrRenderer/graphics/tlas_manager.hpp"
#include "SkrRenderer/gpu_scene.generated.h" // IWYU pragma: export

namespace sugoi
{
struct archetype_t;
}
namespace skr
{

// 类型定义
using GPUSceneInstanceID = uint32_t;
using GPUArchetypeID = uint16_t;
using CPUTypeID = skr::ecs::TypeIndex;
static constexpr GPUSceneInstanceID INVALID_GPU_SCENE_INSTANCE_ID = 0xFFFFFFFF;

// D3D12 24位 InstanceID 限制
struct GPUSceneCustomIndex
{
    GPUSceneCustomIndex()
        : packed(0x00FFFFFF)
    {
    }
    GPUSceneCustomIndex(uint32_t index)
        : packed(index)
    {
        SKR_ASSERT(index <= 0x00FFFFFF);
    }
    uint32_t GetInstanceID() const { return packed & 0x00FFFFFF; }

private:
    uint32_t packed;
};

struct [[secs_managed_component, sattr(guid = "fd6cd47d-bb68-4d1c-bd26-ad3717f10ea7")]]
GPUSceneInstance
{
public:
    skr::ecs::Entity entity;
    std::atomic_bool _ready_on_gpu = false;
};

// 主管理器
struct SKR_RENDERER_API GPUScene final
{
public:
    void Initialize(gpu::TableManager* table_manager, skr::RenderDevice* render_device, skr::ecs::ECSWorld* world);
    void Shutdown();

    // TODO: ADD A TRACKER COMPONENT TO REPLACE THIS KIND OF API
    bool CanRemoveEntity(skr::ecs::Entity entity);

    void AddEntity(skr::ecs::Entity entity);
    void RemoveEntity(skr::ecs::Entity entity);

    void RequireUpload(skr::ecs::Entity entity);
    void ExecuteUpload(skr::render_graph::RenderGraph* graph);

    inline skr::ecs::ECSWorld* GetECSWorld() const { return ecs_world; }
    inline skr::render_graph::BufferHandle GetSceneBuffer(skr::render_graph::RenderGraph* graph) const
    {
        return frame_ctxs.get(graph).instance_table_handle;
    }
    skr::render_graph::AccelerationStructureHandle GetTLAS(skr::render_graph::RenderGraph* graph) const
    {
        return frame_ctxs.get(graph).tlas_handle;
    }
    inline uint32_t GetInstanceCount() const { return total_inst_count; }

protected:
    void AdjustDatabase(skr::render_graph::RenderGraph* graph);

private:
    friend struct GPUSceneInstanceTask;
    friend struct AddEntityToGPUScene;
    friend struct RemoveEntityFromGPUScene;
    friend struct ScanGPUScene;

    skr::ecs::ECSWorld* ecs_world = nullptr;
    skr::RenderDevice* render_device = nullptr;

    TLASManager* tlas_manager = nullptr;
    std::atomic_bool tlas_dirty = false;
    skr::ConcurrentQueue<CGPUAccelerationStructureId> dirty_blases;
    skr::Vector<CGPUAccelerationStructureInstanceDesc> tlas_instances;

    skr::RC<gpu::TableInstance> instance_table;
    skr::ConcurrentQueue<GPUSceneInstanceID> free_insts;
    std::atomic<GPUSceneInstanceID> free_inst_count = 0;
    std::atomic<GPUSceneInstanceID> latest_inst_index = 0;
    std::atomic<GPUSceneInstanceID> total_inst_count = 0;

private:
    static constexpr uint32_t kLaneCount = 2;
    struct UpdateLane
    {
        shared_atomic_mutex add_mtx;
        skr::Vector<skr::ecs::Entity> add_ents;

        shared_atomic_mutex remove_mtx;
        skr::Vector<skr::ecs::Entity> remove_ents;

        shared_atomic_mutex dirty_mtx;
        skr::Set<skr::ecs::Entity> dirty_ents;
        skr::Vector<skr::ecs::Entity> tmp_dirty_ents;
    } lanes[kLaneCount];
    std::atomic_uint32_t front_lane = 0;

    UpdateLane& GetFrontLane() { return lanes[front_lane]; }
    UpdateLane& GetLaneForUpload() { return lanes[(front_lane + kLaneCount - 1) % kLaneCount]; }
    void SwitchLane() { front_lane = (front_lane + kLaneCount + 1) % kLaneCount; }

    // upload buffer management
    struct UploadContext
    {
        skr::task::event_t add_finish = skr::task::event_t(true);
        skr::task::event_t scan_finish = skr::task::event_t(true);
    };
    skr::render_graph::FrameResource<UploadContext> upload_ctxs;

    // Buffer with FrameResource for deferred destruction
    struct FrameContext
    {
        FrameContext() = default;

        // Delete copies.
        FrameContext(FrameContext const&) = delete;
        FrameContext& operator=(FrameContext const&) = delete;

        // Single buffer to discard when this frame comes around again
        TLASHandle frame_tlas;
        skr::render_graph::AccelerationStructureHandle tlas_handle;
        skr::render_graph::BufferHandle instance_table_handle;
    };
    skr::render_graph::FrameResource<FrameContext> frame_ctxs;
};

} // namespace skr