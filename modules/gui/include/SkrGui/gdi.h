#pragma once
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrGui/module.configure.h"
#include "platform/configure.h"
#include "utils/types.h"
#include <containers/vector.hpp>

namespace skr { namespace render_graph { class RenderGraph; } }

namespace skr {
namespace gdi {

enum class EGDIBackend
{
    NANOVG,
    Count
};

struct SKR_GUI_API SGDIPaint
{

};

struct SGDITransform
{
    skr_float4x4_t transform;
};

struct SGDIVertex
{
    skr_float4_t position;
    skr_float2_t texcoord;
    skr_float2_t aa;
    skr_float2_t clipUV; //uv in clipspace
    skr_float2_t clipUV2;
    uint32_t     color; 
};

struct SKR_GUI_API SGDIElement
{
    virtual ~SGDIElement() SKR_NOEXCEPT = default;
    
    virtual void begin_frame(float devicePixelRatio) = 0;
    virtual void begin_path() = 0;
    virtual void rect(float x, float y, float w, float h) = 0;
    virtual void rounded_rect_varying(float x, float y, float w, float h, float radTopLeft, float radTopRight, float radBottomRight, float radBottomLeft) = 0;
    virtual void fill_color(uint32_t r, uint32_t g, uint32_t b, uint32_t a) = 0;
    virtual void fill_color(float r, float g, float b, float a) = 0;
    virtual void fill_paint(SGDIPaint* paint) = 0;
    virtual void fill() = 0;
};

struct SKR_GUI_API SGDICanvas
{
    virtual ~SGDICanvas() SKR_NOEXCEPT = default;

    virtual void add_element(SGDIElement* element, const skr_float4_t& transform);
    virtual void remove_element(SGDIElement* element);
    virtual skr::span<SGDIElement*> all_elements();
protected:
    skr::vector<SGDIElement*> all_elements_;
};

struct SKR_GUI_API SGDICanvasGroup
{
    virtual ~SGDICanvasGroup() SKR_NOEXCEPT = default;

    virtual void add_canvas(SGDICanvas* canvas);
    virtual void remove_canvas(SGDICanvas* canvas);
    virtual skr::span<SGDICanvas*> all_canvas();
protected:
    skr::vector<SGDICanvas*> all_canvas_;
};

struct SKR_GUI_API SGDIDevice
{
    virtual ~SGDIDevice() SKR_NOEXCEPT = default;

    [[nodiscard]] static SGDIDevice* Create(CGPUDeviceId device, EGDIBackend backend);
    static void Free(SGDIDevice* device);

    [[nodiscard]] virtual SGDICanvas* create_canvas();
    virtual void free_canvas(SGDICanvas* canvas);

    [[nodiscard]] virtual SGDICanvasGroup* create_canvas_group();
    virtual void free_canvas_group(SGDICanvasGroup* canvas_group);

    [[nodiscard]] virtual SGDIElement* create_element() = 0;
    virtual void free_element(SGDIElement* element) = 0;

    virtual void render(skr::render_graph::RenderGraph* rg, SGDICanvasGroup* canvas_group) = 0;
};

} }
