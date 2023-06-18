#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

namespace skr::gui
{
struct SKR_GUI_API ISingleChildRenderObject SKR_GUI_INTERFACE_BASE {
    SKR_GUI_INTERFACE_ROOT(ISingleChildRenderObject, "5349672b-bfc5-46a9-9a02-40ef563c196d")
    virtual ~ISingleChildRenderObject() = default;

    virtual SKR_GUI_TYPE_ID accept_child_type() const SKR_NOEXCEPT = 0;
    virtual void            set_child(RenderObject* child) SKR_NOEXCEPT = 0;
};

// 施舍一点样板代码，快拷走使用吧
// ===> .hpp
//     //==> MIXIN: single child render object
// public:
//     SKR_GUI_TYPE_ID   accept_child_type() const noexcept override;
//     void              set_child(RenderObject* child) noexcept override;
//     void              flush_depth() noexcept override;
//     void              visit_children(function_ref<void(RenderObject*)> visitor) const noexcept override;
//     void              visit_children_recursive(function_ref<void(RenderObject*)> visitor) const noexcept override;
//     void              attach(NotNull<PipelineOwner*> owner) noexcept override;
//     void              detach() noexcept override;
//     inline __CHILD_TYPE__* child() const noexcept { return _child; }
//
// private:
//     __CHILD_TYPE__* _child;
//     //==> MIXIN: single child render object

// ===> .cpp
// //==> MIXIN: single child render object
// SKR_GUI_TYPE_ID __MIX_IN_TARGET__::accept_child_type() const noexcept { return SKR_GUI_TYPE_ID_OF_STATIC(__CHILD_TYPE__); }
// void            __MIX_IN_TARGET__::set_child(RenderObject* child) noexcept
// {
//     if (_child) drop_child(make_not_null(_child));
//     _child = child->type_cast_fast<__CHILD_TYPE__>();
//     if (_child) adopt_child(make_not_null(_child));
// }
// void __MIX_IN_TARGET__::flush_depth() noexcept
// {
//     __SUPER_TYPE__::flush_depth();
//     if (_child) _child->flush_depth();
// }
// void __MIX_IN_TARGET__::visit_children(function_ref<void(RenderObject*)> visitor) const noexcept
// {
//     if (_child) visitor(_child);
// }
// void __MIX_IN_TARGET__::visit_children_recursive(function_ref<void(RenderObject*)> visitor) const noexcept
// {
//     if (_child) _child->visit_children_recursive(visitor);
// }
// void __MIX_IN_TARGET__::attach(NotNull<PipelineOwner*> owner) noexcept
// {
//     __SUPER_TYPE__ ::attach(owner);
//     if (_child) _child->attach(owner);
// }
// void __MIX_IN_TARGET__::detach() noexcept
// {
//     if (_child) _child->detach();
//     __SUPER_TYPE__::detach();
// }
// //==> MIXIN: single child render object

} // namespace skr::gui