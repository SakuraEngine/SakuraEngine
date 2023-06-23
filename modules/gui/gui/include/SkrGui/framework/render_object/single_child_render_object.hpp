#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#include "SkrGui/framework/render_object/render_object.hpp"

namespace skr::gui
{
struct SKR_GUI_API ISingleChildRenderObject SKR_GUI_INTERFACE_BASE {
    SKR_GUI_INTERFACE_ROOT(ISingleChildRenderObject, "5349672b-bfc5-46a9-9a02-40ef563c196d")
    virtual ~ISingleChildRenderObject() = default;

    virtual SKR_GUI_TYPE_ID accept_child_type() const SKR_NOEXCEPT = 0;
    virtual void            set_child(RenderObject* child) SKR_NOEXCEPT = 0;
};

template <typename TSelf, typename TChild>
struct SingleChildRenderObjectMixin {
    TChild* _child;

    inline SKR_GUI_TYPE_ID accept_child_type(const TSelf& self) const SKR_NOEXCEPT
    {
        return SKR_GUI_TYPE_ID_OF_STATIC(TSelf);
    }
    inline void set_child(TSelf& self, RenderObject* child) SKR_NOEXCEPT
    {
        if (_child) _child->unmount();
        _child = child->type_cast_fast<TChild>();
        if (_child) _child->mount(make_not_null(&self));
    }
    inline void visit_children(const TSelf& self, RenderObject::VisitFuncRef visitor) const SKR_NOEXCEPT
    {
        if (_child) visitor(make_not_null(_child));
    }
};

#define SKR_GUI_SINGLE_CHILD_RENDER_OBJECT_MIXIN(__SELF, __CHILD)                    \
    /*===============> Begin Single Child Render Object Mixin <===============*/     \
private:                                                                             \
    SingleChildRenderObjectMixin<__SELF, __CHILD> _single_child_render_object_mixin; \
                                                                                     \
public:                                                                              \
    SKR_GUI_TYPE_ID accept_child_type() const noexcept override                      \
    {                                                                                \
        return _single_child_render_object_mixin.accept_child_type(*this);           \
    }                                                                                \
    void set_child(RenderObject* child) noexcept override                            \
    {                                                                                \
        _single_child_render_object_mixin.set_child(*this, child);                   \
    }                                                                                \
    void visit_children(VisitFuncRef visitor) const noexcept override                \
    {                                                                                \
        _single_child_render_object_mixin.visit_children(*this, visitor);            \
    }                                                                                \
    inline __CHILD* child() const noexcept                                           \
    {                                                                                \
        return _single_child_render_object_mixin._child;                             \
    }                                                                                \
    /*===============> End Single Child Render Object Mixin <===============*/

} // namespace skr::gui