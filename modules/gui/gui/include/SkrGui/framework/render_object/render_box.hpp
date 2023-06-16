#pragma once
#include "SkrGui/framework/render_object/render_object.hpp"
#include "SkrGui/math/layout.hpp"

SKR_DECLARE_TYPE_ID_FWD(skr::gdi, IGDIDevice, skr_gdi_device)
SKR_DECLARE_TYPE_ID_FWD(skr::gdi, IGDIElement, skr_gdi_element)

namespace skr::gui
{

struct HitTestRecord {
};

struct SKR_GUI_API RenderBox : public RenderObject {
    SKR_GUI_TYPE(RenderBox, "01a2eb19-1299-4069-962f-88db0c719134", RenderObject);

public:
    RenderBox(skr_gdi_device_id gdi_device);
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

    skr_gdi_device_id  gdi_device = nullptr;
    skr_gdi_element_id debug_element = nullptr;
};

} // namespace skr::gui

SKR_DECLARE_TYPE_ID(skr::gui::RenderBox, skr_gui_render_box);