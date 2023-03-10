#include "SkrGui/interface/window.hpp"
#include "SkrGui/window_context.hpp"
#include "SkrGui/interface/gdi_renderer.hpp"
#include "SkrGui/render_elements/render_window.hpp"
#include "platform/memory.h"

namespace skr {
namespace gui {

IPlatformWindow::~IPlatformWindow() SKR_NOEXCEPT
{

}

struct WindowContextImpl : public WindowContext
{
    IPlatformWindow* get_platform_window() const SKR_NOEXCEPT final
    {
        return platform_window;
    }
    
    void set_root_element(struct RenderWindow* root) SKR_NOEXCEPT final
    {
        root_element = root;
    }   

    RenderWindow* get_root_element() const SKR_NOEXCEPT final
    {
        return root_element;
    }

    void draw(const DrawParams* params) SKR_NOEXCEPT final;
    void render(skr_gdi_renderer_id renderer, const RenderParams* params) SKR_NOEXCEPT final;

    // impl helpers

    bool initialize(const WindowContextDescriptor* desc) SKR_NOEXCEPT
    {
        platform_window = desc->platform_window;
        root_element = desc->root_window;
        gdi_device = desc->gdi_device;
        return true;
    }

    bool finalize()
    {
        return true;
    }

protected:
    RenderWindow* root_element = nullptr;
    IPlatformWindow* platform_window = nullptr;
    skr_gdi_device_id gdi_device = nullptr;
};

void WindowContextImpl::draw(const DrawParams* params) SKR_NOEXCEPT
{
    if (root_element && root_element->get_gdi_viewport())
    {
        root_element->draw(nullptr, nullptr);
    }
}

void WindowContextImpl::render(skr_gdi_renderer_id renderer, const RenderParams* params) SKR_NOEXCEPT
{
    if (root_element && root_element->get_gdi_viewport())
    {
        renderer->render(root_element->get_gdi_viewport(), params->gdi_params);
    }
}

WindowContext* WindowContext::Create(const WindowContextDescriptor* desc) SKR_NOEXCEPT
{
    WindowContextImpl* context = SkrNew<WindowContextImpl>();
    if (context->initialize(desc))
    {
        return context;    
    }
    else
    {
        SkrDelete(context);
        return nullptr;
    }
}

void WindowContext::Free(WindowContext* context) SKR_NOEXCEPT
{
    WindowContextImpl* impl = static_cast<WindowContextImpl*>(context);
    impl->finalize();
    SkrDelete(impl);
}

} }