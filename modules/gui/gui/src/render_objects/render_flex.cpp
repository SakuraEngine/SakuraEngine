#include "SkrGui/render_objects/render_flex.hpp"

// help functions
namespace skr::gui
{
struct _FlexHelper {
    template <typename TH, typename TV>
    inline static auto _select_main(const RenderFlex& self, TH&& horizontal, TV&& vertical) -> decltype(std::declval<TH>()())
    {
        switch (self._flex_direction)
        {
            case FlexDirection::Row:
            case FlexDirection::RowReverse:
                return horizontal();
                break;
            case FlexDirection::Column:
            case FlexDirection::ColumnReverse:
                return vertical();
                break;
        }
    }
    template <typename TH, typename TV>
    inline static auto _select_cross(const RenderFlex& self, TH&& horizontal, TV&& vertical) -> decltype(std::declval<TH>()())
    {
        switch (self._flex_direction)
        {
            case FlexDirection::Row:
            case FlexDirection::RowReverse:
                return vertical();
                break;
            case FlexDirection::Column:
            case FlexDirection::ColumnReverse:
                return horizontal();
                break;
        }
    }
    inline static float _get_main_size(const RenderFlex& self, Size size) SKR_NOEXCEPT
    {
        return _select_main(
        self,
        [&]() { return size.width; },
        [&]() { return size.height; });
    }
    inline static float _get_cross_size(const RenderFlex& self, Size size) SKR_NOEXCEPT
    {
        return _select_cross(
        self, [&]() { return size.width; }, [&]() { return size.height; });
    }
    inline static Size _combine_size(const RenderFlex& self, float main_size, float cross_size) SKR_NOEXCEPT
    {
        return _select_main(
        self,
        [&]() { return Size{ main_size, cross_size }; },
        [&]() { return Size{ cross_size, main_size }; });
    }
    inline static Offset _combine_offset(const RenderFlex& self, float main_offset, float cross_offset) SKR_NOEXCEPT
    {
        return _select_main(
        self,
        [&]() { return Offset{ main_offset, cross_offset }; },
        [&]() { return Offset{ cross_offset, main_offset }; });
    }

    inline static bool _is_coord_flipped(const RenderFlex& self) SKR_NOEXCEPT
    {
        switch (self._flex_direction)
        {
            case FlexDirection::Row:
            case FlexDirection::RowReverse:
                return true;
            case FlexDirection::Column:
            case FlexDirection::ColumnReverse:
                return false;
        }
    }

    inline static bool _is_main_axis_flipped(const RenderFlex& self) SKR_NOEXCEPT
    {
        switch (self._flex_direction)
        {
            case FlexDirection::Row:
            case FlexDirection::Column:
                return false;
            case FlexDirection::RowReverse:
            case FlexDirection::ColumnReverse:
                return true;
        }
    }

    inline static bool _is_cross_axis_flipped(const RenderFlex& self) SKR_NOEXCEPT
    {
        return false;
    }

    template <typename TLayoutFunc>
    inline static void _compute_sizes(const RenderFlex& self,
                                      BoxConstraints    constraints,
                                      TLayoutFunc&&     layout_func,
                                      float&            out_main_size,
                                      float&            out_cross_size,
                                      float&            out_allocated_size)
    {
        // collect data
        const float max_main_size = _get_main_size(self, constraints.max_size());
        const bool  can_flex = max_main_size < std::numeric_limits<float>::infinity();
        const bool  is_coord_flipped = _is_coord_flipped(self);

        out_cross_size = 0;
        out_allocated_size = 0;
        float total_flex = 0;

        // Step1. Calculate non-flexible children's size
        for (const auto& slot : self._flexible_slots)
        {
            if (slot.flex > 0)
            {
                total_flex += slot.flex;
            }
            else
            {
                // solve child constraints
                BoxConstraints inner_constraints = self._cross_axis_alignment == CrossAxisAlignment::Stretch ?
                                                   is_coord_flipped ?
                                                   BoxConstraints::TightWidth(constraints.max_width) :
                                                   BoxConstraints::TightHeight(constraints.max_height) :
                                                   is_coord_flipped ?
                                                   BoxConstraints::LooseWidth(constraints.max_width) :
                                                   BoxConstraints::LooseHeight(constraints.max_height);

                // layout child
                const Size child_size = layout_func(slot.child, inner_constraints);
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
            for (const auto& slot : self._flexible_slots)
            {
                if (slot.flex > 0)
                {
                    // solve child extent
                    const float max_child_extent = can_flex ? space_per_flex * slot.flex : std::numeric_limits<float>::infinity();
                    float       min_child_extent;
                    switch (slot.flex_fit)
                    {
                        case FlexFit::Tight:
                            min_child_extent = max_child_extent;
                            break;
                        case FlexFit::Loose:
                            min_child_extent = 0.0f;
                            break;
                    }

                    // solve child constraints
                    BoxConstraints inner_constraints = self._cross_axis_alignment == CrossAxisAlignment::Stretch ?
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
                    const Size child_size = layout_func(slot.child, inner_constraints);
                    out_allocated_size += _get_main_size(self, child_size);
                    out_cross_size = std::max(out_cross_size, _get_cross_size(self, child_size));
                }
            }
        }

        out_main_size = can_flex && self._main_axis_size == MainAxisSize::Max ? max_main_size : out_allocated_size;
    }
};
} // namespace skr::gui

namespace skr::gui
{
// intrinsic size
float RenderFlex::compute_min_intrinsic_width(float height) const SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return Super::get_min_intrinsic_width(height);
}
float RenderFlex::compute_max_intrinsic_width(float height) const SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return Super::get_max_intrinsic_width(height);
}
float RenderFlex::compute_min_intrinsic_height(float width) const SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return Super::get_min_intrinsic_height(width);
}
float RenderFlex::compute_max_intrinsic_height(float width) const SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return Super::get_max_intrinsic_height(width);
}

