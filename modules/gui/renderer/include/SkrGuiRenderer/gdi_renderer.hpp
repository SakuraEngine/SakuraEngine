#pragma once
#include "SkrGuiRenderer/module.configure.h"
#include "SkrGui/interface/gdi_renderer.hpp"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include <containers/vector.hpp>
#include <EASTL/vector_map.h>

#include "cgpu/cgpux.h"
#include "cgpu/io.h"
#include "rtm/rtmx.h"
#include "platform/atomic.h"
#include "utils/threaded_service.h"
#include "utils/concurrent_queue.h"

namespace skr {
namespace gdi {

enum EGDIRendererPipelineAttribute
{
    GDI_RENDERER_PIPELINE_ATTRIBUTE_TEST_Z = 0x00000001,
    GDI_RENDERER_PIPELINE_ATTRIBUTE_TEXTURED = GDI_RENDERER_PIPELINE_ATTRIBUTE_TEST_Z << 1,
    GDI_RENDERER_PIPELINE_ATTRIBUTE_WRITE_Z = GDI_RENDERER_PIPELINE_ATTRIBUTE_TEXTURED << 1,
    GDI_RENDERER_PIPELINE_ATTRIBUTE_CUSTOM_SAMPLER = GDI_RENDERER_PIPELINE_ATTRIBUTE_WRITE_Z << 1,
    GDI_RENDERER_PIPELINE_ATTRIBUTE_COUNT = 4
};
using GDIRendererPipelineAttributes = uint32_t;

struct GDIElementDrawCommand_RenderGraph
{
    GDIRendererPipelineAttributes attributes = 0;
    GDITextureId texture = nullptr;
    GDIMaterialId material = nullptr;
    uint32_t first_index = 0;
    uint32_t index_count = 0;
    uint32_t ib_offset = 0;
    uint32_t vb_offset = 0;
    uint32_t tb_offset = 0;
    uint32_t pb_offset = 0;
};

struct SKR_GUI_RENDERER_API GDIViewportData_RenderGraph
{
    inline GDIViewportData_RenderGraph(GDIViewport* viewport) SKR_NOEXCEPT : viewport(viewport) { }

    GDIViewport* viewport = nullptr;

    skr::vector<skr::render_graph::BufferHandle> vertex_buffers;
    skr::vector<skr::render_graph::BufferHandle> transform_buffers;
    skr::vector<skr::render_graph::BufferHandle> projection_buffers;
    skr::vector<skr::render_graph::BufferHandle> index_buffers;

    skr::vector<GDIElementDrawCommand_RenderGraph> render_commands;
    skr::vector<GDIVertex> render_vertices;
    skr::vector<rtm::matrix4x4f> render_transforms;
    skr::vector<rtm::matrix4x4f> render_projections;
    skr::vector<index_t> render_indices;
};

struct GDIRendererDescriptor_RenderGraph
{
    ECGPUFormat target_format;
    CGPUDeviceId device = nullptr;
    CGPUQueueId transfer_queue = nullptr;
    skr_vfs_t* vfs = nullptr;
    skr_io_ram_service_t* ram_service = nullptr;
    skr_io_vram_service_t* vram_service = nullptr;
    skr_threaded_service_t* aux_service = nullptr;
};

struct ViewportRenderParams_RenderGraph
{
    skr::render_graph::RenderGraph* render_graph = nullptr;
};

struct GDIImageDescriptor_RenderGraph
{
    bool useImageCoder = false;
    skr_guid_t guid;
};

struct GDITextureDescriptor_RenderGraph
{
    bool useImageCoder = false;
    skr_guid_t guid;
};

struct SKR_GUI_RENDERER_API GDIImageAsyncData_RenderGraph
{
    friend struct GDITextureAsyncData_RenderGraph;

    struct 
    {
        skr::string uri = "";
    } from_file;
    
