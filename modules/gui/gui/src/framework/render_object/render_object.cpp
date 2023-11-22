#include "SkrGui/framework/render_object/render_object.hpp"
#include "SkrGui/framework/build_owner.hpp"
#include "SkrGui/framework/layer/offset_layer.hpp"
#include "SkrGui/framework/render_object/render_native_window.hpp"
#include "SkrGui/backend/device/window.hpp"

namespace skr::gui
{

RenderObject::RenderObject() SKR_NOEXCEPT
{
}
RenderObject::~RenderObject()
{
}

// new lifecycle
void RenderObject::mount(NotNull<RenderObject*> parent) SKR_NOEXCEPT
{
    // validate
    if (_parent != nullptr)
    {
        unmount();
        SKR_GUI_LOG_ERROR(u8"already mounted");
    }
    {
        RenderObject* node = parent;
        while (node->_parent)
        {
            node = node->_parent;
            if (node == this)
            {
                SKR_GUI_LOG_ERROR(u8"cycle in the tree");
                break;
            }
        }
    }

    // mount
    _parent = parent;
    if (parent->owner())
    {
        struct _RecursiveHelper {
            NotNull<BuildOwner*> owner;

            inline bool operator()(NotNull<RenderObject*> obj) const SKR_NOEXCEPT
            {
                obj->attach(owner);
                obj->visit_children(_RecursiveHelper{ owner });
                return true;
            }
        };
        _RecursiveHelper{ _parent->owner() }(this);
    }
    _lifecycle = ERenderObjectLifecycle::Mounted;
}
void RenderObject::unmount() SKR_NOEXCEPT
{
    // validate
    if (_parent == nullptr) { SKR_GUI_LOG_ERROR(u8"already unmounted"); }

    // unmount
    _parent = nullptr;
    if (owner())
    {
        struct _RecursiveHelper {
            bool operator()(NotNull<RenderObject*> obj) const SKR_NOEXCEPT
            {
                obj->detach();
                obj->visit_children(_RecursiveHelper{});
                return true;
            }
        };
        _RecursiveHelper{}(this);
    }
    _lifecycle = ERenderObjectLifecycle::Unmounted;
}
void RenderObject::destroy() SKR_NOEXCEPT
{
    // TODO. release layer
    _lifecycle = ERenderObjectLifecycle::Destroyed;
}
void RenderObject::attach(NotNull<BuildOwner*> owner) SKR_NOEXCEPT
{
    // validate
    if (_owner != nullptr) { SKR_GUI_LOG_ERROR(u8"already attached"); }
    if (_parent == nullptr) { SKR_GUI_LOG_ERROR(u8"parent is nullptr"); }

    // attach
    _owner = owner;
    _depth = _parent ? _parent->_depth + 1 : 0;

    // If the node was dirtied in some way while unattached, make sure to add
    // it to the appropriate dirty list now that an owner is available
    if (_needs_layout && _relayout_boundary != nullptr)
    {
        // Don't enter this block if we've never laid out at all;
        // scheduleInitialLayout() will handle it
        cancel_needs_layout();
        mark_needs_layout();
    }
    if (_needs_paint && _layer != nullptr)
    {
        // Don't enter this block if we've never painted at all;
        // scheduleInitialPaint() will handle it
        cancel_needs_paint();
        mark_needs_paint();
    }

    // recursive
}
void RenderObject::detach() SKR_NOEXCEPT
{
    if (_owner == nullptr) { SKR_GUI_LOG_ERROR(u8"already detached"); }
    _owner = nullptr;
    if (_parent != nullptr && _owner != _parent->_owner) { SKR_GUI_LOG_ERROR(u8"detach from owner but parent is still attached"); }
}

// layout & paint marks
void RenderObject::mark_needs_layout() SKR_NOEXCEPT
{
    if (_relayout_boundary == nullptr)
    {
        _needs_layout = true;
        if (_parent != nullptr)
        {
            _mark_parent_needs_layout();
        }
    }
    else if (_relayout_boundary != this)
    {
        _mark_parent_needs_layout();
    }
    else
    {
        _needs_layout = true;
        if (_owner != nullptr)
        {
            _owner->schedule_layout_for(this);
        }
    }
}
void RenderObject::mark_needs_paint() SKR_NOEXCEPT
{
    if (!_needs_paint)
    {
        _needs_paint = true;

        if (is_repaint_boundary() && _was_repaint_boundary)
        {
            if (owner())
            {
                owner()->schedule_paint_for(this);
            }
        }
        else if (parent() && parent()->type_is<RenderObject>())
        {
            parent()->type_cast_fast<RenderObject>()->mark_needs_paint();
        }
        else
        {
            SKR_UNREACHABLE_CODE()
        }
    }
}
void mark_needs_layer_update() SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION()
}

