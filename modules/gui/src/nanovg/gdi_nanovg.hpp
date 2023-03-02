#pragma once
#include "SkrGui/gdi.h"
#include "nanovg.h"
#include <EASTL/vector.h>

namespace skr {
namespace gdi {

struct SGDIElementNVG : public SGDIElement
{
    inline SGDIElementNVG(NVGcontext* nvg) : nvg(nvg) {}

    void begin_frame(float devicePixelRatio) final;
    void begin_path() final;
    void rounded_rect_varying(float x, float y, float w, float h, float radTopLeft, float radTopRight, float radBottomRight, float radBottomLeft) final;
    void fill_color(uint32_t r, uint32_t g, uint32_t b, uint32_t a) final;
    void fill_color(float r, float g, float b, float a) final;
    void fill_paint(SGDIPaint* paint) final;
    void fill() final;

    NVGcontext* nvg = nullptr;
    eastl::vector<SGDIVertex> vertices;
    eastl::vector<uint32_t> indices;
};

struct SGDICanvasNVG : public SGDICanvas
{
    void add_element(SGDIElement* element, const skr_float4x4_t& transform) final;
};

struct SGDIDeviceNVG : public SGDIDevice
{
    static SGDIDevice* Create(EGDIBackend backend);
    static void Free(SGDIDevice* device);

    SGDICanvas* create_canvas() final;
    void free_canvas(SGDICanvas* canvas) final;

    SGDIElement* create_element() final;
    void free_element(SGDIElement* element) final;

    void render(skr::render_graph::RenderGraph* rg, SGDICanvas* canvas) final;
};


} }