    struct 
    {
        uint32_t width;
        uint32_t height;
        EGDIImageFormat gdi_format;
        // TODO: mip_count
        // uint32_t mip_count = 0;
    } from_data;

    bool useImageCoder = false;
    skr_async_request_t ram_request = {};
    skr_async_request_t aux_request = {};
    skr_threaded_service_t* aux_service = nullptr; 
    
    GDIImageId DoAsync(struct GDIImage_RenderGraph* owner, skr_vfs_t* vfs, skr_io_ram_service_t* ram_service) SKR_NOEXCEPT;

protected:
    eastl::function<void()> ram_io_enqueued_callback = {};
    eastl::function<void()> ram_io_finished_callback = {};
    eastl::function<void()> ram_data_finsihed_callback = {};
};

struct SKR_GUI_RENDERER_API GDITextureAsyncData_RenderGraph
{
    skr_async_request_t vram_request = {};
    skr_async_vtexture_destination_t vram_destination = {};

    CGPUDeviceId device;
    CGPUQueueId transfer_queue;
    CGPURootSignatureId root_signature = nullptr;
    skr_io_vram_service_t* vram_service = nullptr; 

    GDITextureId DoAsync(struct GDITexture_RenderGraph* texture, skr_vfs_t* vfs, skr_io_ram_service_t* ram_service) SKR_NOEXCEPT;

protected:
    eastl::function<void()> vram_enqueued_callback = {};
    eastl::function<void()> vram_finsihed_callback = {};
};

struct SKR_GUI_RENDERER_API GDIImage_RenderGraph : public IGDIImage
{
    GDIImage_RenderGraph(struct GDIRenderer_RenderGraph* renderer) SKR_NOEXCEPT
        : renderer(renderer) { }

    EGDIResourceState get_state() const SKR_NOEXCEPT final;
    GDIRendererId get_renderer() const SKR_NOEXCEPT final;
    uint32_t get_width() const SKR_NOEXCEPT final;
    uint32_t get_height() const SKR_NOEXCEPT final;
    LiteSpan<const uint8_t> get_data() const SKR_NOEXCEPT final;
    EGDIImageFormat get_format() const SKR_NOEXCEPT final;

    void preInit(const GDIImageDescriptor* desc);

    uint32_t image_width = 0;
    uint32_t image_height = 0;
    uint32_t image_depth = 0;
    ECGPUFormat format = CGPU_FORMAT_UNDEFINED;

    SAtomicU32 state = static_cast<uint32_t>(EGDIResourceState::Requsted);
    EGDIImageSource source = EGDIImageSource::Count;

    skr_blob_t pixel_data;
    skr_async_ram_destination_t raw_data = {};

    GDIImageAsyncData_RenderGraph async_data;
    struct GDIRenderer_RenderGraph* renderer = nullptr;
};

struct SKR_GUI_RENDERER_API GDITexture_RenderGraph : public IGDITexture
{
    GDITexture_RenderGraph(struct GDIRenderer_RenderGraph* renderer) SKR_NOEXCEPT
        : intermediate_image(renderer), renderer(renderer) { }

    EGDIResourceState get_state() const SKR_NOEXCEPT final;
    GDIRendererId get_renderer() const SKR_NOEXCEPT final;
    uint32_t get_width() const SKR_NOEXCEPT final;
    uint32_t get_height() const SKR_NOEXCEPT final;

    EGDITextureType get_type() const SKR_NOEXCEPT final;

    void intializeBindTable() SKR_NOEXCEPT;
    
    SAtomicU32 state = static_cast<uint32_t>(EGDIResourceState::Requsted);
    EGDITextureSource source = EGDITextureSource::Count;
    GDITextureAsyncData_RenderGraph async_data;

    GDIImage_RenderGraph intermediate_image;