// dry layout
Size RenderFlex::compute_dry_layout(BoxConstraints constraints) const SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return Super::compute_dry_layout(constraints);
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

    set_size(constraints().constrain(is_coord_flipped ? Size{ cross_size, main_size } : Size{ main_size, cross_size }));

    // Step6. Place each child based on main axis alignment and cross axis alignment
    float leading_space, between_space;
    {
        // calc slack & overflow
        float slack_space = main_size - allocated_size;
        _overflow = std::max(0.f, -slack_space);
        slack_space = std::max(0.f, slack_space);

        // calc leading space & between space
        const auto child_count = _flexible_slots.size();
        switch (_main_axis_alignment)
        {
            case MainAxisAlignment::Start:
                leading_space = 0.0f;
                between_space = 0.0f;
                break;
            case MainAxisAlignment::End:
                leading_space = slack_space;
                between_space = 0.0f;
                break;
            case MainAxisAlignment::Center:
                leading_space = slack_space / 2.0f;
                between_space = 0.0f;
                break;
            case MainAxisAlignment::SpaceBetween:
                leading_space = 0.0f;
                between_space = child_count > 1 ? (slack_space / (child_count - 1)) : 0.0f;
                break;
            case MainAxisAlignment::SpaceAround:
                between_space = child_count > 0 ? (slack_space / child_count) : 0.0f;
                leading_space = between_space / 2.0f;
                break;
            case MainAxisAlignment::SpaceEvenly:
                between_space = child_count > 0 ? (slack_space / (child_count + 1)) : 0.0f;
                leading_space = between_space;
                break;
        }
    }

    const bool flip_main_axis = _FlexHelper::_is_main_axis_flipped(*this);
    const bool flip_cross_axis = _FlexHelper::_is_cross_axis_flipped(*this);
    float      child_main_offset = flip_main_axis ? (main_size - leading_space) : leading_space;
    for (size_t i = 0; i < _flexible_slots.size(); ++i)
    {
        auto&       slot = _flexible_slots[i];
        const Size  child_size = slot.child->size();
        const float child_main_size = _FlexHelper::_get_main_size(*this, child_size);
        const float child_cross_size = _FlexHelper::_get_cross_size(*this, child_size);

        // cross axis offset
        float child_cross_offset;
        switch (_cross_axis_alignment)
        {
            case CrossAxisAlignment::Start:
                child_cross_offset = flip_cross_axis ? cross_size - child_cross_size : 0.f;
                break;
            case CrossAxisAlignment::End:
                child_cross_offset = flip_cross_axis ? 0.0f : cross_size - child_cross_size;
                break;
            case CrossAxisAlignment::Center:
                child_cross_offset = (cross_size - child_cross_size) / 2.f;
                break;
            case CrossAxisAlignment::Stretch:
                child_cross_offset = 0.f;
                break;
            case CrossAxisAlignment::Baseline:
                // TODO. baseline
                child_cross_offset = 0.f;
                break;
        }

        // main axis offset
        if (flip_main_axis) { child_main_offset -= child_main_size; }

        // set child offset
        slot.offset = _FlexHelper::_combine_offset(*this, child_main_offset, child_cross_offset);

        // update main axis offset
        child_main_offset += flip_main_axis ? -between_space : child_main_size + between_space;
    }
}
} // namespace skr::gui