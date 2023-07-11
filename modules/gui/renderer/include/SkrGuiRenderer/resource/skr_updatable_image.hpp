#pragma once
#include "SkrGuiRenderer/module.configure.h"
#include "SkrGui/backend/resource/resource.hpp"
#include "cgpu/api.h"
#include "cgpu/cgpux.h"

namespace skr::gui
{
struct SkrRenderDevice;
struct SKR_GUI_RENDERER_API SkrUpdatableImage final : public IUpdatableImage {

    SkrUpdatableImage(SkrRenderDevice* render_device);
    ~SkrUpdatableImage();

    EResourceState            state() const SKR_NOEXCEPT override;
    void                      request() override;
    void                      cancel_request() override;
    void                      destroy() override;
    Sizei                     size() const SKR_NOEXCEPT override;
    Rectf                     uv_rect() const SKR_NOEXCEPT override;
    EdgeInsetsf               nine_inset() const SKR_NOEXCEPT override;
    void                      update(const UpdatableImageDesc& desc) override;
    const UpdatableImageDesc& desc() const SKR_NOEXCEPT override { return _desc; }

    CGPUTextureId     texture() const SKR_NOEXCEPT { return _texture; }
    CGPUTextureViewId texture_view() const SKR_NOEXCEPT { return _texture_view; }
    CGPUXBindTableId  bind_table() const SKR_NOEXCEPT { return _bind_table; }

private:
    SkrRenderDevice* _render_device = nullptr;

    CGPUTextureId     _cgpu_texture = nullptr;
    CGPUTextureId     _texture      = nullptr;
    CGPUTextureViewId _texture_view = nullptr;
    CGPUXBindTableId  _bind_table   = nullptr;

    UpdatableImageDesc _desc = {};
};
} // namespace skr::gui