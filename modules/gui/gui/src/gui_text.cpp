#include "SkrGui/dev/gdi/gdi.hpp"
#include "dev/text_server/text_server_adv.h"
#include "containers/sptr.hpp"

namespace skr
{
namespace gdi
{

struct GDITextImpl : public IGDIText {
    GDITextImpl(gdi::IGDIRenderer* renderer)
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

    static GDITextImpl*        this_;
    godot::TextServerAdvanced* text_server = nullptr;
    gdi::IGDIRenderer*         pRenderer = nullptr;
};

GDITextImpl* GDITextImpl::this_ = nullptr;
bool         IGDIText::Initialize(gdi::IGDIRenderer* renderer)
{
    if (auto gdi_text = SkrNew<GDITextImpl>(renderer))
    {
        GDITextImpl::this_ = gdi_text;
        return GDITextImpl::this_;
    }
    return true;
}

bool IGDIText::Finalize()
{
    SkrDelete(GDITextImpl::this_);
    return true;
}

IGDIText* IGDIText::Get()
{
    return GDITextImpl::this_;
}

} // namespace gdi
} // namespace skr

godot::TextServer* godot::get_text_server()
{
    if (auto gdi_text = static_cast<skr::gdi::GDITextImpl*>(skr::gdi::IGDIText::Get()))
    {
        return gdi_text->text_server;
    }
    return nullptr;
}