// layout process
bool RenderObject::is_sized_by_parent() const SKR_NOEXCEPT { return false; }
void RenderObject::layout(bool parent_uses_size) SKR_NOEXCEPT
{
    bool          is_relayout_boundary = !parent_uses_size || is_sized_by_parent() || _force_relayout_boundary || !_parent;
    RenderObject* relayout_boundary    = is_relayout_boundary ? this : _parent->_relayout_boundary;

    if (!_needs_layout && !_is_constraints_changed)
    {
        if (relayout_boundary != _relayout_boundary)
        {
            _relayout_boundary = relayout_boundary;
            visit_children([](RenderObject* child) {
                child->_flush_relayout_boundary();
                return true;
            });
        }
    }
    else
    {
        _relayout_boundary = relayout_boundary;
        if (is_sized_by_parent())
        {
            perform_resize();
        }
        perform_layout();

        cancel_needs_layout();
        mark_needs_paint();
    }

    // clean up flags
    _is_constraints_changed = false;
}
void RenderObject::perform_resize() SKR_NOEXCEPT {}
void RenderObject::perform_layout() SKR_NOEXCEPT {}

// paint process
void RenderObject::paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT {}
bool RenderObject::is_repaint_boundary() const SKR_NOEXCEPT { return false; }

// layer composite
NotNull<OffsetLayer*> RenderObject::update_layer(OffsetLayer* old_layer)
{
    return old_layer != nullptr ? old_layer : SkrNew<OffsetLayer>();
}

// transform
bool    RenderObject::paints_child(NotNull<const RenderObject*> child) const SKR_NOEXCEPT { return true; }
void    RenderObject::apply_paint_transform(NotNull<const RenderObject*> child, Matrix4& transform) const SKR_NOEXCEPT {}
Matrix4 RenderObject::get_transform_to(const RenderObject* ancestor) const SKR_NOEXCEPT
{
    // setup ancestor
    if (ancestor == nullptr)
    {
        ancestor = this;
        while (ancestor->parent() != nullptr)
        {
            ancestor = ancestor->parent();
        }
    }

    // append transform
    Matrix4 transform   = Matrix4::Identity();
    auto    cur_node    = this;
    auto    parent_node = cur_node->parent();
    while (cur_node != ancestor)
    {
        if (!parent_node)
        {
            SKR_LOG_ERROR(u8"ancestor is not in the parent chain");
            return {};
        }

        parent_node->apply_paint_transform(cur_node, transform);
        cur_node    = parent_node;
        parent_node = cur_node->parent();
    }
    return transform;
}
Offsetf RenderObject::global_to_local(Offsetf global_position, const RenderObject* ancestor) const SKR_NOEXCEPT
{
    Matrix4 transform = get_transform_to(ancestor);
    Matrix4 inv_transform;
    if (transform.try_inverse(inv_transform))
    {
        return inv_transform.transform(global_position);
    }
    return {};
}
Offsetf RenderObject::local_to_global(Offsetf local_position, const RenderObject* ancestor) const SKR_NOEXCEPT
{
    Matrix4 transform = get_transform_to(ancestor);
    return transform.transform(local_position);
}
Offsetf RenderObject::system_to_local(Offsetf system_position) const SKR_NOEXCEPT
{
    auto root_widget = this;
    while (root_widget->parent())
    {
        root_widget = root_widget->parent();
    }
    auto root_window = root_widget->type_cast_fast<RenderNativeWindow>();
    auto global_pos  = root_window->window()->type_cast_fast<INativeWindow>()->to_relative(system_position);
    return global_to_local(global_pos);
}
Offsetf RenderObject::local_to_system(Offsetf local_position) const SKR_NOEXCEPT
{
    auto root_widget = this;
    while (root_widget->parent())
    {
        root_widget = root_widget->parent();
    }
    auto root_window = root_widget->type_cast_fast<RenderNativeWindow>();
    auto global_pos  = local_to_global(local_position);
    return root_window->window()->type_cast_fast<INativeWindow>()->to_absolute(global_pos);
}

// event
bool RenderObject::handle_event(NotNull<PointerEvent*> event, NotNull<HitTestEntry*> entry)
{
    return false;
}

// layout & paint marks
void RenderObject::_mark_parent_needs_layout() SKR_NOEXCEPT
{
    _needs_layout = true;
    _parent->mark_needs_layout();
    // TODO. 针对 sliver 在 layout 中 create widget 的行为，需要在这里阻断向 parent 的传递
}
void RenderObject::_flush_relayout_boundary() SKR_NOEXCEPT
{
    if (_relayout_boundary != this)
    {
        auto parent_relayout_boundary = _parent->_relayout_boundary;
        if (parent_relayout_boundary != _relayout_boundary)
        {
            visit_children([](RenderObject* child) {
                child->_flush_relayout_boundary();
                return true;
            });
        }
    }
}

} // namespace skr::gui