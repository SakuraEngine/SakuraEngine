#include "backend/embedded_text/text_service.hpp"
#include "SkrGui/backend/embed_services.hpp"
#include "backend/text_server/text_server_adv.h"
#include "containers/sptr.hpp"
#include "backend/embedded_text/paragraph.hpp"

namespace godot
{
TextServerAdvanced*& _text_server();
}

namespace skr::gui
{
_EmbeddedTextService::_EmbeddedTextService(IResourceService* resource_service)
{
    godot::SkrGuiData data;
    data.resource_service = resource_service;
    godot::_text_server() = SkrNew<godot::TextServerAdvanced>(data);
}
_EmbeddedTextService::~_EmbeddedTextService()
{
    SkrDelete(godot::_text_server());
}

NotNull<IParagraph*> _EmbeddedTextService::create_paragraph()
{
    return make_not_null(SkrNew<_EmbeddedParagraph>(this));
}
void _EmbeddedTextService::destroy_paragraph(NotNull<IParagraph*> paragraph)
{
    SkrDelete(paragraph.get());
}

} // namespace skr::gui

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
