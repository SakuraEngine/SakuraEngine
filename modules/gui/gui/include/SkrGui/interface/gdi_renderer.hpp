#pragma once 
#include "SkrGui/gdi/gdi.hpp"
#include "SkrGui/interface/gdi_renderer.hpp"

namespace skr {
namespace gdi {

struct SGDIRendererDescriptor
{
    void* usr_data = nullptr;
};

struct SGDIRenderParams
{
    void* usr_data = nullptr;
};

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

typedef struct SGDIImageDescriptor
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
} SGDIImageDescriptor;

typedef struct SGDITextureDescriptor
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
} SGDITextureDescriptor;

struct SKR_GUI_API IGDIImage : public SGDIResource
{
    virtual ~IGDIImage() SKR_NOEXCEPT = default;
    virtual SGDIRendererId get_renderer() const SKR_NOEXCEPT = 0;
    virtual uint32_t get_width() const SKR_NOEXCEPT = 0;
    virtual uint32_t get_height() const SKR_NOEXCEPT = 0;
    virtual LiteSpan<const uint8_t> get_data() const SKR_NOEXCEPT = 0;
    virtual EGDIImageFormat get_format() const SKR_NOEXCEPT = 0;
};

struct SKR_GUI_API IGDITexture : public SGDIResource
{
    virtual ~IGDITexture() SKR_NOEXCEPT = default;
    virtual SGDIRendererId get_renderer() const SKR_NOEXCEPT = 0;
    virtual uint32_t get_width() const SKR_NOEXCEPT = 0;
    virtual uint32_t get_height() const SKR_NOEXCEPT = 0;
    
    virtual EGDITextureType get_type() const SKR_NOEXCEPT = 0;
};

struct SKR_GUI_API IGDIRenderer
{
    virtual ~IGDIRenderer() SKR_NOEXCEPT = default;

    virtual LiteSpan<SGDIVertex> fetch_element_vertices(SGDIElement* element) SKR_NOEXCEPT;
    virtual LiteSpan<index_t> fetch_element_indices(SGDIElement* element) SKR_NOEXCEPT;
    virtual LiteSpan<SGDIElementDrawCommand> fetch_element_draw_commands(SGDIElement* element) SKR_NOEXCEPT;

    // Tier 1
    virtual int initialize(const SGDIRendererDescriptor* desc) SKR_NOEXCEPT = 0;
    virtual int finalize() SKR_NOEXCEPT = 0;
    [[nodiscard]] virtual SGDIImageId create_image(const SGDIImageDescriptor* descriptor) SKR_NOEXCEPT = 0;
    [[nodiscard]] virtual SGDITextureId create_texture(const SGDITextureDescriptor* descriptor) SKR_NOEXCEPT = 0;
    virtual void free_image(SGDIImageId image) SKR_NOEXCEPT = 0;
    virtual void free_texture(SGDITextureId texture) SKR_NOEXCEPT = 0;
    virtual void render(SGDICanvasGroup* canvas_group, SGDIRenderParams* params) SKR_NOEXCEPT = 0;

    // Tier 2
    virtual bool support_hardware_z(float* out_min, float* max) const SKR_NOEXCEPT = 0;
    virtual bool support_mipmap_generation() const SKR_NOEXCEPT = 0;

    // Tier 3
};

} }

SKR_DECLARE_TYPE_ID(skr::gdi::IGDIRenderer, skr_gdi_renderer);