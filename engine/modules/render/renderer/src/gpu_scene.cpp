#include "SkrCore/log.h"
#include "SkrRuntime/ecs/world.hpp"
#include "SkrRenderGraph/backend/graph_backend.hpp"
#include "SkrRenderer/gpu_scene.h"
#include "SkrRenderer/render_device.h"
#include "SkrRenderer/render_mesh.h"
#include "SkrRenderer/shared/gpu_scene.hpp"
#include "SkrSceneCore/scene_components.h"
#include "SkrProfile/profile.h"
#include <atomic>

namespace skr
{
struct GPUSceneInstanceTask
{
    void build(skr::ecs::AccessBuilder& builder)
    {
        builder.write(&GPUSceneInstanceTask::instances);
    }
    skr::ecs::ComponentView<GPUSceneInstance> instances;
    skr::GPUScene* pScene = nullptr;
};

struct AddEntityToGPUScene : public GPUSceneInstanceTask
{
    void build(skr::ecs::AccessBuilder& builder)
    {
        GPUSceneInstanceTask::build(builder);
        builder.read(&AddEntityToGPUScene::transforms);
        builder.read(&AddEntityToGPUScene::meshes);
        builder.write(&AddEntityToGPUScene::instances);
        builder.write(&AddEntityToGPUScene::gpu_instances);
    }

