#pragma once
#include "SkrGui/module.configure.h"
#include "SkrGui/gdi/gdi.h"
#include "EASTL/vector.h"
#include "rtm/matrix4x4f.h"

namespace skr
{
namespace gui
{
class RenderElement
{
public:
    RenderElement();
    virtual ~RenderElement();

    virtual void set_parent(RenderElement* parent);
    virtual void add_child(RenderElement* child);
    virtual void insert_child(RenderElement* child, int index);
    virtual int get_child_index(RenderElement* child);
    virtual void remove_child(RenderElement* child);
    virtual void set_render_matrix(const rtm::matrix4x4f& matrix);

    virtual void set_active(bool active);

    virtual void layout(struct Constraints* constraints, bool needSize = false) = 0;
    virtual void markLayoutDirty();

    virtual void draw(gdi::SGDICanvas* canvas) = 0;

protected:
    bool active = true;
    bool layoutDirty = true;
    RenderElement* parent = nullptr;
    eastl::vector<RenderElement*> children;
    rtm::matrix4x4f render_matrix;
};
} // namespace gui
} // namespace skr