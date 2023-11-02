#pragma once
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#ifndef __meta__
    #include "SkrGui/framework/build_context.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "0ff4a42c-7195-48c5-a979-263dab05ac2d",
    "rtti": true
)
SKR_GUI_API IBuildContext : virtual public skr::rttr::IObject
{
    SKR_RTTR_GENERATE_BODY()
    virtual ~IBuildContext() = default;

    virtual Widget*           bound_widget() const SKR_NOEXCEPT                                                  = 0;
    virtual BuildOwner*       build_owner() const SKR_NOEXCEPT                                                   = 0;
    virtual bool              is_destroyed() const SKR_NOEXCEPT                                                  = 0;
    virtual RenderObject*     find_render_object() const SKR_NOEXCEPT                                            = 0;
    virtual RenderObject*     find_ancestor_render_object() const SKR_NOEXCEPT                                   = 0;
    virtual Optional<Sizef>   render_box_size() const SKR_NOEXCEPT                                               = 0;
    virtual InheritedWidget*  depend_on_inherited_element(NotNull<InheritedElement*> ancestor) SKR_NOEXCEPT      = 0;
    virtual InheritedWidget*  depend_on_inherited_widget_of_exact_type(const GUID& type_id) SKR_NOEXCEPT         = 0;
    virtual InheritedElement* get_element_for_inherited_widget_of_exact_type(const GUID& type_id) SKR_NOEXCEPT   = 0;
    virtual Widget*           find_ancestor_widget_of_exact_type(const GUID& type_id) SKR_NOEXCEPT               = 0;
    virtual State*            find_ancestor_state_of_exact_type(const GUID& type_id) SKR_NOEXCEPT                = 0;
    virtual State*            find_root_ancestor_state_of_exact_type(const GUID& type_id) SKR_NOEXCEPT           = 0;
    virtual RenderObject*     find_ancestor_render_object_of_exact_type(const GUID& type_id) SKR_NOEXCEPT        = 0;
    virtual void              visit_ancestor_elements(FunctionRef<bool(NotNull<Element*>)> visitor) SKR_NOEXCEPT = 0;
    virtual void              visit_child_elements(FunctionRef<void(NotNull<Element*>)> visitor) SKR_NOEXCEPT    = 0;
    virtual void              dispatch_notification(NotNull<Notification*> notification) SKR_NOEXCEPT            = 0;

    // TODO. describeElement
    // TODO. describeWidget
    // TODO. describeMissingAncestor
    // TODO. describeOwnershipChain
};
} // namespace gui sreflect
} // namespace skr sreflect