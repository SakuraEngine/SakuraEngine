#pragma once
#include "SkrGui/render_elements/element.hpp"

namespace skr
{
namespace gui
{
class RenderText : public RenderElement
{
public:
    RenderText();
    virtual ~RenderText();

    virtual void layout(struct Constraints* constraints, bool needSize = false) override;
    virtual void draw(gdi::SGDICanvas* canvas) override;

    virtual skr_float2_t get_size() const;
    virtual void set_size(const skr_float2_t& size);
};
}
}