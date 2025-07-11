#include "SkrGuiRenderer/device/skr_native_device.hpp"
#include "SkrGui/backend/embed_services.hpp"
#include "SkrGuiRenderer/device/skr_native_window.hpp"
#include "SkrGuiRenderer/render/skr_render_device.hpp"
#include "SkrGuiRenderer/render/skr_render_window.hpp"
#include "SkrGuiRenderer/resource/skr_resource_device.hpp"
#include "SkrGui/framework/layer/native_window_layer.hpp"
#include "SkrGuiRenderer/resource/skr_updatable_image.hpp"

namespace skr::gui
{
void SkrNativeDevice::init()
{
    // init render device
    _render_device = SkrNew<SkrRenderDevice>();
    _render_device->init();

    // init resource service
    _resource_device = SkrNew<SkrResourceDevice>();
    _resource_device->init();

    // get display metrics
    int   display_count;
    auto* displays = SDL_GetDisplays(&display_count);
    for (int i = 0; i < display_count; ++i)
    {
        auto     display_id = displays[i];
        SDL_Rect display_bounds;
        SDL_GetDisplayBounds(display_id, &display_bounds);

        auto out = _display_metrics.monitors.add_default().ref();
        // name
        // device_id
        // native_size
        // max_resolution
        // display_area
        out.work_area = Recti::LTWH(
            display_bounds.x,
            display_bounds.y,
            display_bounds.w,
            display_bounds.h
        );
        // is_primary
        // features
    }

    // TODO. total data
    // _display_metrics.primary_display_area = _display_metrics.monitors[0].work_area;

    // init text device
    embedded_init_text_service(this);
}
void SkrNativeDevice::shutdown()
{
    // shutdown text device
    embedded_shutdown_text_service();

    // clean up all updatable images
    for (auto image : _all_updatable_images)
    {
        image->destroy();
    }
    _all_updatable_images.clear();

    // shutdown resource device
    _resource_device->shutdown();
    SkrDelete(_resource_device);

    // shutdown render device
    _render_device->shutdown();
    SkrDelete(_render_device);
}

// view
NotNull<INativeWindow*> SkrNativeDevice::create_window()
{
    auto view = SkrNew<SkrNativeWindow>(this);
    _all_windows.add(view);
    return view;
}
void SkrNativeDevice::destroy_window(NotNull<INativeWindow*> view)
{
    // erase it
    _all_windows.remove(view);

    // delete
    SkrDelete(view.get());
}

void SkrNativeDevice::render_all_windows() SKR_NOEXCEPT
{
    auto render_graph = render_device()->render_graph();

    // combine render graph
    for (auto window : _all_windows)
    {
        window->render_window()->render(window->native_layer(), window->absolute_size());
    }

    // commit render graph
    auto frame_index = render_device()->render_graph()->execute();

    // TODO: 更优雅地回收垃圾
    if (frame_index >= RG_MAX_FRAME_IN_FLIGHT * 10)
        render_graph->collect_garbage(frame_index - RG_MAX_FRAME_IN_FLIGHT * 10);

    // present
    for (auto window : _all_windows)
    {
        window->render_window()->present();
    }
}

// display info
const DisplayMetrics& SkrNativeDevice::display_metrics() const
{
    return _display_metrics;
}

// resource management
NotNull<IUpdatableImage*> SkrNativeDevice::create_updatable_image()
{
    auto image = SkrNew<SkrUpdatableImage>(_render_device);
    _all_updatable_images.add(image);
    return image;
}

// canvas management
NotNull<ICanvas*> SkrNativeDevice::create_canvas()
{
    return embedded_create_canvas();
}
void SkrNativeDevice::destroy_canvas(NotNull<ICanvas*> canvas)
{
    embedded_destroy_canvas(canvas);
}

// text management
NotNull<IParagraph*> SkrNativeDevice::create_paragraph()
{
    return embedded_create_paragraph();
}
void SkrNativeDevice::destroy_paragraph(NotNull<IParagraph*> paragraph)
{
    embedded_destroy_paragraph(paragraph);
}

} // namespace skr::gui