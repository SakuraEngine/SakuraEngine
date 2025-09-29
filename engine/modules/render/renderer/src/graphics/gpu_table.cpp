#include "SkrRenderGraph/backend/graph_backend.hpp"
#include "SkrRenderer/graphics/gpu_table.hpp"
#include <utility>

namespace skr::gpu
{

TableConfig::TableConfig(CGPUDeviceId device, const char8_t* name)
    : device_(device)
    , name(name)
{
}

TableConfig& TableConfig::with_instances(uint32_t _initial_instances)
{
    initial_instances = _initial_instances;
    return *this;
}

TableConfig& TableConfig::with_page_size(uint32_t _page_size)
{
    page_size = _page_size;
    return *this;
}

TableConfig& TableConfig::add_component(TableSegmentID type_id, uint32_t element_size, uint32_t element_align)
{
    // 创建独立的Segment
    TableSegment segment;
    segment.stride = element_size;
    segment.align = element_align;
    segment.buffer_offset = 0; // 将在calculate_layout中计算

    uint32_t segment_index = segments_.size();
    segments_.add(segment);

    return *this;
}

TableInstanceBase::~TableInstanceBase()
{
    releaseBuffer();

    capacity_bytes = 0;
    instance_capacity = 0;
    config.segments_.clear();
}

TableInstanceBase::TableInstanceBase(const TableManager* manager, const TableConfig& cfg)
    : manager(manager)
    , config(cfg)
{
    // 根据布局类型确定实际容量
    if (config.page_size == UINT32_MAX)
    {
        // 连续布局：直接使用请求的容量
        instance_capacity = config.initial_instances;
        SKR_LOG_INFO(u8"TableInstanceBase: Continuous layout, capacity = %u instances", instance_capacity);
    }
    else
    {
        // 分页布局：容量必须是页大小的整数倍
        uint32_t page_count = (config.initial_instances + config.page_size - 1) / config.page_size;
        instance_capacity = page_count * config.page_size;
        SKR_LOG_INFO(u8"TableInstanceBase: Paged layout, requested = %u, page_size = %u, page_count = %u, capacity = %u instances",
            config.initial_instances,
            config.page_size,
            page_count,
            instance_capacity);
    }

    calculateLayout();
    createBuffer();
}

void TableInstanceBase::calculateLayout()
{
    uint32_t offset = 0;

    if (config.page_size == UINT32_MAX)
    {
        // 连续布局（传统 SOA）
        for (auto& segment : config.segments_)
        {
            // 对齐
            offset = (offset + segment.align - 1) & ~(segment.align - 1);
            segment.buffer_offset = offset;
            offset += segment.stride * instance_capacity;
        }

        capacity_bytes = offset;
    }
    else
    {
        // Paged SOA
        // 基于 instance_capacity 计算需要的页数
        uint32_t page_count = (instance_capacity + config.page_size - 1) / config.page_size;

        // 计算每页的字节大小
        page_stride_bytes = 0;
        for (const auto& segment : config.segments_)
        {
            page_stride_bytes = (page_stride_bytes + segment.align - 1) & ~(segment.align - 1);
            page_stride_bytes += segment.stride * config.page_size;
        }
        // page_stride_bytes = (page_stride_bytes + 255) & ~255; // 对齐 256 字节

        // 设置每个Segment在页内的偏移
        offset = 0;
        for (auto& segment : config.segments_)
        {
            offset = (offset + segment.align - 1) & ~(segment.align - 1);
            segment.buffer_offset = offset; // 页内偏移

            offset += segment.stride * config.page_size;
        }

        // 总容量字节数
        capacity_bytes = page_stride_bytes * page_count;
    }
}

uint32_t TableInstanceBase::GetComponentSize(TableSegmentID component_id) const
{
    return config.segments_[component_id].stride;
}

uint32_t TableInstanceBase::GetComponentOffset(TableSegmentID component_id, TableInstanceBaseID instance_id) const
{
    // 查找组件映射
    const auto& segment = config.segments_[component_id];

    if (config.page_size == UINT32_MAX)
    {
        // 连续布局
        // 地址 = Segment基址 + 实例偏移 * Segment步长 + 组件在Segment内的偏移
        return segment.buffer_offset + (instance_id * segment.stride);
    }
    else
    {
        // 分页布局
        uint32_t page = instance_id / config.page_size;
        uint32_t offset_in_page = instance_id % config.page_size;
        uint32_t page_start = page * page_stride_bytes;

        // 地址 = 页起始 + Segment在页内基址 + 页内实例偏移 * Segment步长 + 组件在Segment内的偏移
        uint32_t result = page_start + segment.buffer_offset + (offset_in_page * segment.stride);

        return result;
    }
    SKR_LOG_WARN(u8"TableInstanceBase: Component %u not found", component_id);
    return 0;
}

skr::RG::BufferHandle TableInstanceBase::UpdateTableBuffer(skr::RG::RenderGraph* graph, uint64_t required_instances)
{
    SkrZoneScopedN("GPUScene::AdjustBuffer");
    auto& frame_ctx = frame_ctxs.get(graph);

    // First, clean up any buffer marked for discard from previous frame
    if (frame_ctx.buffer_to_discard != nullptr)
    {
        cgpu_free_buffer(frame_ctx.buffer_to_discard);
        frame_ctx.buffer_to_discard = nullptr;
    }

    if (needsResize(required_instances))
    {
        SKR_LOG_INFO(u8"GPUScene: GPUScene data resize needed. Current: %u/%u instances", required_instances, GetInstanceCapacity());

        // Calculate new capacity
        const uint32_t old_capacity = instance_capacity;
        const uint64_t old_capacity_bytes = capacity_bytes;
        const uint32_t new_capacity = static_cast<uint32_t>(required_instances * 1.5f);

        // Resize and get old buffer (no blocking sync needed now)
        auto old_buffer = Resize(new_capacity);
        frame_ctx.buffer_handle = importBuffer(graph, config.name.c_str());

        if (old_buffer && GetBuffer())
        {
            // Import old buffer for copy source
            auto old_buffer_handle = graph->create_buffer(
                [=](skr::RG::RenderGraph& g, skr::RG::BufferBuilder& builder) {
                    builder.set_name(u8"old_scene_buffer")
                        .import(old_buffer, CGPU_RESOURCE_STATE_SHADER_RESOURCE);
                });
            // Copy data from old to new buffer
            CopySegments(graph, old_buffer_handle, frame_ctx.buffer_handle, old_capacity_bytes);

            // Mark old buffer for deferred destruction (will be freed next time this frame slot comes around)
            // Unlike TLAS, we don't reuse the old buffer - we always discard it after copy
            frame_ctx.buffer_to_discard = old_buffer;
        }
    }
    else
    {
        // Import scene buffer with proper state management
        frame_ctx.buffer_handle = importBuffer(graph, config.name.c_str());
    }

    return frame_ctx.buffer_handle;
}

skr::RG::BufferHandle TableInstanceBase::UpdateTableBufferStructured(skr::RG::RenderGraph* graph, uint64_t required_instances)
{
    SkrZoneScopedN("GPUScene::AdjustBuffer");
    auto& frame_ctx = frame_ctxs.get(graph);

    // First, clean up any buffer marked for discard from previous frame
    if (frame_ctx.buffer_to_discard != nullptr)
    {
        cgpu_free_buffer(frame_ctx.buffer_to_discard);
        frame_ctx.buffer_to_discard = nullptr;
    }

    if (needsResize(required_instances))
    {
        SKR_LOG_INFO(u8"GPUScene: GPUScene data resize needed. Current: %u/%u instances", required_instances, GetInstanceCapacity());

        // Calculate new capacity
        const uint32_t old_capacity = instance_capacity;
        const uint64_t old_capacity_bytes = capacity_bytes;
        const uint32_t new_capacity = static_cast<uint32_t>(required_instances * 1.5f);

        // Resize and get old buffer (no blocking sync needed now)
        auto old_buffer = Resize(new_capacity);
        frame_ctx.buffer_handle = importBufferStructured(graph, config.name.c_str());

        if (old_buffer && GetBuffer())
        {
            // Import old buffer for copy source
            auto old_buffer_handle = graph->create_buffer(
                [=](skr::RG::RenderGraph& g, skr::RG::BufferBuilder& builder) {
                    builder.set_name(u8"old_scene_buffer")
                        .import(old_buffer, CGPU_RESOURCE_STATE_SHADER_RESOURCE);
                });
            // Copy data from old to new buffer
            CopySegments(graph, old_buffer_handle, frame_ctx.buffer_handle, old_capacity_bytes);

            // Mark old buffer for deferred destruction (will be freed next time this frame slot comes around)
            // Unlike TLAS, we don't reuse the old buffer - we always discard it after copy
            frame_ctx.buffer_to_discard = old_buffer;
        }
    }
    else
    {
        // Import scene buffer with proper state management
        frame_ctx.buffer_handle = importBufferStructured(graph, config.name.c_str());
    }

    return frame_ctx.buffer_handle;
}

bool TableInstanceBase::needsResize(uint32_t required_instances) const
{
    return required_instances > instance_capacity;
}

CGPUBufferId TableInstanceBase::Resize(uint32_t new_instance_capacity)
{
    // 对于分页布局，确保容量是页大小的整数倍
    if (config.page_size != UINT32_MAX)
    {
        uint32_t page_count = (new_instance_capacity + config.page_size - 1) / config.page_size;
        new_instance_capacity = page_count * config.page_size;
    }

    if (new_instance_capacity <= instance_capacity)
    {
        SKR_LOG_WARN(u8"TableInstanceBase: New capacity %u not greater than current %u",
            new_instance_capacity,
            instance_capacity);
        return nullptr;
    }

    // 保存旧 buffer
    CGPUBufferId old_buffer = buffer;

    // 新 buffer 需要重新开始状态跟踪
    state_tracker.reset();

    // 更新容量并重新计算布局
    uint32_t old_instance_capacity = instance_capacity;
    instance_capacity = new_instance_capacity;

    // 重新计算布局
    calculateLayout();

    // 创建新 buffer
    buffer = createBufferWithCapacity(new_instance_capacity);

    SKR_LOG_INFO(u8"TableInstanceBase: Resized from %u to %u instances, new buffer size: %llu MB",
        old_instance_capacity,
        new_instance_capacity,
        capacity_bytes / (1024 * 1024));

    return old_buffer;
}

void TableInstanceBase::CopySegments(skr::RG::RenderGraph* graph,
    skr::RG::BufferHandle src_buffer,
    skr::RG::BufferHandle dst_buffer,
    uint64_t old_buffer_size) const
{
    if (old_buffer_size == 0)
        return;

    graph->add_copy_pass(
        [=, this](skr::RG::RenderGraph& g, skr::RG::CopyPassBuilder& builder) {
            builder.set_name(skr::format(u8"TableInstanceBase_Copy-{}", (uint64_t)this).c_str());

            builder.buffer_to_buffer(
                src_buffer.range(0, old_buffer_size),
                dst_buffer.range(0, old_buffer_size));
        },
        {});
}

static constexpr uint64_t kBatchBytesSize = 1024 * 1024;
static constexpr uint64_t kStridePerOp = 16 * sizeof(float);
static_assert((kBatchBytesSize % kStridePerOp) == 0);

TableInstanceBase::UploadBatch::UploadBatch()
{
    bytes.resize_unsafe(kBatchBytesSize);
    uploads.resize_zeroed(kBatchBytesSize / kStridePerOp);
}

TableInstanceBase::UploadContext::UploadContext()
{
    current_batch = skr::RC<UploadBatch>::New();
}

skr::RC<TableInstanceBase::UploadBatch> TableInstanceBase::UploadContext::GetBatchForUpdate(uint64_t ops, uint64_t& op_start)
{
    mtx.lock();
    SKR_DEFER({ mtx.unlock(); });

    // try close batch
    if (current_batch != nullptr)
    {
        if (current_batch->used_ops + ops > current_batch->uploads.size())
        {
            batches.enqueue(current_batch);
            current_batch = nullptr;
        }
    }

    // pick new batch
    if (!current_batch)
    {
        if (!free_batches.try_dequeue(current_batch))
        {
            current_batch = skr::RC<UploadBatch>::New();
        }
    }
    op_start = current_batch->used_ops.fetch_add(ops);
    return current_batch;
}

void TableInstanceBase::StoreInternal(uint64_t dst_offset, const void* data, uint64_t size)
{
    const auto OpsCount = (size + kStridePerOp - 1) / kStridePerOp;
    uint64_t OpStart = 0;
    auto batch = upload_ctx.GetBatchForUpdate(OpsCount, OpStart);
    auto OpsAddress = &batch->uploads[OpStart];
    auto BytesAddress = &batch->bytes[OpStart * kStridePerOp];
    ::memcpy(BytesAddress, data, size);
    for (uint32_t i = 0; i < OpsCount; i++)
    {
        const Upload upload = {
            .src_offset = (uint32_t)((OpStart * kStridePerOp) + (i * kStridePerOp)),
            .dst_offset = (uint32_t)(dst_offset + i * kStridePerOp),
            .data_size = (uint32_t)skr::min(size, kStridePerOp)
        };
        size -= kStridePerOp;
        OpsAddress[i] = upload;
    }
}

void TableInstanceBase::DispatchSparseUpload(skr::RG::RenderGraph* graph, const RG::ComputePassExecuteFunction& on_exec)
{
    SkrZoneScopedN("GPUScene::DispatchSparseUpload");

    auto upload_buffer = graph->create_buffer(
        [=, this](skr::RG::RenderGraph& g, skr::RG::BufferBuilder& builder) {
            builder.set_name(skr::format(u8"GPUTable{}-UploadData", config.name).c_str())
                .size(kBatchBytesSize)
                .memory_usage(CGPU_MEM_USAGE_CPU_TO_GPU)
                .as_upload_buffer()
                .allow_shader_read()
                .prefer_on_device();
        });

    auto ops_buffer = graph->create_buffer(
        [=, this](skr::RG::RenderGraph& g, skr::RG::BufferBuilder& builder) {
            builder.set_name(skr::format(u8"GPUTable{}-UploadOps", config.name).c_str())
                .size(sizeof(Upload) * kBatchBytesSize / kStridePerOp)
                .memory_usage(CGPU_MEM_USAGE_CPU_TO_GPU)
                .as_upload_buffer()
                .allow_shader_read()
                .prefer_on_device();
        });

    graph->add_compute_pass(
        [=, this](skr::RG::RenderGraph& g, skr::RG::ComputePassBuilder& builder) {
            builder.set_name(skr::format(u8"GPUTable{}-DataSparseUploadPass", config.name).c_str())
                .set_pipeline(manager->GetSparseUploadPipeline())
                .read(u8"upload_buffer", upload_buffer)
                .read(u8"upload_operations", ops_buffer)
                .readwrite(u8"target_buffer", frame_ctxs.get(&g).buffer_handle);
        },
        [this, ops_buffer, upload_buffer, exec = on_exec](skr::RG::RenderGraph& g, skr::RG::ComputePassContext& ctx) {
            SkrZoneScopedN("GPUTable::SparseUploadScene");

            if (exec)
                exec(g, ctx);

            // close batch
            if (upload_ctx.current_batch)
            {
                upload_ctx.batches.enqueue(upload_ctx.current_batch);
                upload_ctx.current_batch = nullptr;
            }
            // get batch to upload
            skr::RC<UploadBatch> BatchToExecute = nullptr;
            upload_ctx.batches.try_dequeue(BatchToExecute);
            if (!BatchToExecute)
                return;

            const uint32_t OpsToExecute = BatchToExecute->used_ops;
            if (OpsToExecute > 0)
            {
                SkrZoneScopedN("GPUScene::CollectAndDisptachCopyPass");

                // Fetch upload data
                if (auto scene_data = ctx.resolve(upload_buffer)->info->cpu_mapped_address)
                {
                    ::memcpy(scene_data, BatchToExecute->bytes.data(), OpsToExecute * kStridePerOp);
                }
                if (auto ops = ctx.resolve(ops_buffer)->info->cpu_mapped_address)
                {
                    ::memcpy(ops, BatchToExecute->uploads.data(), OpsToExecute * sizeof(Upload));
                }

                struct SparseUploadConstants
                {
                    uint32_t num_operations;
                } constants;
                constants.num_operations = static_cast<uint32_t>(OpsToExecute);
                cgpu_compute_encoder_push_constants(ctx.encoder, manager->GetSparseUploadPipeline()->root_signature, u8"constants", &constants);

                cgpu_compute_encoder_set_threadgroup_size(ctx.encoder, 256u, 1, 1);
                cgpu_compute_encoder_dispatch(ctx.encoder, constants.num_operations, 1, 1);
            }

            BatchToExecute->used_ops = 0;
            upload_ctx.free_batches.enqueue(BatchToExecute);
        });
}

CGPUBufferId TableInstanceBase::createBufferWithCapacity(uint32_t capacity)
{
    if (!config.device_)
    {
        SKR_LOG_ERROR(u8"TableInstanceBase: Device not set");
        return nullptr;
    }

    // capacity_bytes 已经在 calculate_layout 中计算
    if (capacity_bytes == 0)
    {
        SKR_LOG_WARN(u8"TableInstanceBase: No data to allocate");
        return nullptr;
    }

    auto table_name = skr::format(u8"TableInstance-{}", config.name);
    CGPUBufferDescriptor buffer_desc = {};
    buffer_desc.name = table_name.c_str();
    buffer_desc.size = capacity_bytes;
    buffer_desc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
    buffer_desc.usages = CGPU_BUFFER_USAGE_SHADER_READ | CGPU_BUFFER_USAGE_SHADER_READWRITE;
    buffer_desc.flags = CGPU_BUFFER_FLAG_DEDICATED_BIT;

    CGPUBufferId new_buffer = cgpu_create_buffer(config.device_, &buffer_desc);
    if (!new_buffer)
    {
        SKR_LOG_ERROR(u8"TableInstanceBase: Failed to create GPU buffer");
    }

    return new_buffer;
}

skr::RG::BufferHandle TableInstanceBase::importBuffer(skr::RG::RenderGraph* graph, const char8_t* name)
{
    if (!buffer || !graph)
        return {};

    auto handle = graph->create_buffer(
        [=, this](skr::RG::RenderGraph& g, skr::RG::BufferBuilder& builder) {
            builder.set_name(name)
                .import(buffer, state_tracker.get_last_state())
                .allow_shader_readwrite();
        });
    graph->track_state(handle, state_tracker);

    return handle;
}

skr::RG::BufferHandle TableInstanceBase::importBufferStructured(skr::RG::RenderGraph* graph, const char8_t* name)
{
    if (!buffer || !graph)
        return {};

    auto handle = graph->create_buffer(
        [=, this](skr::RG::RenderGraph& g, skr::RG::BufferBuilder& builder) {
            builder.set_name(name)
                .import(buffer, state_tracker.get_last_state())
                .structured(0, instance_capacity, config.segments_[0].stride)
                .allow_shader_readwrite();
        });
    graph->track_state(handle, state_tracker);

    return handle;
}

void TableInstanceBase::createBuffer()
{
    if (!config.device_)
    {
        SKR_LOG_ERROR(u8"TableInstanceBase: Device not set");
        return;
    }

    if (buffer)
    {
        SKR_LOG_WARN(u8"TableInstanceBase: Buffer already created");
        return;
    }

    if (capacity_bytes == 0)
    {
        SKR_LOG_WARN(u8"TableInstanceBase: No data to allocate");
        return;
    }

    CGPUBufferDescriptor buffer_desc = {};
    buffer_desc.name = config.name.c_str();
    buffer_desc.size = capacity_bytes;
    buffer_desc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
    buffer_desc.usages = CGPU_BUFFER_USAGE_SHADER_READ | CGPU_BUFFER_USAGE_SHADER_READWRITE;
    buffer_desc.flags = CGPU_BUFFER_FLAG_DEDICATED_BIT;

    buffer = cgpu_create_buffer(config.device_, &buffer_desc);
    if (!buffer)
        SKR_LOG_ERROR(u8"TableInstanceBase: Failed to create GPU buffer");
    else
        SKR_LOG_INFO(u8"TableInstanceBase: Created GPU buffer (%lld MB)", capacity_bytes / (1024 * 1024));
}

void TableInstanceBase::releaseBuffer()
{
    for (uint32_t i = 0; i < frame_ctxs.max_frames_in_flight(); ++i)
    {
        auto& ctx = frame_ctxs[i];
        if (ctx.buffer_to_discard)
        {
            cgpu_free_buffer(ctx.buffer_to_discard);
            ctx.buffer_to_discard = nullptr;
        }
    }
    if (buffer)
    {
        cgpu_free_buffer(buffer);
        buffer = nullptr;
        SKR_LOG_DEBUG(u8"TableInstanceBase: Released GPU buffer");
    }
}

TableInstance::TableInstance(const TableManager* manager, const TableConfig& config)
    : skr::gpu::TableInstanceBase(manager, config)
{
}

uint64_t TableInstance::bindless_index() const
{
    // TODO: SUPPORT BINDLESS
    return ~0;
}

skr::RC<TableInstance> TableManager::CreateTable(const TableConfig& cfg)
{
    auto table = skr::RC<TableInstance>::New(this, cfg);
    tables_mtx.lock();
    tables.add(table);
    tables_mtx.unlock();
    return table;
}

void TableManager::UploadToGPU(skr::RG::RenderGraph& graph)
{
    tables_mtx.lock();
    tables.remove_all_if([](RCWeak<TableInstance> t) { return t.is_expired(); });
    tables_mtx.unlock();

    // calculate size & upload infos
    tables_mtx.lock_shared();
    for (auto table : tables)
    {
        auto tbl = table.lock();
    }
    tables_mtx.unlock_shared();
}

inline static void read_bytes(const char8_t* file_name, uint32_t** bytes, uint32_t* length)
{
    FILE* f = fopen((const char*)file_name, "rb");
    fseek(f, 0, SEEK_END);
    *length = ftell(f);
    fseek(f, 0, SEEK_SET);
    *bytes = (uint32_t*)malloc(*length);
    fread(*bytes, *length, 1, f);
    fclose(f);
}

inline static void read_shader_bytes(const char8_t* virtual_path, uint32_t** bytes, uint32_t* length, ECGPUBackend backend)
{
    char8_t shader_file[256];
    const char8_t* shader_path = SKR_UTF8("./../resources/shaders/");
    strcpy((char*)shader_file, (const char*)shader_path);
    strcat((char*)shader_file, (const char*)virtual_path);
    switch (backend)
    {
    case CGPU_BACKEND_VULKAN:
        strcat((char*)shader_file, (const char*)SKR_UTF8(".spv"));
        break;
    case CGPU_BACKEND_METAL:
        strcat((char*)shader_file, (const char*)SKR_UTF8(".metallib"));
        break;
    case CGPU_BACKEND_D3D12:
    case CGPU_BACKEND_XBOX_D3D12:
        strcat((char*)shader_file, (const char*)SKR_UTF8(".dxil"));
        break;
    default:
        break;
    }
    read_bytes(shader_file, bytes, length);
}

void TableManager::Initialize(CGPUDeviceId device)
{
    SKR_LOG_DEBUG(u8"Creating SparseUpload compute pipeline...");

    // 1. Create compute shader
    uint32_t *shader_bytes, shader_length;
    read_shader_bytes(u8"sparse_upload.sparse_upload", &shader_bytes, &shader_length, device->adapter->instance->backend);

    CGPUShaderLibraryDescriptor shader_desc = {
        .name = u8"SparseUploadComputeShader",
        .code = shader_bytes,
        .code_size = shader_length
    };
    sparse_upload_shader = cgpu_create_shader_library(device, &shader_desc);
    free(shader_bytes);

    if (!sparse_upload_shader)
    {
        SKR_LOG_ERROR(u8"Failed to create SparseUpload compute shader");
        return;
    }

    // 2. Create root signature
    const char8_t* push_constant_name = u8"constants";
    CGPUShaderEntryDescriptor compute_shader_entry = {
        .library = sparse_upload_shader,
        .entry = u8"sparse_upload",
    };

    CGPURootSignatureDescriptor root_desc = {
        .shaders = &compute_shader_entry,
        .shader_count = 1,
        .push_constant_names = &push_constant_name,
        .push_constant_count = 1,
    };
    sparse_upload_root_signature = cgpu_create_root_signature(device, &root_desc);

    if (!sparse_upload_root_signature)
    {
        SKR_LOG_ERROR(u8"Failed to create SparseUpload root signature");
        return;
    }

    // 3. Create compute pipeline
    CGPUComputePipelineDescriptor pipeline_desc = {
        .root_signature = sparse_upload_root_signature,
        .compute_shader = &compute_shader_entry,
        .name = u8"SparseUploadPipeline"
    };
    sparse_upload_pipeline = cgpu_create_compute_pipeline(device, &pipeline_desc);

    if (!sparse_upload_pipeline)
    {
        SKR_LOG_ERROR(u8"Failed to create SparseUpload compute pipeline");
        return;
    }

    SKR_LOG_INFO(u8"SparseUpload compute pipeline created successfully");
}

void TableManager::Shutdown()
{
    SKR_LOG_INFO(u8"Shutting down GPUScene...");

    if (sparse_upload_pipeline)
    {
        cgpu_free_compute_pipeline(sparse_upload_pipeline);
        sparse_upload_pipeline = nullptr;
    }

    if (sparse_upload_root_signature)
    {
        cgpu_free_root_signature(sparse_upload_root_signature);
        sparse_upload_root_signature = nullptr;
    }

    if (sparse_upload_shader)
    {
        cgpu_free_shader_library(sparse_upload_shader);
        sparse_upload_shader = nullptr;
    }

    SKR_LOG_INFO(u8"GPUScene shutdown complete");
}

skr::RC<TableManager> TableManager::Create(CGPUDeviceId cgpu_device)
{
    auto manager = skr::RC<TableManager>::New();
    manager->Initialize(cgpu_device);
    return manager;
}

TableManager::~TableManager()
{
    Shutdown();
}

} // namespace skr::gpu
