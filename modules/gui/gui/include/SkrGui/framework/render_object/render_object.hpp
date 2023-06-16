#pragma once
#include "SkrGui/framework/diagnostics.hpp"

namespace skr::gdi
{
struct IGDICanvas;
struct IGDIElement;
struct IGDIViewport;
} // namespace skr::gdi

namespace skr::gui
{
struct WindowContext;
struct SKR_GUI_API RenderObject : public DiagnosticableTreeNode {
    SKR_GUI_TYPE(RenderObject, "74844fa6-8994-4915-8f8e-ec944a1cbea4", DiagnosticableTreeNode);

    struct DrawParams {
        gdi::IGDIViewport* viewport = nullptr;
        gdi::IGDICanvas*   canvas = nullptr;
        WindowContext*     window_context = nullptr;
        int32_t            ui_z = 0;
    };

public:
    RenderObject();
    virtual ~RenderObject();

    virtual void          set_parent(RenderObject* parent);
    virtual void          add_child(RenderObject* child);
    virtual void          insert_child(RenderObject* child, int index);
    virtual int           get_child_index(RenderObject* child);
    virtual void          remove_child(RenderObject* child);
    virtual int           get_child_count() const;
    virtual RenderObject* get_child(int index) const;
    virtual void          set_render_matrix(const skr_float4x4_t& matrix);

    virtual void set_active(bool active);
    virtual void markLayoutDirty();

    virtual void before_draw(const DrawParams* params);
    virtual void draw(const DrawParams* params);
    virtual void after_draw(const DrawParams* params);

    virtual Span<DiagnosticableTreeNode* const> get_diagnostics_children() const override;

protected:
    void addElementToCanvas(const DrawParams* params, gdi::IGDIElement* element);

    bool                 active = true;
    bool                 layoutDirty = true;
    RenderObject*        parent = nullptr;
    Array<RenderObject*> children;
    skr_float4x4_t       render_matrix;
};

} // namespace skr::gui