    void run(skr::ecs::TaskContext& Context)
    {
        SkrZoneScopedN("GPUScene::AddEntityToGPUScene");

        sugoi_chunk_view_t v = {};
        sugoiS_access(pScene->ecs_world->get_storage(), (sugoi_entity_t)Context.entities()[0], &v);
        {
            for (auto i = 0; i < Context.size(); i++)
            {
                auto entity = Context.entities()[i];
                const auto& mesh = meshes[i];
                const bool MeshNotResolved = !mesh.mesh_resource.is_resolved();
                if (MeshNotResolved)
                {
                    pScene->AddEntity(entity);
                    continue;
                }

                auto mesh_resource = mesh.mesh_resource.get_resolved();
                if (mesh_resource->render_mesh->need_build_blas)
                {
                    pScene->dirty_blases.enqueue(mesh_resource->render_mesh->blas);
                    mesh_resource->render_mesh->need_build_blas = false;
                }

                const auto entity_transform = transforms[i].get().to_matrix();
                auto& multi_insts = gpu_instances[i];
                auto& instance_data = instances[i];
                instance_data.entity = entity;
                for (auto& gpu_inst : multi_insts)
                {
                   // 分配 instance id
                    if (pScene->free_insts.try_dequeue(gpu_inst.global_index))
                        pScene->free_inst_count -= 1;
                    else
                        gpu_inst.global_index = pScene->latest_inst_index++;

                    // 分配 prim ids
                    gpu_inst.primitives = gpu::Range<gpu::Primitive>(
                        mesh_resource->render_mesh->primitive_table_id_start,
                        (uint32_t)mesh_resource->primitives.size()
                    );

                    // add tlas
                    gpu_inst.transform = entity_transform * gpu_inst.transform;
                    const auto& transform = gpu_inst.transform;
                    auto& tlas_instance = pScene->tlas_instances[gpu_inst.global_index];
                    tlas_instance.bottom = mesh_resource->render_mesh->blas;
                    tlas_instance.instance_id = gpu_inst.global_index;
                    tlas_instance.instance_mask = 255;
                    float transform34[12] = {
                        transform.m00, transform.m10, transform.m20, transform.m30, // Row 0: X axis + X translation
                        transform.m01,
                        transform.m11,
                        transform.m21,
                        transform.m31, // Row 1: Y axis + Y translation
                        transform.m02,
                        transform.m12,
                        transform.m22,
                        transform.m32 // Row 2: Z axis + Z translation
                    };
                    memcpy(tlas_instance.transform, transform34, sizeof(transform34));

                    pScene->tlas_dirty = true;
                    pScene->total_inst_count += 1;
                }

                instance_data._ready_on_gpu = true;
            }
        }
    }
    skr::ecs::ComponentView<const MeshComponent> meshes;
    skr::ecs::ComponentView<GPUSceneInstance> instances;
    skr::ecs::ComponentView<gpu::Instance> gpu_instances;
    skr::ecs::ComponentView<const skr::scene::TransformComponent> transforms;
};

struct ScanGPUScene : public GPUSceneInstanceTask
{
    void build(skr::ecs::AccessBuilder& builder)
    {
        GPUSceneInstanceTask::build(builder);
        builder.read(&ScanGPUScene::gpu_instances);
    }
    void run(skr::ecs::TaskContext& Context)
    {
        SkrZoneScopedN("GPUScene::ScanGPUScene");

        const auto& Lane = pScene->GetLaneForUpload();
        sugoi_chunk_view_t v = {};
        sugoiS_access(pScene->ecs_world->get_storage(), (sugoi_entity_t)Context.entities()[0], &v);
        for (auto i = 0; i < Context.size(); i++)
        {
            auto entity = Context.entities()[i];
            const auto& instance_data = instances[i];
            auto& multi_insts = gpu_instances[i];
            for (auto& gpu_inst : multi_insts)
            {
                gpu::GPUDatablock<gpu::Instance>::StoreInstance(
                    *pScene->instance_table, 
                    gpu_inst.global_index, 
                    gpu_inst
                );
            }
        }
    }
    ScanGPUScene(skr::render_graph::RenderGraph* g)
        : graph(g)
    {
    }
    skr::render_graph::RenderGraph* graph;
    skr::ecs::ComponentView<const gpu::Instance> gpu_instances;
};

void GPUScene::AdjustDatabase(skr::render_graph::RenderGraph* graph)
{
    SkrZoneScopedN("GPUScene::AdjustBuffer");
    const auto& Lane = GetLaneForUpload();
    auto& frame_ctx = frame_ctxs.get(graph);

    const auto existed_instances = tlas_instances.size();
    if (free_inst_count < Lane.add_ents.size())
    {
        tlas_instances.resize_zeroed(tlas_instances.size() + Lane.add_ents.size());
    }
    const auto required_instances = tlas_instances.size();
    frame_ctx.instance_table_handle = instance_table->UpdateTableBuffer(graph, required_instances);
}

void GPUScene::ExecuteUpload(skr::render_graph::RenderGraph* graph)
{
    using namespace skr::render_graph;

    SkrZoneScopedN("GPUScene::ExecuteUpload");
    SwitchLane();

    // Reset Frame Resources
    auto& frame_ctx = frame_ctxs.get(graph);
    frame_ctx.frame_tlas = {};
    frame_ctx.tlas_handle = {};

    // Ensure buffers are sized correctly and import buffers to render graph
    AdjustDatabase(graph);

    auto ImportTLAS = [&frame_ctx, this, graph]() {
        // Import TLAS to RenderGraph if it exists
        frame_ctx.frame_tlas = tlas_manager->GetLatestTLAS(graph);
        frame_ctx.tlas_handle = {};
        if (frame_ctx.frame_tlas.get() != nullptr)
        {
            frame_ctx.tlas_handle = graph->create_acceleration_structure(
                [&frame_ctx, graph](RenderGraph& rg, class RenderGraph::AccelerationStructureBuilder& builder) {
                    builder.set_name(u8"GPUScene-TLAS")
                        .import(frame_ctx.frame_tlas.get());
                }
            );
        }
    };
    SKR_DEFER({ ImportTLAS(); });

    auto& Lane = GetLaneForUpload();

    // Schedule remove & add & scan
    auto get_batchsize = +[](uint64_t ecount) { return std::max(ecount / 8ull, 1024ull); };
    auto& upload_ctx = upload_ctxs.get(graph);
    {
        SkrZoneScopedN("GPUScene::Tasks");
        if (!Lane.remove_ents.is_empty())
        {
            SkrZoneScopedN("GPUScene::RemoveEntityFromGPUScene");
            for (auto to_remove : Lane.remove_ents)
            {
                RemoveEntity(to_remove);
            }
        }
        if (!Lane.add_ents.is_empty())
        {
            SkrZoneScopedN("GPUScene::AddEntityToGPUScene");
            skr::ecs::TaskOptions options;
            upload_ctx.add_finish.clear();
            options.on_finishes.add(upload_ctx.add_finish);

            AddEntityToGPUScene add;
            add.pScene = this;
            ecs_world->dispatch_task(add, get_batchsize(Lane.add_ents.size()), Lane.add_ents, std::move(options));
        }
        if (!Lane.dirty_ents.is_empty())
        {
            SkrZoneScopedN("GPUScene::ScanGPUScene");

            skr::ecs::TaskOptions options;
            upload_ctx.scan_finish.clear();
            options.on_finishes.add(upload_ctx.scan_finish);

            ScanGPUScene scan(graph);
            scan.pScene = this;
            Lane.tmp_dirty_ents.reserve(Lane.dirty_ents.size());
            Lane.tmp_dirty_ents.append(Lane.dirty_ents);
            ecs_world->dispatch_task(scan, get_batchsize(Lane.tmp_dirty_ents.size()), Lane.tmp_dirty_ents, std::move(options));
        }
    }

    instance_table->DispatchSparseUpload(graph, 
        [this](skr::render_graph::RenderGraph& g, skr::render_graph::ComputePassContext& ctx) {
        auto& upload_ctx = upload_ctxs.get(ctx.graph);
        upload_ctx.add_finish.wait(true);
        upload_ctx.scan_finish.wait(true);

        // Send BLAS / TLAS requests
        {
            SkrZoneScopedN("GPUScene::UpdateAccelerationStructure");
            TLASUpdateRequest update_request;
            {
                CGPUAccelerationStructureId blas = nullptr;
                while (dirty_blases.try_dequeue(blas) && blas)
                {
                    update_request.blases_to_build.add_unique(blas);
                }
            }
            if (tlas_dirty || !update_request.blases_to_build.is_empty())
            {
                if (total_inst_count != 0)
                {
                    CGPUAccelerationStructureDescriptor tlas_desc = {};
                    tlas_desc.type = CGPU_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
                    tlas_desc.flags = CGPU_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
                    tlas_desc.top.count = total_inst_count;
                    tlas_desc.top.instances = tlas_instances.data();
                    update_request.tlas_desc = tlas_desc;
                }
                tlas_manager->Request(ctx.graph, update_request);
                tlas_dirty = false;
            }
        }
        auto& Lane = GetLaneForUpload();
        {
            SkrZoneScopedN("GPUScene::CleanUpLane");

            Lane.add_ents.clear();
            Lane.remove_ents.clear();
            Lane.dirty_ents.clear();
            Lane.tmp_dirty_ents.clear();
        } });
}

void GPUScene::AddEntity(skr::ecs::Entity entity)
{
    auto& Lane = GetFrontLane();
    {
        Lane.add_mtx.lock();
        Lane.add_ents.add(entity);
        Lane.add_mtx.unlock();
    }
    RequireUpload(entity);
}

bool GPUScene::CanRemoveEntity(skr::ecs::Entity entity)
{
    const auto instance_data = ecs_world->random_readwrite<GPUSceneInstance>().get(entity);
    return instance_data->_ready_on_gpu;
}

void GPUScene::RemoveEntity(skr::ecs::Entity entity)
{
    auto& Lane = GetFrontLane();
    const auto instance_data = ecs_world->random_readwrite<GPUSceneInstance>().get(entity);
    const auto multi_insts = ecs_world->random_readwrite<skr::gpu::Instance>().get(entity);
    if (instance_data->_ready_on_gpu)
    {
        const auto prims = ecs_world->random_readwrite<gpu::Primitive>().get(entity);
        const auto mats = ecs_world->random_readwrite<gpu::Material>().get(entity);
        for (auto inst : *multi_insts)
        {
            free_insts.enqueue(inst.global_index);
            free_inst_count += 1;
            total_inst_count -= 1;

            auto& tlas_instance = tlas_instances[inst.global_index];
            memset(tlas_instance.transform, 0, sizeof(tlas_instance.transform));
            tlas_dirty = true;
        }
    }
    else
    {
        Lane.remove_mtx.lock();
        Lane.remove_ents.add(entity);
        Lane.remove_mtx.unlock();
    }
}

void GPUScene::RequireUpload(skr::ecs::Entity entity)
{
    auto& Lane = GetFrontLane();
    Lane.dirty_mtx.lock();
    Lane.dirty_ents.add(entity);
    Lane.dirty_mtx.unlock();
}

void GPUScene::Initialize(gpu::TableManager* table_manager, skr::RenderDevice* render_device, skr::ecs::ECSWorld* world)
{
    SKR_LOG_INFO(u8"Initializing GPUScene...");

    ecs_world = world;
    render_device = render_device;

    tlas_manager = TLASManager::Create(1 + RG_MAX_FRAME_IN_FLIGHT, render_device);
    {
        gpu::TableConfig table_builder(render_device->get_cgpu_device(), u8"Instances");
        table_builder.with_instances(16 * 1024);
        gpu::GPUDatablock<gpu::Instance>::SetupTableConfig(table_builder);
        instance_table = table_manager->CreateTable(table_builder);
    }
    SKR_LOG_INFO(u8"GPUScene initialized successfully");
}

void GPUScene::Shutdown()
{
    SKR_LOG_INFO(u8"Shutting down GPUScene...");

    // Shutdown allocators (will release buffers)
    instance_table.reset();

    for (uint32_t i = 0; i < frame_ctxs.max_frames_in_flight(); ++i)
    {
        auto& ctx = frame_ctxs[i];
        ctx.frame_tlas = {};
    }
    TLASManager::Destroy(tlas_manager);

    ecs_world = nullptr;
    render_device = nullptr;

    SKR_LOG_INFO(u8"GPUScene shutdown complete");
}

} // namespace skr