#include "SkrGuiRenderer/resource/skr_updatable_image.hpp"
#include "SkrGuiRenderer/render/skr_render_device.hpp"

namespace skr::gui
{
SkrUpdatableImage::SkrUpdatableImage(SkrRenderDevice* render_device)
    : _render_device(render_device)
{
}
SkrUpdatableImage::~SkrUpdatableImage()
{
    if (_cgpu_texture)
    {
        cgpu_free_texture(_cgpu_texture);
    }
}

EResourceState SkrUpdatableImage::state() const SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION()
    return EResourceState::Entry;
}
void SkrUpdatableImage::request()
{
    SKR_UNIMPLEMENTED_FUNCTION()
}
void SkrUpdatableImage::cancel_request()
{
    SKR_UNIMPLEMENTED_FUNCTION()
}
void SkrUpdatableImage::destroy(){
    SKR_UNIMPLEMENTED_FUNCTION()

} Sizei SkrUpdatableImage::size() const SKR_NOEXCEPT
{
    return _desc.size;
}
Rectf SkrUpdatableImage::uv_rect() const SKR_NOEXCEPT
{
    return Rectf{ 0, 0, 1, 1 };
}
EdgeInsetsf SkrUpdatableImage::nine_inset() const SKR_NOEXCEPT
{
    return { 0, 0, 0, 0 };
}
void SkrUpdatableImage::update(const UpdatableImageDesc& desc)
{
    if (_cgpu_texture && (desc.size != _desc.size || _desc.format != desc.format))
    {
        cgpu_free_texture(_cgpu_texture);
        _cgpu_texture = nullptr;
    }
}
} // namespace skr::gui