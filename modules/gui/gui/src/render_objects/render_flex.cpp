#include "SkrGui/render_objects/render_flex.hpp"
#include "SkrGui/framework/painting_context.hpp"

// help functions
namespace skr::gui
{
struct _FlexHelper {
    // main-cross select & combine
    template <typename TH, typename TV>
    inline static auto _select_main(const RenderFlex& self, TH&& horizontal, TV&& vertical) -> decltype(std::declval<TH>()())
    {
        switch (self._flex_direction)
        {
            case EFlexDirection::Row:
            case EFlexDirection::RowReverse:
                return horizontal();
            case EFlexDirection::Column:
            case EFlexDirection::ColumnReverse:
                return vertical();
            default:
                SKR_UNREACHABLE_CODE()
                return {};
        }
    }
    template <typename TH, typename TV>
    inline static auto _select_cross(const RenderFlex& self, TH&& horizontal, TV&& vertical) -> decltype(std::declval<TH>()())
    {
        switch (self._flex_direction)
        {
            case EFlexDirection::Row:
            case EFlexDirection::RowReverse:
                return vertical();
            case EFlexDirection::Column:
            case EFlexDirection::ColumnReverse:
                return horizontal();
            default:
                SKR_UNREACHABLE_CODE()
                return {};
        }
    }
    inline static float _get_main_size(const RenderFlex& self, Sizef size) SKR_NOEXCEPT
    {
        return _select_main(
        self,
        [&]() { return size.width; },
        [&]() { return size.height; });
    }
    inline static float _get_cross_size(const RenderFlex& self, Sizef size) SKR_NOEXCEPT
    {
        return _select_cross(
        self, [&]() { return size.width; }, [&]() { return size.height; });
    }
    inline static Sizef _combine_size(const RenderFlex& self, float main_size, float cross_size) SKR_NOEXCEPT
    {
        return _select_main(
        self,
        [&]() { return Sizef{ main_size, cross_size }; },
        [&]() { return Sizef{ cross_size, main_size }; });
    }
    inline static Offsetf _combine_offset(const RenderFlex& self, float main_offset, float cross_offset) SKR_NOEXCEPT
    {
        return _select_main(
        self,
        [&]() { return Offsetf{ main_offset, cross_offset }; },
        [&]() { return Offsetf{ cross_offset, main_offset }; });
    }

    // flip
    inline static bool _is_coord_flipped(EFlexDirection dir)
    {
        switch (dir)
        {
            case EFlexDirection::Row:
            case EFlexDirection::RowReverse:
                return false;
            case EFlexDirection::Column:
            case EFlexDirection::ColumnReverse:
                return true;
            default:
                SKR_UNREACHABLE_CODE()
                return false;
        }
    }
    inline static bool _is_coord_flipped(const RenderFlex& self) SKR_NOEXCEPT
    {
        return _is_coord_flipped(self._flex_direction);
    }
    inline static bool _is_main_axis_flipped(const RenderFlex& self) SKR_NOEXCEPT
    {
        switch (self._flex_direction)
        {
            case EFlexDirection::Row:
            case EFlexDirection::Column:
                return false;
            case EFlexDirection::RowReverse:
            case EFlexDirection::ColumnReverse:
                return true;
            default:
                SKR_UNREACHABLE_CODE()
                return false;
        }
    }
    inline static bool _is_cross_axis_flipped(const RenderFlex& self) SKR_NOEXCEPT
    {
        return false;
    }

    // check
    inline static bool _can_compute_intrinsics(const RenderFlex& self) SKR_NOEXCEPT
    {
        return self._cross_axis_alignment != ECrossAxisAlignment::Baseline;
    }

