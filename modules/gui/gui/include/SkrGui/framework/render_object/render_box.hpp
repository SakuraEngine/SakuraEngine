#pragma once
#include "SkrGui/framework/render_object/render_object.hpp"
#include "SkrGui/math/layout.hpp"

namespace skr::gdi
{
struct IGDIDevice;
struct IGDIElement;
} // namespace skr::gdi

namespace skr::gui
{
using gdi::IGDIDevice;
using gdi::IGDIElement;

struct HitTestRecord {
};

struct SKR_GUI_API RenderBox : public RenderObject {
    SKR_GUI_TYPE(RenderBox, "01a2eb19-1299-4069-962f-88db0c719134", RenderObject);

public:
    RenderBox(IGDIDevice* gdi_device);
    virtual ~RenderBox();

    virtual void layout(BoxConstraint constraints, bool needSize = false) = 0;
    virtual void before_draw(const DrawParams* params) override;
    virtual void draw(const DrawParams* params) override;
    virtual bool hit_test(const Ray& point, HitTestRecord* record) const;

    virtual Size get_size() const;
    virtual void set_size(const Size& size);
    virtual void set_position(const Offset& position);
    RenderBox*   get_child_as_box(int index) const { return (RenderBox*)get_child(index); }

    virtual void enable_debug_draw(bool enable);

protected:
    bool   draw_debug_rect = false;
    Offset pos = { 0, 0 };
    Size   size = { 0, 0 };

    IGDIDevice*  gdi_device = nullptr;
    IGDIElement* debug_element = nullptr;
};

} // namespace skr::gui