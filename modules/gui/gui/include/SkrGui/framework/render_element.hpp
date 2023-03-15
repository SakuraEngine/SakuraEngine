#pragma once
#include "SkrGui/framework/diagnostics.hpp"

SKR_DECLARE_TYPE_ID_FWD(skr::gdi, GDICanvas, skr_gdi_canvas)
SKR_DECLARE_TYPE_ID_FWD(skr::gdi, GDIElement, skr_gdi_element)
SKR_DECLARE_TYPE_ID_FWD(skr::gdi, GDIViewport, skr_gdi_viewport)
SKR_DECLARE_TYPE_ID_FWD(skr::gui, WindowContext, skr_gui_window_context)

typedef struct skr_gui_render_element_draw_params_t
{
    skr_gdi_viewport_id viewport SKR_IF_CPP(= nullptr);
    skr_gdi_canvas_id canvas SKR_IF_CPP(= nullptr);
    skr_gui_window_context_id window_context SKR_IF_CPP(= nullptr);
    int32_t ui_z SKR_IF_CPP(= 0);
} skr_gui_render_element_draw_params_t;

namespace skr {
namespace gui {

struct SKR_GUI_API RenderElement : public DiagnosticableTreeNode
{
    using DrawParams = skr_gui_render_element_draw_params_t;
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

    virtual void before_draw(const DrawParams* params);
    virtual void draw(const DrawParams* params);
    virtual void after_draw(const DrawParams* params);

    virtual LiteSpan<DiagnosticableTreeNode* const> get_diagnostics_children() const override;

protected:
    void addElementToCanvas(const DrawParams* params, gdi::GDIElement* element);

    bool active = true;
    bool layoutDirty = true;
    RenderElement* parent = nullptr;
    VectorStorage<RenderElement*> children;
    skr_float4x4_t render_matrix;
};

} // namespace gui
} // namespace skr