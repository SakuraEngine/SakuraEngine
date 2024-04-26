#include "SkrGui/render_objects/render_stack.hpp"
#include "SkrGui/framework/painting_context.hpp"

namespace skr::gui
{
struct _StackHelper {
    inline static BoxConstraints _fit_constraints(const RenderStack& self, const BoxConstraints& constraints) SKR_NOEXCEPT
    {
        switch (self._child_fit)
        {
            case EPositionalFit::Loose:
                return constraints.loosen();
            case EPositionalFit::Expand:
                return BoxConstraints::Tight(constraints.biggest());
            case EPositionalFit::PassThrough:
                return constraints;
            default:
                SKR_UNREACHABLE_CODE();
                return {};
        }
    }

    template <typename TLayoutFunc>
    inline static Sizef _compute_size(const RenderStack&    self,
                                      const BoxConstraints& constraints,
                                      TLayoutFunc&&         layout_func) SKR_NOEXCEPT
    {
        if (self.children().size() == 0) return constraints.biggest().is_finite() ? constraints.biggest() : constraints.smallest();
        if (self._stack_size == EStackSize::Expand) return constraints.biggest();

        BoxConstraints fit_constraints = _fit_constraints(self, constraints);
        float          width           = constraints.min_width;
        float          height          = constraints.min_height;
        for (const auto& slot : self.children())
        {
            Sizef child_size = layout_func(slot.child, fit_constraints);
            width            = std::max(width, child_size.width);
            height           = std::max(height, child_size.height);
        }
        return { width, height };
    }

    template <typename TIntrinsicFunc>
    inline static float _get_intrinsic(
    const RenderStack& self,
    TIntrinsicFunc&&   intrinsic_func) SKR_NOEXCEPT
    {
        float result = 0.0f;
        for (const auto& slot : self.children())
        {
            result = std::max(result, intrinsic_func(slot.child));
        }
        return result;
    }
};
} // namespace skr::gui

namespace skr::gui
{
// intrinsic size
float RenderStack::compute_min_intrinsic_width(float height) const SKR_NOEXCEPT
{
    return _StackHelper::_get_intrinsic(
    *this,
    [height](const RenderBox* child) { return child->get_min_intrinsic_width(height); });
}
float RenderStack::compute_max_intrinsic_width(float height) const SKR_NOEXCEPT
{
    return _StackHelper::_get_intrinsic(
    *this,
    [height](const RenderBox* child) { return child->get_max_intrinsic_width(height); });
}
float RenderStack::compute_min_intrinsic_height(float width) const SKR_NOEXCEPT
{
    return _StackHelper::_get_intrinsic(
    *this,
    [width](const RenderBox* child) { return child->get_min_intrinsic_height(width); });
}
float RenderStack::compute_max_intrinsic_height(float width) const SKR_NOEXCEPT
{
    return _StackHelper::_get_intrinsic(
    *this,
    [width](const RenderBox* child) { return child->get_max_intrinsic_height(width); });
}

// dry layout
Sizef RenderStack::compute_dry_layout(BoxConstraints constraints) const SKR_NOEXCEPT
{
    return _StackHelper::_compute_size(
    *this,
    constraints,
    [](const RenderBox* child, const BoxConstraints& constraints) {
        return child->get_dry_layout(constraints);
    });
}

// layout
void RenderStack::perform_layout() SKR_NOEXCEPT
{
    set_size(_StackHelper::_compute_size(
    *this,
    constraints(),
    [](RenderBox* child, const BoxConstraints& constraints) {
        child->set_constraints(constraints);
        child->layout(true);
        return child->size();
    }));

    for (auto& slot : children())
    {
        slot.data.offset = _stack_alignment.along_size(size() - slot.child->size());
    }
}

// paint
void RenderStack::paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT
{
    // TODO. clip behaviour
    for (const auto& slot : children())
    {
        if (slot.child)
        {
            context->paint_child(slot.child, slot.data.offset + offset);
        }
        else
        {
            SKR_GUI_LOG_ERROR(u8"RenderFlex::paint: child is nullptr.");
        }
    }
}

// setter
void RenderStack::set_stack_alignment(Alignment alignment) SKR_NOEXCEPT
{
    if (_stack_alignment != alignment)
    {
        _stack_alignment = alignment;
        mark_needs_layout();
    }
}
void RenderStack::set_child_fit(EPositionalFit fit) SKR_NOEXCEPT
{
    if (_child_fit != fit)
    {
        _child_fit = fit;
        mark_needs_layout();
    }
}
void RenderStack::set_stack_size(EStackSize size) SKR_NOEXCEPT
{
    if (_stack_size != size)
    {
        _stack_size = size;
        mark_needs_layout();
    }
}

// hit test
bool RenderStack::hit_test(HitTestResult* result, Offsetf local_position) const SKR_NOEXCEPT
{
    if (size().contains(local_position))
    {
        for (auto i = children().size(); i > 0; --i)
        {
            const auto& child_data = children()[i - 1];

            bool is_hit = result->add_with_paint_offset(
            child_data.data.offset,
            local_position,
            [&child_data](HitTestResult* result, Offsetf transformed_position) {
                return child_data.child->hit_test(result, transformed_position);
            });

            if (is_hit)
            {
                return true;
            }
        }
    }
    return false;
}

// transform
void RenderStack::apply_paint_transform(NotNull<const RenderObject*> child, Matrix4& transform) const SKR_NOEXCEPT
{
    auto slot = child->slot();
    if (!slot.is_valid() || !children().is_valid_index(slot.index))
    {
        SKR_GUI_LOG_ERROR(u8"child slot is invalid.");
        return;
    }
    else if (children()[slot.index].child != child)
    {
        SKR_GUI_LOG_ERROR(u8"child is not a child of this object.");
        return;
    }

    transform = Matrix4::Translate(children()[slot.index].data.offset) * transform;
}

} // namespace skr::gui