#pragma once
#include "SkrGui/framework/fwd_containers.hpp"

SKR_DECLARE_TYPE_ID_FWD(skr::gdi, GDIPaint, skr_gdi_paint)
SKR_DECLARE_TYPE_ID_FWD(skr::gdi, IGDIImage, skr_gdi_image)
SKR_DECLARE_TYPE_ID_FWD(skr::gdi, IGDITexture, skr_gdi_texture)
SKR_DECLARE_TYPE_ID_FWD(skr::gdi, IGDIRenderer, skr_gdi_renderer)
SKR_DECLARE_TYPE_ID_FWD(skr::gdi, IGDIMaterial, skr_gdi_material)

typedef struct skr_gdi_element_draw_command_t
{
    skr_gdi_texture_id texture SKR_IF_CPP(= nullptr);
    skr_gdi_material_id material SKR_IF_CPP(= nullptr);
    uint32_t first_index SKR_IF_CPP(= 0);
    uint32_t index_count SKR_IF_CPP(= 0);
} skr_gdi_element_draw_command_t;

typedef struct skr_gdi_vertex_t
{
    skr_float4_t position;
    skr_float2_t texcoord;
    skr_float2_t aa;
    skr_float2_t clipUV; //uv in clipspace
    skr_float2_t clipUV2;
    uint32_t     color; 
} skr_gdi_vertex_t;

typedef uint16_t skr_gdi_index_t;

typedef void(*skr_gdi_custom_vertex_painter_t)(struct skr_gdi_vertex_t* pVertex, void* usrdata);

namespace skr {
namespace gdi {

template<typename T>
using LiteSpan = skr::gui::LiteSpan<T>;

using index_t = skr_gdi_index_t;
typedef struct IGDIImage* GDIImageId;
typedef struct IGDITexture* GDITextureId;
typedef struct IGDIMaterial* GDIMaterialId;
typedef struct IGDIRenderer* GDIRendererId;

// gdi
using GDIElementDrawCommand = skr_gdi_element_draw_command_t;
using GDIVertex = skr_gdi_vertex_t;

enum class EGDIBackend
{
    NANOVG,
    Count
};

enum class EGDIResourceState : uint32_t
{
    Requsted     =    0x00000001,
    Loading      =    0x00000002,
    Initializing =    0x00000004,
    Okay         =    0x00000008,
    Finalizing   =    0x00000010,
    Count = 5
};

struct SKR_GUI_API GDIResource
{
    virtual ~GDIResource() SKR_NOEXCEPT = default;
    virtual EGDIResourceState get_state() const SKR_NOEXCEPT = 0;
};

struct SKR_GUI_API GDIPaint
{
    virtual ~GDIPaint() SKR_NOEXCEPT = default;

    /*
    virtual void radial_gradient(float cx, float cy, float inr, float outr, skr_float4_t icol, skr_float4_t ocol) SKR_NOEXCEPT = 0;
    virtual void box_gradient(float x, float y, float w, float h, float r, float f, skr_float4_t icol, skr_float4_t ocol) SKR_NOEXCEPT = 0;
    virtual void linear_gradient(float sx, float sy, float ex, float ey, skr_float4_t icol, skr_float4_t ocol) SKR_NOEXCEPT = 0;
    */
    virtual void enable_imagespace_coordinate(bool enable) SKR_NOEXCEPT = 0;
    virtual void custom_vertex_color(skr_gdi_custom_vertex_painter_t painter, void* usrdata) SKR_NOEXCEPT = 0;

    virtual void set_pattern(float cx, float cy, float w, float h, float angle, GDITextureId texture, skr_float4_t ocol) SKR_NOEXCEPT = 0;
    virtual void set_pattern(float cx, float cy, float w, float h, float angle, GDIMaterialId material, skr_float4_t ocol) SKR_NOEXCEPT = 0;
};

enum class EGDIWinding : uint32_t
{
    CCW = 1,
    CW = 2,
    Count = 2
};

enum class EGDISolidity : uint32_t
{
    Solid = 1,
    Hole = 2,
    Count = 2
};

struct SKR_GUI_API GDIElement
{
    friend struct IGDIRenderer;
    virtual ~GDIElement() SKR_NOEXCEPT = default;
    
    virtual void begin_frame(float devicePixelRatio) = 0;
    virtual void begin_path() = 0;
    virtual void close_path() = 0;
    virtual void arc(float cx, float cy, float r, float a0, float a1, EGDIWinding dir) = 0;
    virtual void rect(float x, float y, float w, float h) = 0;
    virtual void circle(float cx, float cy, float r) = 0;
    virtual void rounded_rect_varying(float x, float y, float w, float h, float radTopLeft, float radTopRight, float radBottomRight, float radBottomLeft) = 0;
    
