#pragma once
#include "SkrGui/render_elements/element.hpp"

namespace skr {
namespace gui {
    
struct SKR_GUI_API RenderText : public RenderElement
{
public:
    RenderText();
    virtual ~RenderText();

    virtual void layout(struct Constraints* constraints, bool needSize = false) override;
    virtual void draw(skr_gdi_viewport_id viewport, skr_gdi_canvas_id canvas) override;

    virtual skr_float2_t get_size() const;
    virtual void set_size(const skr_float2_t& size);
};

} }

SKR_DECLARE_TYPE_ID(skr::gui::RenderText, skr_gui_render_text);