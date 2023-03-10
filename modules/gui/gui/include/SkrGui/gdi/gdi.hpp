#pragma once
#include "SkrGui/module.configure.h"
#include "utils/types.h"

namespace skr {
namespace gdi {

using index_t = uint16_t;
struct IGDIImage;
struct IGDITexture;
struct SGDIMaterial;
struct IGDIRenderer;
typedef struct IGDIImage* SGDIImageId;
typedef struct IGDITexture* SGDITextureId;
typedef struct SGDIMaterial* SGDIMaterialId;
typedef struct IGDIRenderer* SGDIRendererId;

// gdi

template<typename T>
struct LiteSpan
{
    inline constexpr uint64_t size() const SKR_NOEXCEPT { return size_; }
    inline SKR_CONSTEXPR T* data() const SKR_NOEXCEPT { return data_; }
    inline SKR_CONSTEXPR T& operator[](uint64_t index) const SKR_NOEXCEPT { return data_[index]; }
    inline SKR_CONSTEXPR T* begin() const SKR_NOEXCEPT { return data_; }
    inline SKR_CONSTEXPR T* end() const SKR_NOEXCEPT { return data_ + size_; }
    inline SKR_CONSTEXPR bool empty() const SKR_NOEXCEPT { return size_ == 0; }
    T* data_ = nullptr;
    uint64_t size_ = 0;
};

enum class EGDIBackend
{
    NANOVG,
    Count
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

enum class EGDIResourceState : uint32_t
{
    Requsted     =    0x00000001,
    Loading      =    0x00000002,
    Initializing =    0x00000004,
    Okay         =    0x00000008,
    Finalizing   =    0x00000010,
    Count = 5
};

struct SKR_GUI_API SGDIResource
{
    virtual ~SGDIResource() SKR_NOEXCEPT = default;
    virtual EGDIResourceState get_state() const SKR_NOEXCEPT = 0;
};

struct SGDIElementDrawCommand
{
    SGDITextureId texture = nullptr;
    SGDIMaterialId material = nullptr;
    uint32_t first_index = 0;
    uint32_t index_count = 0;
};

struct SKR_GUI_API SGDIPaint
{
    virtual ~SGDIPaint() SKR_NOEXCEPT = default;

    virtual void set_pattern(float cx, float cy, float w, float h, float angle, SGDITextureId texture, skr_float4_t ocol) SKR_NOEXCEPT = 0;
    virtual void set_pattern(float cx, float cy, float w, float h, float angle, SGDIMaterialId material, skr_float4_t ocol) SKR_NOEXCEPT = 0;
};

struct SKR_GUI_API SGDIElement
{
    friend struct IGDIRenderer;
    virtual ~SGDIElement() SKR_NOEXCEPT = default;
    
    virtual void begin_frame(float devicePixelRatio) = 0;
    virtual void begin_path() = 0;
    virtual void rect(float x, float y, float w, float h) = 0;
    virtual void rounded_rect_varying(float x, float y, float w, float h, float radTopLeft, float radTopRight, float radBottomRight, float radBottomLeft) = 0;
    
    virtual void move_to(float x, float y) = 0;
    virtual void line_to(float x, float y) = 0;
    
    virtual void stroke_color(uint32_t r, uint32_t g, uint32_t b, uint32_t a) = 0;
    virtual void stroke_color(float r, float g, float b, float a) = 0;
    virtual void stroke_width(float size) = 0;
    virtual void stroke() = 0;

    virtual void fill_color(uint32_t r, uint32_t g, uint32_t b, uint32_t a) = 0;
    virtual void fill_color(float r, float g, float b, float a) = 0;
    virtual void fill_paint(SGDIPaint* paint) = 0;
    virtual void fill() = 0;

    virtual void set_z(int32_t z) = 0;
    virtual int32_t get_z() const = 0;
};

struct SKR_GUI_API SGDIRenderGroup
{
    virtual ~SGDIRenderGroup() SKR_NOEXCEPT = default;

    virtual void add_element(SGDIElement* element) SKR_NOEXCEPT = 0;
    virtual void remove_element(SGDIElement* element) SKR_NOEXCEPT = 0;
    virtual LiteSpan<SGDIElement*> all_elements() SKR_NOEXCEPT = 0;

    virtual void set_zrange(int32_t min, int32_t max) SKR_NOEXCEPT = 0;
    virtual void get_zrange(int32_t* out_min, int32_t* out_max) SKR_NOEXCEPT = 0;

    virtual void enable_hardware_z() SKR_NOEXCEPT = 0;
    virtual void disable_hardware_z() SKR_NOEXCEPT = 0;
    virtual bool is_hardware_z_enabled() const SKR_NOEXCEPT = 0;

    skr_float2_t pivot = { 0.f, 0.f };
    skr_float2_t size = { 0.f, 0.0f };
};

struct SKR_GUI_API SGDICanvas
{
    virtual ~SGDICanvas() SKR_NOEXCEPT = default;

    virtual void add_render_group(SGDIRenderGroup* group) SKR_NOEXCEPT = 0;
    virtual void remove_render_group(SGDIRenderGroup* group) SKR_NOEXCEPT = 0;
    virtual LiteSpan<SGDIRenderGroup*> all_render_groups() SKR_NOEXCEPT = 0;
};

struct SKR_GUI_API SGDIDevice
{
    virtual ~SGDIDevice() SKR_NOEXCEPT = default;

    [[nodiscard]] static SGDIDevice* Create(EGDIBackend backend);
    static void Free(SGDIDevice* device);

    [[nodiscard]] virtual SGDIRenderGroup* create_render_group();
    virtual void free_render_group(SGDIRenderGroup* canvas);

    [[nodiscard]] virtual SGDICanvas* create_canvas();
    virtual void free_canvas(SGDICanvas* render_group);

    [[nodiscard]] virtual SGDIElement* create_element() = 0;
    virtual void free_element(SGDIElement* element) = 0;

    [[nodiscard]] virtual SGDIPaint* create_paint() = 0;
    virtual void free_paint(SGDIPaint* paint) = 0;
};

} }

SKR_DECLARE_TYPE_ID(skr::gdi::SGDIDevice, skr_gdi_device);