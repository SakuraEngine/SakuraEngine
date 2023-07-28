#include "SkrGui/framework/render_object/render_object.hpp"
#include "SkrGui/framework/pipeline_owner.hpp"
#include "SkrGui/framework/layer/offet_layer.hpp"

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
            NotNull<PipelineOwner*> owner;

            inline void operator()(NotNull<RenderObject*> obj) const SKR_NOEXCEPT
            {
                obj->attach(owner);
                obj->visit_children(_RecursiveHelper{ owner });
            }
        };
        _RecursiveHelper{ make_not_null(_parent->owner()) }(make_not_null(this));
        this->visit_children(_RecursiveHelper{ make_not_null(_parent->owner()) });
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
            void operator()(NotNull<RenderObject*> obj) const SKR_NOEXCEPT
            {
                obj->detach();
                obj->visit_children(_RecursiveHelper{});
            }
        };
        this->visit_children(_RecursiveHelper{});
    }
    _lifecycle = ERenderObjectLifecycle::Unmounted;
}
void RenderObject::destroy() SKR_NOEXCEPT
{
    // TODO. release layer
    _lifecycle = ERenderObjectLifecycle::Destroyed;
}
void RenderObject::attach(NotNull<PipelineOwner*> owner) SKR_NOEXCEPT
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
            _owner->schedule_layout_for(make_not_null(this));
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
                owner()->schedule_paint_for(make_not_null(this));
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
            visit_children([](RenderObject* child) { child->_flush_relayout_boundary(); });
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
    return old_layer != nullptr ? make_not_null(old_layer) : make_not_null(SkrNew<OffsetLayer>());
}

// transform
bool    RenderObject::paints_child(NotNull<RenderObject*> child) const SKR_NOEXCEPT { return true; }
void    RenderObject::apply_paint_transform(NotNull<RenderObject*> child, Matrix4& transform) const SKR_NOEXCEPT {}
Matrix4 RenderObject::get_transform_to(RenderObject* ancestor) const SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION()
    return {};
}

// layout & paint marks
void RenderObject::_mark_parent_needs_layout() SKR_NOEXCEPT
{
    _needs_layout = true;
    _parent->mark_needs_layout();
    // TODO. layout call from paint
}
void RenderObject::_flush_relayout_boundary() SKR_NOEXCEPT
{
    if (_relayout_boundary != this)
    {
        auto parent_relayout_boundary = _parent->_relayout_boundary;
        if (parent_relayout_boundary != _relayout_boundary)
        {
            visit_children([](RenderObject* child) { child->_flush_relayout_boundary(); });
        }
    }
}

} // namespace skr::gui