    // compute
    template <typename TLayoutFunc>
    inline static void _compute_sizes(const RenderFlex& self,
                                      BoxConstraints    constraints,
                                      TLayoutFunc&&     layout_func,
                                      float&            out_main_size,
                                      float&            out_cross_size,
                                      float&            out_allocated_size)
    {
        // collect data
        const float max_main_size    = _get_main_size(self, constraints.max_size());
        const bool  can_flex         = max_main_size < std::numeric_limits<float>::infinity();
        const bool  is_coord_flipped = _is_coord_flipped(self);

        out_cross_size     = 0;
        out_allocated_size = 0;
        float total_flex   = 0;

        // Step1. Calculate non-flexible children's size
        for (const auto& slot : self.children())
        {
            if (slot.data.flex > 0)
            {
                total_flex += slot.data.flex;
            }
            else
            {
                // solve child constraints
                BoxConstraints inner_constraints = self._cross_axis_alignment == ECrossAxisAlignment::Stretch ?
                                                   is_coord_flipped ?
                                                   BoxConstraints::TightWidth(constraints.max_width) :
                                                   BoxConstraints::TightHeight(constraints.max_height) :
                                                   is_coord_flipped ?
                                                   BoxConstraints::LooseWidth(constraints.max_width) :
                                                   BoxConstraints::LooseHeight(constraints.max_height);

                // layout child
                const Sizef child_size = layout_func(slot.child, inner_constraints);
                out_allocated_size += _get_main_size(self, child_size);
                out_cross_size = std::max(out_cross_size, _get_cross_size(self, child_size));
            }
        }

        // Step2. Calculate size of each child based on flex factor and available space
        // Step3. Layout each reaming child
        const float free_space = std::max(0.f, can_flex ? max_main_size : 0.0f) - out_allocated_size;
        if (total_flex > 0)
        {
            const float space_per_flex = can_flex ? (free_space / total_flex) : std::numeric_limits<float>::signaling_NaN();
            for (const auto& slot : self.children())
            {
                if (slot.data.flex > 0)
                {
                    // solve child extent
                    const float max_child_extent = can_flex ? space_per_flex * slot.data.flex : std::numeric_limits<float>::infinity();
                    float       min_child_extent{};
                    switch (slot.data.flex_fit)
                    {
                        case EFlexFit::Tight:
                            min_child_extent = max_child_extent;
                            break;
                        case EFlexFit::Loose:
                            min_child_extent = 0.0f;
                            break;
                    }

                    // solve child constraints
                    BoxConstraints inner_constraints = self._cross_axis_alignment == ECrossAxisAlignment::Stretch ?
                                                       is_coord_flipped ?
                                                       BoxConstraints{
                                                           constraints.max_width,
                                                           constraints.max_width,
                                                           min_child_extent,
                                                           max_child_extent,
                                                       } :
                                                       BoxConstraints{
                                                           min_child_extent,
                                                           max_child_extent,
                                                           constraints.max_height,
                                                           constraints.max_height,
                                                       } :
                                                       is_coord_flipped ?
                                                       BoxConstraints{
                                                           0,
                                                           constraints.max_width,
                                                           min_child_extent,
                                                           max_child_extent,
                                                       } :
                                                       BoxConstraints{
                                                           min_child_extent,
                                                           max_child_extent,
                                                           0,
                                                           constraints.max_height,
                                                       };

                    // layout child
                    const Sizef child_size = layout_func(slot.child, inner_constraints);
                    out_allocated_size += _get_main_size(self, child_size);
                    out_cross_size = std::max(out_cross_size, _get_cross_size(self, child_size));
                }
            }
        }

        out_main_size = can_flex && self._main_axis_size == EMainAxisSize::Max ? max_main_size : out_allocated_size;
    }
    template <typename TChildSizeFunc>
    inline static float _get_intrinsics_size(const RenderFlex& self,
                                             EFlexDirection    sizing_direction,
                                             float             extent,
                                             TChildSizeFunc&&  child_size_func)
    {
        if (!_can_compute_intrinsics(self))
        {
            SKR_GUI_LOG_ERROR(u8"Intrinsics are not available for ECrossAxisAlignment.baseline, which requires a full layout.");
            return 0.0f;
        }

        if (_is_coord_flipped(sizing_direction) == _is_coord_flipped(self))
        {
            // INTRINSIC MAIN SIZE
            // Intrinsic main size is the smallest size the flex container can take
            // while maintaining the min/max-content contributions of its flex items.
            float total_flex = 0.f, inflexible_space = 0.f, max_flex_fraction_so_far = 0.f;
            for (const auto& slot : self.children())
            {
                total_flex += slot.data.flex;
                if (slot.data.flex > 0)
                {
                    const float flex_fraction = child_size_func(slot.child, extent) / slot.data.flex;
                    max_flex_fraction_so_far  = std::max(max_flex_fraction_so_far, flex_fraction);
                }
                else
                {
                    inflexible_space += child_size_func(slot.child, extent);
                }
            }
            return max_flex_fraction_so_far * total_flex + inflexible_space;
        }
        else
        {
            // INTRINSIC CROSS SIZE
            // Intrinsic cross size is the max of the intrinsic cross sizes of the
            // children, after the flexible children are fit into the available space,
            // with the children sized using their max intrinsic dimensions.

            // Get inflexible space using the max intrinsic dimensions of fixed children in the main direction.
            const float available_main_size = extent;
            float       total_flex = 0.f, inflexible_space = 0.f, max_cross_size = 0.f;
            for (const auto& slot : self.children())
            {
                total_flex += slot.data.flex;
                float main_size{}, cross_size{};
                if (slot.data.flex == 0)
                {
                    switch (self._flex_direction)
                    {
                        case EFlexDirection::Row:
                        case EFlexDirection::RowReverse:
                            main_size  = slot.child->get_max_intrinsic_width(std::numeric_limits<float>::infinity());
                            cross_size = child_size_func(slot.child, main_size);
                            break;
                        case EFlexDirection::Column:
                        case EFlexDirection::ColumnReverse:
                            main_size  = slot.child->get_max_intrinsic_height(std::numeric_limits<float>::infinity());
                            cross_size = child_size_func(slot.child, main_size);
                            break;
                    }
                    inflexible_space += main_size;
                    max_cross_size = std::max(max_cross_size, cross_size);
                }
            }

            // Determine the spacePerFlex by allocating the remaining available space.
            // When you're overconstrained spacePerFlex can be negative.
            const float space_per_flex = std::max(0.f, (available_main_size - inflexible_space) / total_flex);

            // Sizef remaining (flexible) items, find the maximum cross size.
            for (const auto& slot : self.children())
            {
                if (slot.data.flex > 0)
                {
                    max_cross_size = std::max(max_cross_size, child_size_func(slot.child, space_per_flex * slot.data.flex));
                }
            }
            return max_cross_size;
        }
    }
};
} // namespace skr::gui

