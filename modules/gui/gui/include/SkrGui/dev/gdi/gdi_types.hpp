#pragma once
#include "SkrGui/fwd_config.hpp"

// fwd
namespace skr::gdi
{
struct GDICanvas;
struct GDIDevice;
struct GDIPaint;
struct IGDIRenderer;
struct GDIResource;
struct IGDIImage;
struct IGDITextureUpdate;
struct IGDITexture;
struct IGDIMaterial;
struct GDIText;
struct GDIViewport;
struct GDIElement;
} // namespace skr::gdi

// basic types
namespace skr::gdi
{

struct GDIElementDrawCommand {
    IGDITexture*  texture = nullptr;
    IGDIMaterial* material = nullptr;
    uint32_t      first_index = 0;
    uint32_t      index_count = 0;
    skr_float4_t  texture_swizzle = { 0.0f, 0.0f, 0.0f, 0.0f };
};

struct GDIVertex {
    skr_float4_t position;
    skr_float2_t texcoord;
    skr_float2_t aa;
    skr_float2_t clipUV; // uv in clip-space
    skr_float2_t clipUV2;
    uint32_t     color;
};

enum class EGDIBackend
{
    NANOVG,
    Count
};

using GDIIndex = uint16_t;
using CustomVertexPainter = void (*)(GDIVertex* pVertex, void* userdata);

template <typename T>
using Span = skr::gui::Span<T>;

} // namespace skr::gdi

// resource types
namespace skr::gdi
{
struct IGDIImage;
struct IGDITexture;

enum class EGDIResourceState : uint32_t
{
    Requested = 0x00000001,
    Loading = 0x00000002,
    Initializing = 0x00000004,
    Okay = 0x00000008,
    Finalizing = 0x00000010,
    Count = 5
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

struct ViewportRenderParams {
    void* usr_data = nullptr;
};

struct GDIRendererDescriptor {
    void* usr_data = nullptr;
};

struct GDIImageDescriptor {
    EGDIImageSource source;
    EGDIImageFormat format;
    union
    {
        struct
        {
            const uint8_t* data = nullptr;
            uint64_t       size = 0;
            uint32_t       w = 0;
            uint32_t       h = 0;
            uint32_t       mip_count = 0;
        } from_data;
        struct
        {
            const char8_t* u8Uri = nullptr;
        } from_file;
    };
    void* usr_data = nullptr;
};

struct GDITextureDescriptor {
    EGDITextureSource source;
    EGDIImageFormat   format;
    union
    {
        struct
        {
            IGDIImage* image = nullptr;
        } from_image;
        struct
        {
            const uint8_t* data = nullptr;
            uint64_t       size = 0;
            uint32_t       w = 0;
            uint32_t       h = 0;
            uint32_t       mip_count = 0;
        } from_data;
        struct
        {
            const char8_t* u8Uri = nullptr;
            uint32_t       w = 0;
            uint32_t       h = 0;
            uint32_t       mip_count = 0;
        } from_file;
    };
    void* usr_data = nullptr;
};

struct GDITextureUpdateDescriptor {
    IGDITexture* texture = nullptr;
    IGDIImage*   image = nullptr;
};
} // namespace skr::gdi