    virtual void translate(float x, float y) = 0;
    virtual void rotate(float r) = 0;
    virtual void move_to(float x, float y) = 0;
    virtual void line_to(float x, float y) = 0;
    
    virtual void stroke_color(uint32_t r, uint32_t g, uint32_t b, uint32_t a) = 0;
    virtual void stroke_color(float r, float g, float b, float a) = 0;
    virtual void stroke_paint(GDIPaint* paint) = 0;
    virtual void stroke_width(float size) = 0;
    virtual void stroke() = 0;

    virtual void path_winding(EGDISolidity dir) = 0;

    virtual void fill_color(uint32_t r, uint32_t g, uint32_t b, uint32_t a) = 0;
    virtual void fill_color(float r, float g, float b, float a) = 0;
    virtual void fill_paint(GDIPaint* paint) = 0;
    virtual void fill() = 0;

    virtual void save() = 0;
    virtual void restore() = 0;

    virtual void set_z(int32_t z) = 0;
    virtual int32_t get_z() const = 0;
};

struct SKR_GUI_API GDICanvas
{
    virtual ~GDICanvas() SKR_NOEXCEPT = default;

    virtual void add_element(GDIElement* element) SKR_NOEXCEPT = 0;
    virtual void remove_element(GDIElement* element) SKR_NOEXCEPT = 0;
    virtual LiteSpan<GDIElement*> all_elements() SKR_NOEXCEPT = 0;
    virtual void clear_elements() SKR_NOEXCEPT = 0;

    virtual void set_zrange(int32_t min, int32_t max) SKR_NOEXCEPT = 0;
    virtual void get_zrange(int32_t* out_min, int32_t* out_max) SKR_NOEXCEPT = 0;

    virtual void enable_hardware_z() SKR_NOEXCEPT = 0;
    virtual void disable_hardware_z() SKR_NOEXCEPT = 0;
    virtual bool is_hardware_z_enabled() const SKR_NOEXCEPT = 0;

    virtual void set_pivot(float x, float y) SKR_NOEXCEPT = 0;
    virtual void get_pivot(float* out_x, float* out_y) SKR_NOEXCEPT = 0;

    virtual void set_size(float w, float h) SKR_NOEXCEPT = 0;
    virtual void get_size(float* out_w, float* out_h) SKR_NOEXCEPT = 0;
};

struct SKR_GUI_API GDIViewport
{
    virtual ~GDIViewport() SKR_NOEXCEPT = default;

    virtual void add_canvas(GDICanvas* canvas) SKR_NOEXCEPT = 0;
    virtual void remove_canvas(GDICanvas* canvas) SKR_NOEXCEPT = 0;
    virtual void clear_canvas() SKR_NOEXCEPT = 0;
    virtual LiteSpan<GDICanvas*> all_canvas() SKR_NOEXCEPT = 0;
};

struct SKR_GUI_API GDIDevice
{
    virtual ~GDIDevice() SKR_NOEXCEPT = default;

    [[nodiscard]] static GDIDevice* Create(EGDIBackend backend);
    static void Free(GDIDevice* device);

    [[nodiscard]] virtual GDICanvas* create_canvas();
    virtual void free_canvas(GDICanvas* canvas);

    [[nodiscard]] virtual GDIViewport* create_viewport();
    virtual void free_viewport(GDIViewport* render_group);

    [[nodiscard]] virtual GDIElement* create_element() = 0;
    virtual void free_element(GDIElement* element) = 0;

    [[nodiscard]] virtual GDIPaint* create_paint() = 0;
    virtual void free_paint(GDIPaint* paint) = 0;
};

} }

SKR_DECLARE_TYPE_ID(skr::gdi::GDIResource, skr_gdi_resource)
SKR_DECLARE_TYPE_ID(skr::gdi::GDIPaint, skr_gdi_paint)
SKR_DECLARE_TYPE_ID(skr::gdi::GDIElement, skr_gdi_elements)
SKR_DECLARE_TYPE_ID(skr::gdi::GDICanvas, skr_gdi_canvas)
SKR_DECLARE_TYPE_ID(skr::gdi::GDIViewport, skr_gdi_viewport)
SKR_DECLARE_TYPE_ID(skr::gdi::GDIDevice, skr_gdi_device);