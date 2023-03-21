#pragma once 
#include "SkrGui/gdi/gdi.hpp"
#include "SkrGui/interface/gdi_renderer.hpp"

typedef struct skr_gdi_viewport_render_params_t {
    void* usr_data SKR_IF_CPP(= nullptr);
} skr_gdi_viewport_render_params_t;

namespace skr {
namespace gdi {

struct GDIRendererDescriptor
{
    void* usr_data = nullptr;
};

using ViewportRenderParams = skr_gdi_viewport_render_params_t;

enum class EGDIImageFormat
{
    None = 0,
    RGB8 = 1,
    RGBA8 = 2,
    LA8 = 3,
    R8 = 4,
    Count
};

enum class EGDITextureType : uint32_t
{
    Texture2D,
    Texture2DArray,
    Atlas,
    Count
};

enum class EGDIImageSource : uint32_t
{
    File,
    Data,
    Count
};

enum class EGDITextureSource : uint32_t
{
    File,
    Image,
    Data,
    Count
};

typedef struct GDIImageDescriptor
{
    EGDIImageSource source;
    EGDIImageFormat format;
    union
    {
        struct
        {
            const uint8_t* data = nullptr;
            uint64_t size = 0;
            uint32_t w = 0;
            uint32_t h = 0;
            uint32_t mip_count = 0;
        } from_data;
        struct
        {
            const char* u8Uri = nullptr;
        } from_file;
    };
    void* usr_data = nullptr;
} GDIImageDescriptor;

typedef struct GDITextureDescriptor
{
    EGDITextureSource source;
    EGDIImageFormat format;
    union
    {
        struct 
        {
            IGDIImage* image = nullptr;
        } from_image;
        struct
        {
            const uint8_t* data = nullptr;
            uint64_t size = 0;
            uint32_t w = 0;
            uint32_t h = 0;
            uint32_t mip_count = 0;
        } from_data;
        struct
        {
            const char* u8Uri = nullptr;
            uint32_t w = 0;
            uint32_t h = 0;
            uint32_t mip_count = 0;
        } from_file;
    };
    void* usr_data = nullptr;
} GDITextureDescriptor;

typedef struct GDITextureUpdateDescriptor
{
    IGDITexture* texture = nullptr;
    IGDIImage* image = nullptr;
} GDITextureUpdateDescriptor;

struct SKR_GUI_API IGDIImage : public GDIResource
{
    virtual ~IGDIImage() SKR_NOEXCEPT = default;
    virtual GDIRendererId get_renderer() const SKR_NOEXCEPT = 0;
    virtual uint32_t get_width() const SKR_NOEXCEPT = 0;
    virtual uint32_t get_height() const SKR_NOEXCEPT = 0;
    virtual LiteSpan<const uint8_t> get_data() const SKR_NOEXCEPT = 0;
    virtual EGDIImageFormat get_format() const SKR_NOEXCEPT = 0;
};

struct SKR_GUI_API IGDITextureUpdate : public GDIResource
{
    virtual ~IGDITextureUpdate() SKR_NOEXCEPT = default;
};

struct SKR_GUI_API IGDITexture : public GDIResource
{
    virtual ~IGDITexture() SKR_NOEXCEPT = default;
    virtual GDIRendererId get_renderer() const SKR_NOEXCEPT = 0;
    virtual uint32_t get_width() const SKR_NOEXCEPT = 0;
    virtual uint32_t get_height() const SKR_NOEXCEPT = 0;

    virtual EGDITextureType get_type() const SKR_NOEXCEPT = 0;
};

struct SKR_GUI_API IGDIRenderer
{
    virtual ~IGDIRenderer() SKR_NOEXCEPT = default;

    virtual LiteSpan<GDIVertex> fetch_element_vertices(GDIElement* element) SKR_NOEXCEPT;
    virtual LiteSpan<index_t> fetch_element_indices(GDIElement* element) SKR_NOEXCEPT;
    virtual LiteSpan<GDIElementDrawCommand> fetch_element_draw_commands(GDIElement* element) SKR_NOEXCEPT;

    // Tier 1
    virtual int initialize(const GDIRendererDescriptor* desc) SKR_NOEXCEPT = 0;
    virtual int finalize() SKR_NOEXCEPT = 0;
    [[nodiscard]] virtual GDIImageId create_image(const GDIImageDescriptor* descriptor) SKR_NOEXCEPT = 0;
    [[nodiscard]] virtual GDITextureId create_texture(const GDITextureDescriptor* descriptor) SKR_NOEXCEPT = 0;
    [[nodiscard]] virtual GDITextureUpdateId update_texture(const GDITextureUpdateDescriptor* descriptor) SKR_NOEXCEPT = 0;
    virtual void free_image(GDIImageId image) SKR_NOEXCEPT = 0;
    virtual void free_texture(GDITextureId texture) SKR_NOEXCEPT = 0;
    virtual void free_texture_update(IGDITextureUpdate* update) SKR_NOEXCEPT = 0;
    virtual void render(GDIViewport* render_group, const ViewportRenderParams* params) SKR_NOEXCEPT = 0;

    // Tier 2
    virtual bool support_hardware_z(float* out_min, float* max) const SKR_NOEXCEPT = 0;
    virtual bool support_mipmap_generation() const SKR_NOEXCEPT = 0;

    // Tier 3
};

} }

SKR_DECLARE_TYPE_ID(skr::gdi::IGDIRenderer, skr_gdi_renderer);