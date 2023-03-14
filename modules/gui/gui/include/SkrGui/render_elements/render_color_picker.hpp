#pragma once
#include "SkrGui/framework/render_element.hpp"

SKR_DECLARE_TYPE_ID_FWD(skr::gdi, GDIDevice, skr_gdi_device)
SKR_DECLARE_TYPE_ID_FWD(skr::gdi, GDIElement, skr_gdi_element)
SKR_DECLARE_TYPE_ID_FWD(skr::gdi, GDIPaint, skr_gdi_paint)

namespace skr {
namespace gui {

struct SKR_GUI_API RenderColorPicker : public RenderElement
{
public:
    RenderColorPicker(skr_gdi_device_id gdi_device);
    virtual ~RenderColorPicker();

    virtual void layout(struct Constraints* constraints, bool needSize = false) override;
    virtual void draw(const DrawParams* params) override;

    virtual skr_float2_t get_size() const;
    virtual void set_size(const skr_float2_t& size);

    void draw_color_picker(gdi::GDIElement* element, gdi::GDIPaint* paint, float x, float y, float w, float h);

    float get_current_hue_by_degree() const { return current_degree; }

protected:
    float current_degree = 0.0f;
    skr_gdi_device_id gdi_device = nullptr;
    skr_gdi_element_id gdi_element = nullptr;
    skr_gdi_paint_id gdi_paint = nullptr;
};

} }

SKR_DECLARE_TYPE_ID(skr::gui::RenderColorPicker, skr_gui_render_color_picker);