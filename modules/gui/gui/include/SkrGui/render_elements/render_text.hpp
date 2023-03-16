#pragma once
#include "SkrGui/framework/render_box.hpp"

namespace skr {
namespace gui {
    
struct SKR_GUI_API RenderText : public RenderBox
{
public:
    RenderText(skr_gdi_device_id gdi_device);
    virtual ~RenderText();

    virtual void layout(BoxConstraint constraints, bool needSize = false) override;
    virtual void draw(const DrawParams* params) override;
};

} }

SKR_DECLARE_TYPE_ID(skr::gui::RenderText, skr_gui_render_text);