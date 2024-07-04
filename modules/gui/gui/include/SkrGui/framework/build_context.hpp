#pragma once
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#ifndef __meta__
    #include "SkrGui/framework/build_context.generated.h"
#endif

namespace skr::gui
{
sreflect_interface(
    "guid": "0ff4a42c-7195-48c5-a979-263dab05ac2d"
)
SKR_GUI_API IBuildContext : virtual public skr::rttr::IObject {
    SKR_GENERATE_BODY()
    virtual ~IBuildContext() = default;
    using VisitFuncRef       = FunctionRef<bool(NotNull<Element*>)>;

    // data query
    virtual Widget*     bound_widget() const SKR_NOEXCEPT = 0;
    virtual BuildOwner* build_owner() const SKR_NOEXCEPT  = 0;
    virtual bool        is_destroyed() const SKR_NOEXCEPT = 0;

    // render object query
    virtual RenderObject*   find_render_object() const SKR_NOEXCEPT = 0;
    virtual Optional<Sizef> render_box_size() const SKR_NOEXCEPT    = 0;

    // visit
    virtual void visit_ancestor_elements(VisitFuncRef visitor) const SKR_NOEXCEPT = 0;
    virtual void visit_child_elements(VisitFuncRef visitor) const SKR_NOEXCEPT    = 0;

    // find api
    Widget*       find_ancestor_widget(const GUID& type_id, bool exact_type = false) const SKR_NOEXCEPT;
    State*        find_ancestor_state(const GUID& type_id, bool exact_type = false) const SKR_NOEXCEPT;
    RenderObject* find_ancestor_render_object() const SKR_NOEXCEPT;
    RenderObject* find_ancestor_render_object(const GUID& type_id, bool exact_type = false) const SKR_NOEXCEPT;

    // template find
    template <typename T>
    T* find_ancestor_widget(bool exact_type = false) const SKR_NOEXCEPT
    {
        return static_cast<T*>(find_ancestor_widget(rttr::type_id_of<T>(), exact_type));
    }
    template <typename T>
    T* find_ancestor_state(bool exact_type = false) const SKR_NOEXCEPT
    {
        return static_cast<T*>(find_ancestor_state(rttr::type_id_of<T>(), exact_type));
    }
    template <typename T>
    T* find_ancestor_render_object(bool exact_type = false) const SKR_NOEXCEPT
    {
        return static_cast<T*>(find_ancestor_render_object(rttr::type_id_of<T>(), exact_type));
    }

    // TODO. inherited element
    // virtual InheritedWidget*  depend_on_inherited_element(NotNull<InheritedElement*> ancestor) SKR_NOEXCEPT      = 0;
    // virtual InheritedWidget*  depend_on_inherited_widget_of_exact_type(const GUID& type_id) SKR_NOEXCEPT         = 0;
    // virtual InheritedElement* get_element_for_inherited_widget_of_exact_type(const GUID& type_id) SKR_NOEXCEPT   = 0;

    // virtual Widget*       find_ancestor_widget(const GUID& type_id) SKR_NOEXCEPT        = 0;
    // virtual State*        find_ancestor_state(const GUID& type_id) SKR_NOEXCEPT         = 0;
    // virtual State*        find_root_ancestor_state(const GUID& type_id) SKR_NOEXCEPT    = 0;
    // virtual RenderObject* find_ancestor_render_object(const GUID& type_id) SKR_NOEXCEPT = 0;

    // TODO. notification
    // virtual void dispatch_notification(NotNull<Notification*> notification) SKR_NOEXCEPT = 0;
};
} // namespace skr::gui