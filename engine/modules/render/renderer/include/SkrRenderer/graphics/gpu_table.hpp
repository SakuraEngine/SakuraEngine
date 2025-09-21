#pragma once
#include "SkrBase/atomic/atomic_mutex.hpp"
#include "SkrCore/memory/rc.hpp"
#include "SkrContainersDef/vector.hpp"
#include "SkrContainersDef/concurrent_queue.hpp"
#include "SkrRenderGraph/frame_resource.hpp"
#include "SkrRenderer/render_device.h"

namespace skr::gpu
{

template <typename T> 
struct GPUDatablock;
template<typename T>
inline constexpr bool is_datablock_v = false;
template<typename U>
inline constexpr bool is_datablock_v<GPUDatablock<U>> = true;

using TableSegmentID = uint32_t;
using TableInstanceBaseID = uint32_t;
struct TableManager;

// Segment - Buffer中的基本分配单位
struct TableSegment
{
    uint32_t stride;        // 每个实例的大小（对于bundle是所有组件的总大小）
    uint32_t align;         // 对齐要求
    uint32_t buffer_offset; // 在缓冲区中的偏移
};

// Builder 模式初始化器
struct SKR_RENDERER_API TableConfig
{
public:
    TableConfig(CGPUDeviceId device, const char8_t* name);

    TableConfig& with_instances(uint32_t initial_instances);
    TableConfig& with_page_size(uint32_t page_size);
    TableConfig& add_component(TableSegmentID type_id, uint32_t element_size, uint32_t element_align = 4);

private:
    friend struct TableInstanceBase;
    CGPUDeviceId device_;
    uint32_t initial_instances = 16384;
    uint32_t page_size = UINT32_MAX; 
    skr::Vector<TableSegment> segments_;
    skr::String name = u8"GPUTable";
};

struct SKR_RENDERER_API TableInstanceBase
{
public:
    TableInstanceBase(const TableManager* manager, const TableConfig& config);
    ~TableInstanceBase();
    TableInstanceBase(const TableInstanceBase&) = delete;
    TableInstanceBase& operator=(const TableInstanceBase&) = delete;

    skr::render_graph::BufferHandle UpdateTableBuffer(skr::render_graph::RenderGraph* graph, uint64_t required_instances);
    skr::render_graph::BufferHandle UpdateTableBufferStructured(skr::render_graph::RenderGraph* graph, uint64_t required_instances);
    template <typename T>
    void Store(uint64_t comp_id, uint64_t inst, const T& value)
    {
        const uint64_t offset = GetComponentOffset(comp_id, inst);
        if constexpr (gpu::is_datablock_v<T>)
        {
            StoreInternal(offset, &value, sizeof(value));
        }
        else
        {
            static_assert(GPUDatablock<T>::Size != ~0);
            GPUDatablock<T> data(value);
            StoreInternal(offset, &data, GPUDatablock<T>::Size);
        }
    }
    void DispatchSparseUpload(skr::render_graph::RenderGraph* graph, const render_graph::ComputePassExecuteFunction& on_exec);

    CGPUBufferId Resize(uint32_t new_instance_capacity);
    void CopySegments(skr::render_graph::RenderGraph* graph,
        skr::render_graph::BufferHandle src_buffer,
        skr::render_graph::BufferHandle dst_buffer,
        uint64_t old_buffer_size) const;

    uint32_t GetComponentSize(TableSegmentID component_id) const;
    uint32_t GetComponentOffset(TableSegmentID component_id, TableInstanceBaseID instance_id) const;

    inline CGPUBufferId GetBuffer() const { return buffer; }
    inline uint64_t GetCapacityInBytes() const { return capacity_bytes; }
    inline uint32_t GetInstanceCapacity() const { return instance_capacity; }

    inline uint32_t GetPageSize() const { return config.page_size; }
    inline uint32_t GetPageStrideInSize() const { return page_stride_bytes; }

protected:
    friend struct TableManager;

    bool needsResize(uint32_t required_instances) const;
    skr::render_graph::BufferHandle importBuffer(skr::render_graph::RenderGraph* graph, const char8_t* name);
    skr::render_graph::BufferHandle importBufferStructured(skr::render_graph::RenderGraph* graph, const char8_t* name);
    void StoreInternal(uint64_t offset, const void* data, uint64_t size);    

    struct Upload
    {
        uint32_t src_offset;
        uint32_t dst_offset;
        uint32_t data_size;
    };
    struct FrameContext
    {
        CGPUBufferId buffer_to_discard = nullptr;
        skr::render_graph::BufferHandle buffer_handle; 
    };
    skr::render_graph::FrameResource<FrameContext> frame_ctxs;

    struct UploadBatch
    {
        UploadBatch();
        // Delete copies.
        UploadBatch(UploadBatch const&) = delete;
        UploadBatch& operator=(UploadBatch const&) = delete;

        skr::Vector<Upload> uploads;
        skr::Vector<uint8_t> bytes;
        std::atomic_uint32_t used_ops = 0;
        
        SKR_RC_IMPL();
    };
    struct UploadContext
    {
        UploadContext();

        skr::RC<UploadBatch> GetBatchForUpdate(uint64_t ops, uint64_t& op_start);

        skr::shared_atomic_mutex mtx;
        skr::RC<UploadBatch> current_batch = nullptr;
        skr::ConcurrentQueue<skr::RC<UploadBatch>> batches;
        skr::ConcurrentQueue<skr::RC<UploadBatch>> free_batches;
    } upload_ctx;

    CGPUBufferId buffer = nullptr;
    render_graph::RenderGraphStateTracker state_tracker;
    const TableManager* manager = nullptr;

    TableConfig config;
    uint64_t capacity_bytes = 0;
    uint32_t instance_capacity = 0;
    uint32_t page_stride_bytes = 0; // 每页的字节大小

    void calculateLayout();
    void createBuffer();
    void releaseBuffer();
    CGPUBufferId createBufferWithCapacity(uint32_t instance_capacity);

    SKR_RC_IMPL();
};

struct SKR_RENDERER_API TableInstance : public TableInstanceBase
{
    TableInstance(const TableManager* manager, const TableConfig& config);
    uint64_t bindless_index() const;
};

struct SKR_RENDERER_API TableManager
{
public:
    ~TableManager();
    static skr::RC<TableManager> Create(CGPUDeviceId cgpu_device);
    skr::RC<TableInstance> CreateTable(const TableConfig& cfg);
    void UploadToGPU(skr::render_graph::RenderGraph& graph);
    auto GetSparseUploadPipeline() const { return sparse_upload_pipeline; }
    
private:
    // SparseUpload compute pipeline resources
    CGPUShaderLibraryId sparse_upload_shader = nullptr;
    CGPURootSignatureId sparse_upload_root_signature = nullptr;
    CGPUComputePipelineId sparse_upload_pipeline = nullptr;
    void Initialize(CGPUDeviceId device);
    void Shutdown();

    skr::shared_atomic_mutex tables_mtx;
    skr::Vector<skr::RCWeak<TableInstance>> tables;
    SKR_RC_IMPL();
};

} // namespace skr::gpu
