#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#include "SkrGui/framework/render_object/render_object.hpp"
#ifndef __meta__
    #include "SkrGui/framework/render_object/single_child_render_object.generated.h"
#endif

namespace skr::gui
{
sreflect_interface(
    "guid": "5349672b-bfc5-46a9-9a02-40ef563c196d"
)
SKR_GUI_API ISingleChildRenderObject : virtual public skr::rttr::IObject {
    SKR_GENERATE_BODY()
    virtual ~ISingleChildRenderObject() = default;

    virtual GUID accept_child_type() const SKR_NOEXCEPT               = 0;
    virtual void set_child(NotNull<RenderObject*> child) SKR_NOEXCEPT = 0;
    virtual void remove_child() SKR_NOEXCEPT                          = 0;
};

template <typename TSelf, typename TChild>
struct SingleChildRenderObjectMixin {
    TChild* _child;

    inline GUID accept_child_type(const TSelf& self) const SKR_NOEXCEPT
    {
        return ::skr::rttr::type_id_of<TChild>();
    }
    inline void set_child(TSelf& self, NotNull<RenderObject*> child) SKR_NOEXCEPT
    {
        if (_child) _child->unmount();
        _child = child->type_cast_fast<TChild>();
        _child->mount(&self);
    }
    inline void remove_child(TSelf& self) SKR_NOEXCEPT
    {
        if (_child) _child->unmount();
        _child = nullptr;
    }
    inline void visit_children(const TSelf& self, RenderObject::VisitFuncRef visitor) const SKR_NOEXCEPT
    {
        if (_child) visitor(_child);
    }
};
} // namespace skr::gui

#define SKR_GUI_SINGLE_CHILD_RENDER_OBJECT_MIXIN(__SELF, __CHILD)                \
    /*===============> Begin Single Child Render Object Mixin <===============*/ \
private:                                                                         \
    SingleChildRenderObjectMixin<__SELF, __CHILD>                                \
    _single_child_render_object_mixin = {};                                      \
                                                                                 \
public:                                                                          \
    GUID accept_child_type() const SKR_NOEXCEPT override                         \
    {                                                                            \
        return _single_child_render_object_mixin.accept_child_type(*this);       \
    }                                                                            \
    void set_child(NotNull<RenderObject*> child) SKR_NOEXCEPT override           \
    {                                                                            \
        _single_child_render_object_mixin.set_child(*this, child);               \
    }                                                                            \
    void remove_child() SKR_NOEXCEPT override                                    \
    {                                                                            \
        _single_child_render_object_mixin.remove_child(*this);                   \
    }                                                                            \
    void visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT override        \
    {                                                                            \
        _single_child_render_object_mixin.visit_children(*this, visitor);        \
    }                                                                            \
    inline __CHILD* child() const SKR_NOEXCEPT                                   \
    {                                                                            \
        return _single_child_render_object_mixin._child;                         \
    }                                                                            \
    /*===============> End Single Child Render Object Mixin <===============*/