#include "SkrCore/memory/memory.h"
#include "SkrGui/_private/paragraph.hpp"
#include "backend/text_server_adv/text_server_adv.h"

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
// text
void embedded_init_text_service(INativeDevice* native_device)
{
    godot::SkrGuiData data;
    data.resource_service = native_device;
    godot::_text_server() = SkrNew<godot::TextServerAdvanced>(data);
}
NotNull<IParagraph*> embedded_create_paragraph()
{
    return SkrNew<_EmbeddedParagraph>();
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