namespace skr::gui
{
// intrinsic size
float RenderFlex::compute_min_intrinsic_width(float height) const SKR_NOEXCEPT
{
    return _FlexHelper::_get_intrinsics_size(
    *this,
    EFlexDirection::Row, // reverse is not cared here
    height,
    [](const RenderBox* child, float extent) {
        return child->get_min_intrinsic_width(extent);
    });
}
float RenderFlex::compute_max_intrinsic_width(float height) const SKR_NOEXCEPT
{
    return _FlexHelper::_get_intrinsics_size(
    *this,
    EFlexDirection::Row, // reverse is not cared here
    height,
    [](const RenderBox* child, float extent) {
        return child->get_max_intrinsic_width(extent);
    });
}
float RenderFlex::compute_min_intrinsic_height(float width) const SKR_NOEXCEPT
{
    return _FlexHelper::_get_intrinsics_size(
    *this,
    EFlexDirection::Column, // reverse is not cared here
    width,
    [](const RenderBox* child, float extent) {
        return child->get_min_intrinsic_height(extent);
    });
}
float RenderFlex::compute_max_intrinsic_height(float width) const SKR_NOEXCEPT
{
    return _FlexHelper::_get_intrinsics_size(
    *this,
    EFlexDirection::Column, // reverse is not cared here
    width,
    [](const RenderBox* child, float extent) {
        return child->get_max_intrinsic_height(extent);
    });
}

// dry layout
Sizef RenderFlex::compute_dry_layout(BoxConstraints constraints) const SKR_NOEXCEPT
{
    if (!_FlexHelper::_can_compute_intrinsics(*this))
    {
        SKR_GUI_LOG_ERROR(u8"Dry layout cannot be computed for ECrossAxisAlignment.baseline, which requires a full layout.");
        return Sizef::Zero();
    }

    // compute sizes
    float main_size, cross_size, allocated_size;
    _FlexHelper::_compute_sizes(
    *this,
    constraints,
    +[](RenderBox* child, const BoxConstraints& constraints) {
        return child->get_dry_layout(constraints);
    },
    main_size,
    cross_size,
    allocated_size);

    return _FlexHelper::_combine_size(*this, main_size, cross_size);
}

// layout
void RenderFlex::perform_layout() SKR_NOEXCEPT
{
    const bool is_coord_flipped = _FlexHelper::_is_coord_flipped(*this);

    // Step1. Calculate non-flexible children's size
    // Step2. Calculate size of each child based on flex factor and available space
    // Step3. Layout each reaming child
    // Step4. Calculate self cross axis size
    // Step5. Calculate self main axis size
    float main_size, cross_size, allocated_size;
    _FlexHelper::_compute_sizes(
    *this,
    constraints(),
    +[](RenderBox* child, const BoxConstraints& constraints) {
        child->set_constraints(constraints);
        child->layout(true);
        return child->size();
    },
    main_size,
    cross_size,
    allocated_size);

    // update self size
    set_size(constraints().constrain(is_coord_flipped ? Sizef{ cross_size, main_size } : Sizef{ main_size, cross_size }));
    main_size  = _FlexHelper::_get_main_size(*this, size());
    cross_size = _FlexHelper::_get_cross_size(*this, size());

    // Step6. Place each child based on main axis alignment and cross axis alignment
    float leading_space{}, between_space{};
    {
        // calc slack & overflow
        float slack_space = main_size - allocated_size;
        _overflow         = std::max(0.f, -slack_space);
        slack_space       = std::max(0.f, slack_space);

        // calc leading space & between space
        const auto child_count = children().size();
        switch (_main_axis_alignment)
        {
            case EMainAxisAlignment::Start:
                leading_space = 0.0f;
                between_space = 0.0f;
                break;
            case EMainAxisAlignment::End:
                leading_space = slack_space;
                between_space = 0.0f;
                break;
            case EMainAxisAlignment::Center:
                leading_space = slack_space / 2.0f;
                between_space = 0.0f;
                break;
            case EMainAxisAlignment::SpaceBetween:
                leading_space = 0.0f;
                between_space = child_count > 1 ? (slack_space / (child_count - 1)) : 0.0f;
                break;
            case EMainAxisAlignment::SpaceAround:
                between_space = child_count > 0 ? (slack_space / child_count) : 0.0f;
                leading_space = between_space / 2.0f;
                break;
            case EMainAxisAlignment::SpaceEvenly:
                between_space = child_count > 0 ? (slack_space / (child_count + 1)) : 0.0f;
                leading_space = between_space;
                break;
        }
    }

    const bool flip_main_axis    = _FlexHelper::_is_main_axis_flipped(*this);
    const bool flip_cross_axis   = _FlexHelper::_is_cross_axis_flipped(*this);
    float      child_main_offset = flip_main_axis ? (main_size - leading_space) : leading_space;
    for (size_t i = 0; i < children().size(); ++i)
    {
        auto&       slot             = children()[i];
        const Sizef child_size       = slot.child->size();
        const float child_main_size  = _FlexHelper::_get_main_size(*this, child_size);
        const float child_cross_size = _FlexHelper::_get_cross_size(*this, child_size);

        // cross axis offset
        float child_cross_offset{};
        switch (_cross_axis_alignment)
        {
            case ECrossAxisAlignment::Start:
                child_cross_offset = flip_cross_axis ? cross_size - child_cross_size : 0.f;
                break;
            case ECrossAxisAlignment::End:
                child_cross_offset = flip_cross_axis ? 0.0f : cross_size - child_cross_size;
                break;
            case ECrossAxisAlignment::Center:
                child_cross_offset = (cross_size - child_cross_size) / 2.f;
                break;
            case ECrossAxisAlignment::Stretch:
                child_cross_offset = 0.f;
                break;
            case ECrossAxisAlignment::Baseline:
                // TODO. baseline
                child_cross_offset = 0.f;
                break;
        }

        // main axis offset
        if (flip_main_axis) { child_main_offset -= child_main_size; }

        // set child offset
        slot.data.offset = _FlexHelper::_combine_offset(*this, child_main_offset, child_cross_offset);

        // update main axis offset
        child_main_offset += flip_main_axis ? -between_space : child_main_size + between_space;
    }
}

// paint
void RenderFlex::paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT
{
    // TODO. handle overflow
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
void RenderFlex::set_flex_direction(EFlexDirection value) SKR_NOEXCEPT
{
    if (_flex_direction != value)
    {
        _flex_direction = value;
        mark_needs_layout();
    }
}
void RenderFlex::set_main_axis_alignment(EMainAxisAlignment value) SKR_NOEXCEPT
{
    if (_main_axis_alignment != value)
    {
        _main_axis_alignment = value;
        mark_needs_layout();
    }
}
void RenderFlex::set_cross_axis_alignment(ECrossAxisAlignment value) SKR_NOEXCEPT
{
    if (_cross_axis_alignment != value)
    {
        _cross_axis_alignment = value;
        mark_needs_layout();
    }
}
void RenderFlex::set_main_axis_size(EMainAxisSize value) SKR_NOEXCEPT
{
    if (_main_axis_size != value)
    {
        _main_axis_size = value;
        mark_needs_layout();
    }
}

// hit test
bool RenderFlex::hit_test(HitTestResult* result, Offsetf local_position) const SKR_NOEXCEPT
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
void RenderFlex::apply_paint_transform(NotNull<const RenderObject*> child, Matrix4& transform) const SKR_NOEXCEPT
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