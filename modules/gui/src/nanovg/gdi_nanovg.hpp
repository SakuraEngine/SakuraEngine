#pragma once
#include "SkrGui/gdi.h"
#include "nanovg.h"
#include <EASTL/vector.h>

namespace skr {
namespace gdi {

struct SGDIElementDrawCommandNVG
{
    // texture
    // material
    uint32_t first_index;
    uint32_t index_count;
};

struct SGDIElementDrawCommandNVG2
{
    // texture
    // material
    uint32_t first_index;
    uint32_t index_count;
    uint32_t ib_offset;
    uint32_t vb_offset;
    uint32_t tb_offset;
};

struct SGDIElementNVG : public SGDIElement
{
    using index_t = uint16_t;

    inline SGDIElementNVG() : nvg(nullptr) {}

    void begin_frame(float devicePixelRatio) final;
    void begin_path() final;
    void rect(float x, float y, float w, float h) final;
    void rounded_rect_varying(float x, float y, float w, float h, float radTopLeft, float radTopRight, float radBottomRight, float radBottomLeft) final;
    void fill_color(uint32_t r, uint32_t g, uint32_t b, uint32_t a) final;
    void fill_color(float r, float g, float b, float a) final;
    void fill_paint(SGDIPaint* paint) final;
    void fill() final;

    NVGcontext* nvg = nullptr;
    skr::vector<SGDIVertex> vertices;
    skr::vector<index_t> indices;
    skr::vector<SGDIElementDrawCommandNVG> commands;
};

struct SGDICanvasNVG : public SGDICanvas
{
    void add_element(SGDIElement* element, const skr_float4_t& transform) final;
};

struct SGDICanvasGroupNVG : public SGDICanvasGroup
{
    // void add_canvas(SGDICanvas* canvas) final;
    // void remove_canvas(SGDICanvas* canvas) final;

    skr::vector<skr::render_graph::BufferHandle> vertex_buffers;
    skr::vector<skr::render_graph::BufferHandle> transform_buffers;
    skr::vector<skr::render_graph::BufferHandle> index_buffers;

    skr::vector<SGDIElementDrawCommandNVG2> render_commands;
    skr::vector<SGDIVertex> render_vertices;
    skr::vector<SGDITransform> render_transforms;
    skr::vector<SGDIElementNVG::index_t> render_indices;
};

struct SGDIDeviceNVG : public SGDIDevice
{
    ~SGDIDeviceNVG();
    int initialize(CGPUDeviceId device) SKR_NOEXCEPT;
    void finalize() SKR_NOEXCEPT;

    SGDICanvas* create_canvas() final;
    void free_canvas(SGDICanvas* canvas) final;

    SGDICanvasGroup* create_canvas_group() final;
    void free_canvas_group(SGDICanvasGroup* group) final;

    SGDIElement* create_element() final;
    void free_element(SGDIElement* element) final;

    void render(skr::render_graph::RenderGraph* rg, SGDICanvasGroup* canvas_group) final;

    CGPUVertexLayout vertex_layout = {};
    CGPURenderPipelineId pipeline = nullptr;
};


} }