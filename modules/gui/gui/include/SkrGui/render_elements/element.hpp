#pragma once
#include "SkrGui/module.configure.h"
#include "SkrGui/fwd_containers.hpp"
#include "utils/types.h"

SKR_DECLARE_TYPE_ID_FWD(skr::gdi, SGDICanvas, skr_gdi_canvas)

namespace skr {
namespace gui {

struct SKR_GUI_API RenderElement
{
public:
    RenderElement();
    virtual ~RenderElement();

    virtual void set_parent(RenderElement* parent);
    virtual void add_child(RenderElement* child);
    virtual void insert_child(RenderElement* child, int index);
    virtual int get_child_index(RenderElement* child);
    virtual void remove_child(RenderElement* child);
    virtual void set_render_matrix(const skr_float4x4_t& matrix);

    virtual void set_active(bool active);

    virtual void layout(struct Constraints* constraints, bool needSize = false) = 0;
    virtual void markLayoutDirty();

    virtual void draw(skr_gdi_canvas_id canvas) = 0;

protected:
    bool active = true;
    bool layoutDirty = true;
    RenderElement* parent = nullptr;
    VectorStorage<RenderElement*> children;
    skr_float4x4_t render_matrix;
};

} // namespace gui
} // namespace skr