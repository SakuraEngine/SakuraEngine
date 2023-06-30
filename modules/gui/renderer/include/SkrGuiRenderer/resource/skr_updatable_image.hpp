#pragma once
#include "SkrGuiRenderer/module.configure.h"
#include "SkrGui/backend/resource/resource.hpp"
#include "cgpu/api.h"

namespace skr::gui
{
struct SkrRenderDevice;
struct SKR_GUI_RENDERER_API SkrUpdatableImage final : public IUpdatableImage {

    SkrUpdatableImage(SkrRenderDevice* render_device);
    ~SkrUpdatableImage();

    bool                      is_okey() const SKR_NOEXCEPT override;
    Sizei                     size() const SKR_NOEXCEPT override;
    Rectf                     uv_rect() const SKR_NOEXCEPT override;
    EdgeInsetsf               nine_inset() const SKR_NOEXCEPT override;
    void                      update(const UpdatableImageDesc& desc) override;
    const UpdatableImageDesc& desc() const SKR_NOEXCEPT override { return _desc; }

private:
    SkrRenderDevice* _render_device = nullptr;

    CGPUTextureId _cgpu_texture = nullptr;

    UpdatableImageDesc _desc;
};
} // namespace skr::gui