    CGPUTextureId texture = nullptr;
    CGPUTextureViewId texture_view = nullptr;
    CGPUSamplerId sampler = nullptr;
    CGPUXBindTableId bind_table = nullptr;
    struct GDIRenderer_RenderGraph* renderer = nullptr;
};

struct SKR_GUI_RENDERER_API GDITextureUpdate_RenderGraph : public IGDITextureUpdate
{
    EGDIResourceState get_state() const SKR_NOEXCEPT final;

    GDITextureId texture = nullptr;
    GDIImageId image = nullptr;
    skr::render_graph::BufferHandle upload_buffer;
    skr::render_graph::TextureHandle texture_handle;

    uint64_t execute_frame_index = 0;
    SAtomicU32 state = static_cast<uint32_t>(EGDIResourceState::Requsted);
};

struct SKR_GUI_RENDERER_API GDIRenderer_RenderGraph : public IGDIRenderer
{
    friend struct GDITexture_RenderGraph;

    // Tier 1
    int initialize(const GDIRendererDescriptor* desc) SKR_NOEXCEPT final;
    int finalize() SKR_NOEXCEPT final;
    GDIImageId create_image(const GDIImageDescriptor* descriptor) SKR_NOEXCEPT final;
    GDITextureId create_texture(const GDITextureDescriptor* descriptor) SKR_NOEXCEPT final;
    GDITextureUpdateId update_texture(const GDITextureUpdateDescriptor* descriptor) SKR_NOEXCEPT final;
    void free_image(GDIImageId image) SKR_NOEXCEPT final;
    void free_texture(GDITextureId texture) SKR_NOEXCEPT final;
    void free_texture_update(GDITextureUpdateId texture) SKR_NOEXCEPT final;
    void render(GDIViewport* viewport, const ViewportRenderParams* params) SKR_NOEXCEPT final;

    // Tier 2
    bool support_hardware_z(float* out_min, float* out_max) const SKR_NOEXCEPT final
    {
        if (out_min) *out_min = 0;
        if (out_max) *out_max = 1000.f;
        return true;
    }
    bool support_mipmap_generation() const SKR_NOEXCEPT final;

protected:
    void updatePendingTextures(skr::render_graph::RenderGraph* graph) SKR_NOEXCEPT;
    moodycamel::ConcurrentQueue<GDITextureUpdate_RenderGraph*> request_updates;
    moodycamel::ConcurrentQueue<GDITextureUpdate_RenderGraph*> pending_updates;

protected:
    CGPURenderPipelineId findOrCreateRenderPipeline(GDIRendererPipelineAttributes attributes, ECGPUSampleCount sample_count = CGPU_SAMPLE_COUNT_1);
    CGPURenderPipelineId createRenderPipeline(GDIRendererPipelineAttributes attributes, ECGPUSampleCount sample_count = CGPU_SAMPLE_COUNT_1);
    void createRenderPipelines();

    CGPUVertexLayout vertex_layout = {};
    struct PipelineKey
    {
        GDIRendererPipelineAttributes attributes;
        ECGPUSampleCount sample_count;
        inline bool operator==(const PipelineKey& other) const
        {
            return attributes == other.attributes && sample_count == other.sample_count;
        }
        inline bool operator!=(const PipelineKey& other) const
        {
            return attributes != other.attributes || sample_count != other.sample_count;
        }
        inline bool operator<(const PipelineKey& other) const
        {
            return attributes < other.attributes || (attributes == other.attributes && sample_count < other.sample_count);
        }
    };
    eastl::vector_map<PipelineKey, CGPURenderPipelineId> pipelines;
    skr_threaded_service_t* aux_service = nullptr;
    skr_io_ram_service_t* ram_service = nullptr;
    skr_io_vram_service_t* vram_service = nullptr;
    skr_vfs_t* vfs = nullptr;
    CGPUDeviceId device = nullptr;
    CGPUQueueId transfer_queue = nullptr;
    CGPUSamplerId static_color_sampler = nullptr;
    CGPURootSignaturePoolId rs_pool = nullptr;
    ECGPUFormat target_format;
};

} }