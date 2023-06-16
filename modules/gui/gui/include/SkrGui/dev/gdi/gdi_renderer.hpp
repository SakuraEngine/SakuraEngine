#pragma once
#include "SkrGui/dev/gdi/gdi_types.hpp"

namespace skr::gdi
{
struct SKR_GUI_API IGDIRenderer {
    virtual ~IGDIRenderer() SKR_NOEXCEPT = default;

    virtual Span<GDIVertex>             fetch_element_vertices(IGDIElement* element) SKR_NOEXCEPT;
    virtual Span<GDIIndex>              fetch_element_indices(IGDIElement* element) SKR_NOEXCEPT;
    virtual Span<GDIElementDrawCommand> fetch_element_draw_commands(IGDIElement* element) SKR_NOEXCEPT;

    // Tier 1
    virtual int                              initialize(const GDIRendererDescriptor* desc) SKR_NOEXCEPT = 0;
    virtual int                              finalize() SKR_NOEXCEPT = 0;
    [[nodiscard]] virtual IGDIImage*         create_image(const GDIImageDescriptor* descriptor) SKR_NOEXCEPT = 0;
    [[nodiscard]] virtual IGDITexture*       create_texture(const GDITextureDescriptor* descriptor) SKR_NOEXCEPT = 0;
    [[nodiscard]] virtual IGDITextureUpdate* update_texture(const GDITextureUpdateDescriptor* descriptor) SKR_NOEXCEPT = 0;
    virtual void                             free_image(IGDIImage* image) SKR_NOEXCEPT = 0;
    virtual void                             free_texture(IGDITexture* texture) SKR_NOEXCEPT = 0;
    virtual void                             free_texture_update(IGDITextureUpdate* update) SKR_NOEXCEPT = 0;
    virtual void                             render(IGDIViewport* render_group, const ViewportRenderParams* params) SKR_NOEXCEPT = 0;

    // Tier 2
    virtual bool support_hardware_z(float* out_min, float* max) const SKR_NOEXCEPT = 0;
    virtual bool support_mipmap_generation() const SKR_NOEXCEPT = 0;

    // Tier 3
};

} // namespace skr::gdi