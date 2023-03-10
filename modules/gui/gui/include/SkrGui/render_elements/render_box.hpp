#pragma once
#include "SkrGui/render_elements/element.hpp"

namespace skr {
namespace gui {

struct SKR_GUI_API RenderBox : public RenderElement
{
public:
    RenderBox();
    virtual ~RenderBox();

    virtual void layout(struct Constraints* constraints, bool needSize = false) override;
    virtual void draw(const DrawParams* params) override;

    virtual skr_float2_t get_size() const;
    virtual void set_size(const skr_float2_t& size);
};

} }

SKR_DECLARE_TYPE_ID(skr::gui::RenderBox, skr_gui_render_box);