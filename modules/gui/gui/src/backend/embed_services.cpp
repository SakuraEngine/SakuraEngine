#include "SkrGui/backend/embed_services.hpp"
#include "SkrGui/backend/canvas/canvas.hpp"
#include "backend/text_server/text_server_adv.h"
#include "backend/embedded_text/paragraph.hpp"

namespace godot
{
TextServerAdvanced*& _text_server()
{
    static TextServerAdvanced* text_server = nullptr;
    return text_server;
}
TextServer* get_text_server()
{
    return _text_server();
}
} // namespace godot

namespace skr::gui
{
// canvas
NotNull<ICanvas*> embedded_create_canvas() SKR_NOEXCEPT
{
    return make_not_null(SkrNew<ICanvas>());
}
void embedded_destroy_canvas(NotNull<ICanvas*> canvas) SKR_NOEXCEPT
{
    SkrDelete(canvas.get());
}

// text
void embedded_init_text_service(INativeDevice* native_device)
{
    godot::SkrGuiData data;
    data.resource_service = native_device;
    godot::_text_server() = SkrNew<godot::TextServerAdvanced>(data);
}
NotNull<IParagraph*> embedded_create_paragraph()
{
    return make_not_null(SkrNew<_EmbeddedParagraph>());
}
void embedded_destroy_paragraph(NotNull<IParagraph*> paragraph)
{
    SkrDelete(paragraph.get());
}
void embedded_shutdown_text_service()
{
    SkrDelete(godot::_text_server());
}
} // namespace skr::gui