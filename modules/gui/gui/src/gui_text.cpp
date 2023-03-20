#include "SkrGui/gdi/gdi.hpp"
#include "text_server/text_server_adv.h"
#include "containers/sptr.hpp"

namespace skr {
namespace gdi {

struct GDITextImpl : public GDIText
{
    GDITextImpl(skr_gdi_renderer_id renderer)
        : pRenderer(renderer)
    {
        godot::SkrGuiData data = {};
        data.gdi_renderer = renderer;
        text_server = SkrNew<godot::TextServerAdvanced>(data);
    }

    ~GDITextImpl()
    {
        SkrDelete(text_server);
    }

    static GDITextImpl* this_;
    godot::TextServerAdvanced* text_server = nullptr;
    skr_gdi_renderer_id pRenderer = nullptr;
};

GDITextImpl* GDITextImpl::this_ = nullptr;
bool GDIText::Initialize(skr_gdi_renderer_id renderer)
{
    if (auto gdi_text = SkrNew<GDITextImpl>(renderer))
    {
        GDITextImpl::this_ = gdi_text;
        return GDITextImpl::this_;
    }
    return true;
}

bool GDIText::Finalize()
{
    SkrDelete(GDITextImpl::this_);
    return true;
}

GDIText* GDIText::Get()
{
    return GDITextImpl::this_;
}

} }

godot::TextServer* godot::get_text_server()
{
    if (auto gdi_text = static_cast<skr::gdi::GDITextImpl*>(skr::gdi::GDIText::Get()))
    {
        return gdi_text->text_server;
    }
    return nullptr;
}
