//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!! THIS FILE IS GENERATED, ANY CHANGES WILL BE LOST !!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// BEGIN header includes
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/backend/device/device.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/backend/device/window.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/backend/resource/resource.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/backend/text/paragraph.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/build_context.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/timer_manager.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/element/component_element.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/element/element.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/element/leaf_render_object_element.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/element/multi_child_render_object_element.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/element/proxy_element.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/element/render_native_window_element.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/element/render_object_element.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/element/render_window_element.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/element/single_child_render_object_element.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/element/slot_element.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/element/stateful_element.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/element/stateless_element.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/layer/container_layer.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/layer/geometry_layer.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/layer/layer.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/layer/native_window_layer.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/layer/offset_layer.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/layer/window_layer.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/render_object/multi_child_render_object.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/render_object/render_box.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/render_object/render_native_window.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/render_object/render_object.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/render_object/render_proxy_box.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/render_object/render_proxy_box_with_hit_test_behavior.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/render_object/render_shifted_box.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/render_object/render_window.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/render_object/single_child_render_object.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/widget/leaf_render_object_widget.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/widget/multi_child_render_object_widget.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/widget/proxy_widget.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/widget/render_native_window_widget.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/widget/render_object_widget.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/widget/render_window_widget.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/widget/single_child_render_object_widget.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/widget/slot_widget.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/widget/stateful_widget.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/widget/stateless_widget.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/framework/widget/widget.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/math/flex_layout.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/math/stack_layout.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/render_objects/render_colored_box.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/render_objects/render_color_picker.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/render_objects/render_constrained_box.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/render_objects/render_flex.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/render_objects/render_grid_paper.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/render_objects/render_mouse_region.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/render_objects/render_positioned.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/render_objects/render_stack.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/render_objects/render_text.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/system/input/event.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/system/input/hit_test.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/system/input/input_manager.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/system/input/pointer_event.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/system/input/gesture/click_gesture_recognizer.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/system/input/gesture/gesture_arena.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/system/input/gesture/gesture_arena_manager.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/system/input/gesture/gesture_detector.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/system/input/gesture/gesture_recognizer.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/system/input/gesture/pointer_gesture_recognizer.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/widgets/colored_box.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/widgets/color_picker.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/widgets/flex.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/widgets/flex_slot.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/widgets/grid_paper.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/widgets/mouse_region.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/widgets/positioned.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/widgets/sized_box.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/widgets/stack.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/widgets/text.hpp"
#include "D:/workspace/project/SakuraEngine/modules/gui/gui/include/SkrGui/_private/paragraph.hpp"
// END header includes

// BEGIN push diagnostic
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicitly-unsigned-literal"
#endif
// END push diagnostic
// BEGIN RTTR GENERATED
#include "SkrRT/rttr/type_loader/type_loader.hpp"
#include "SkrRT/rttr/type/record_type.hpp"
#include "SkrRT/rttr/exec_static.hpp"
#include "SkrRT/rttr/type_loader/enum_type_from_traits_loader.hpp"
#include "SkrContainers/tuple.hpp"

namespace skr::rttr 
{
span<EnumItem<skr::gui::EFlexDirection>> EnumTraits<skr::gui::EFlexDirection>::items()
{
    static EnumItem<skr::gui::EFlexDirection> items[] = {
        {u8"Row", skr::gui::EFlexDirection::Row},
        {u8"RowReverse", skr::gui::EFlexDirection::RowReverse},
        {u8"Column", skr::gui::EFlexDirection::Column},
        {u8"ColumnReverse", skr::gui::EFlexDirection::ColumnReverse},
    };
    return items;
}
skr::StringView EnumTraits<skr::gui::EFlexDirection>::to_string(const skr::gui::EFlexDirection& value)
{
    switch (value)
    {
    case skr::gui::EFlexDirection::Row: return u8"Row";
    case skr::gui::EFlexDirection::RowReverse: return u8"RowReverse";
    case skr::gui::EFlexDirection::Column: return u8"Column";
    case skr::gui::EFlexDirection::ColumnReverse: return u8"ColumnReverse";
    default: SKR_UNREACHABLE_CODE(); return u8"skr::gui::EFlexDirection::INVALID_ENUMERATOR";
    }
}
bool EnumTraits<skr::gui::EFlexDirection>::from_string(skr::StringView str, skr::gui::EFlexDirection& value)
{
    const auto hash = skr_hash64(str.raw().data(), str.size(), 0);
    switch(hash)
    {
        case skr::consteval_hash(u8"Row"): if(str == u8"Row") value = skr::gui::EFlexDirection::Row; return true;
        case skr::consteval_hash(u8"RowReverse"): if(str == u8"RowReverse") value = skr::gui::EFlexDirection::RowReverse; return true;
        case skr::consteval_hash(u8"Column"): if(str == u8"Column") value = skr::gui::EFlexDirection::Column; return true;
        case skr::consteval_hash(u8"ColumnReverse"): if(str == u8"ColumnReverse") value = skr::gui::EFlexDirection::ColumnReverse; return true;
        default:
            return false;
    }
}
span<EnumItem<skr::gui::EMainAxisAlignment>> EnumTraits<skr::gui::EMainAxisAlignment>::items()
{
    static EnumItem<skr::gui::EMainAxisAlignment> items[] = {
        {u8"Start", skr::gui::EMainAxisAlignment::Start},
        {u8"End", skr::gui::EMainAxisAlignment::End},
        {u8"Center", skr::gui::EMainAxisAlignment::Center},
        {u8"SpaceBetween", skr::gui::EMainAxisAlignment::SpaceBetween},
        {u8"SpaceAround", skr::gui::EMainAxisAlignment::SpaceAround},
        {u8"SpaceEvenly", skr::gui::EMainAxisAlignment::SpaceEvenly},
    };
    return items;
}
skr::StringView EnumTraits<skr::gui::EMainAxisAlignment>::to_string(const skr::gui::EMainAxisAlignment& value)
{
    switch (value)
    {
    case skr::gui::EMainAxisAlignment::Start: return u8"Start";
    case skr::gui::EMainAxisAlignment::End: return u8"End";
    case skr::gui::EMainAxisAlignment::Center: return u8"Center";
    case skr::gui::EMainAxisAlignment::SpaceBetween: return u8"SpaceBetween";
    case skr::gui::EMainAxisAlignment::SpaceAround: return u8"SpaceAround";
    case skr::gui::EMainAxisAlignment::SpaceEvenly: return u8"SpaceEvenly";
    default: SKR_UNREACHABLE_CODE(); return u8"skr::gui::EMainAxisAlignment::INVALID_ENUMERATOR";
    }
}
bool EnumTraits<skr::gui::EMainAxisAlignment>::from_string(skr::StringView str, skr::gui::EMainAxisAlignment& value)
{
    const auto hash = skr_hash64(str.raw().data(), str.size(), 0);
    switch(hash)
    {
        case skr::consteval_hash(u8"Start"): if(str == u8"Start") value = skr::gui::EMainAxisAlignment::Start; return true;
        case skr::consteval_hash(u8"End"): if(str == u8"End") value = skr::gui::EMainAxisAlignment::End; return true;
        case skr::consteval_hash(u8"Center"): if(str == u8"Center") value = skr::gui::EMainAxisAlignment::Center; return true;
        case skr::consteval_hash(u8"SpaceBetween"): if(str == u8"SpaceBetween") value = skr::gui::EMainAxisAlignment::SpaceBetween; return true;
        case skr::consteval_hash(u8"SpaceAround"): if(str == u8"SpaceAround") value = skr::gui::EMainAxisAlignment::SpaceAround; return true;
        case skr::consteval_hash(u8"SpaceEvenly"): if(str == u8"SpaceEvenly") value = skr::gui::EMainAxisAlignment::SpaceEvenly; return true;
        default:
            return false;
    }
}
span<EnumItem<skr::gui::ECrossAxisAlignment>> EnumTraits<skr::gui::ECrossAxisAlignment>::items()
{
    static EnumItem<skr::gui::ECrossAxisAlignment> items[] = {
        {u8"Start", skr::gui::ECrossAxisAlignment::Start},
        {u8"End", skr::gui::ECrossAxisAlignment::End},
        {u8"Center", skr::gui::ECrossAxisAlignment::Center},
        {u8"Stretch", skr::gui::ECrossAxisAlignment::Stretch},
        {u8"Baseline", skr::gui::ECrossAxisAlignment::Baseline},
    };
    return items;
}
skr::StringView EnumTraits<skr::gui::ECrossAxisAlignment>::to_string(const skr::gui::ECrossAxisAlignment& value)
{
    switch (value)
    {
    case skr::gui::ECrossAxisAlignment::Start: return u8"Start";
    case skr::gui::ECrossAxisAlignment::End: return u8"End";
    case skr::gui::ECrossAxisAlignment::Center: return u8"Center";
    case skr::gui::ECrossAxisAlignment::Stretch: return u8"Stretch";
    case skr::gui::ECrossAxisAlignment::Baseline: return u8"Baseline";
    default: SKR_UNREACHABLE_CODE(); return u8"skr::gui::ECrossAxisAlignment::INVALID_ENUMERATOR";
    }
}
bool EnumTraits<skr::gui::ECrossAxisAlignment>::from_string(skr::StringView str, skr::gui::ECrossAxisAlignment& value)
{
    const auto hash = skr_hash64(str.raw().data(), str.size(), 0);
    switch(hash)
    {
        case skr::consteval_hash(u8"Start"): if(str == u8"Start") value = skr::gui::ECrossAxisAlignment::Start; return true;
        case skr::consteval_hash(u8"End"): if(str == u8"End") value = skr::gui::ECrossAxisAlignment::End; return true;
        case skr::consteval_hash(u8"Center"): if(str == u8"Center") value = skr::gui::ECrossAxisAlignment::Center; return true;
        case skr::consteval_hash(u8"Stretch"): if(str == u8"Stretch") value = skr::gui::ECrossAxisAlignment::Stretch; return true;
        case skr::consteval_hash(u8"Baseline"): if(str == u8"Baseline") value = skr::gui::ECrossAxisAlignment::Baseline; return true;
        default:
            return false;
    }
}
span<EnumItem<skr::gui::EMainAxisSize>> EnumTraits<skr::gui::EMainAxisSize>::items()
{
    static EnumItem<skr::gui::EMainAxisSize> items[] = {
        {u8"Min", skr::gui::EMainAxisSize::Min},
        {u8"Max", skr::gui::EMainAxisSize::Max},
    };
    return items;
}
skr::StringView EnumTraits<skr::gui::EMainAxisSize>::to_string(const skr::gui::EMainAxisSize& value)
{
    switch (value)
    {
    case skr::gui::EMainAxisSize::Min: return u8"Min";
    case skr::gui::EMainAxisSize::Max: return u8"Max";
    default: SKR_UNREACHABLE_CODE(); return u8"skr::gui::EMainAxisSize::INVALID_ENUMERATOR";
    }
}
bool EnumTraits<skr::gui::EMainAxisSize>::from_string(skr::StringView str, skr::gui::EMainAxisSize& value)
{
    const auto hash = skr_hash64(str.raw().data(), str.size(), 0);
    switch(hash)
    {
        case skr::consteval_hash(u8"Min"): if(str == u8"Min") value = skr::gui::EMainAxisSize::Min; return true;
        case skr::consteval_hash(u8"Max"): if(str == u8"Max") value = skr::gui::EMainAxisSize::Max; return true;
        default:
            return false;
    }
}
span<EnumItem<skr::gui::EFlexFit>> EnumTraits<skr::gui::EFlexFit>::items()
{
    static EnumItem<skr::gui::EFlexFit> items[] = {
        {u8"Tight", skr::gui::EFlexFit::Tight},
        {u8"Loose", skr::gui::EFlexFit::Loose},
    };
    return items;
}
skr::StringView EnumTraits<skr::gui::EFlexFit>::to_string(const skr::gui::EFlexFit& value)
{
    switch (value)
    {
    case skr::gui::EFlexFit::Tight: return u8"Tight";
    case skr::gui::EFlexFit::Loose: return u8"Loose";
    default: SKR_UNREACHABLE_CODE(); return u8"skr::gui::EFlexFit::INVALID_ENUMERATOR";
    }
}
bool EnumTraits<skr::gui::EFlexFit>::from_string(skr::StringView str, skr::gui::EFlexFit& value)
{
    const auto hash = skr_hash64(str.raw().data(), str.size(), 0);
    switch(hash)
    {
        case skr::consteval_hash(u8"Tight"): if(str == u8"Tight") value = skr::gui::EFlexFit::Tight; return true;
        case skr::consteval_hash(u8"Loose"): if(str == u8"Loose") value = skr::gui::EFlexFit::Loose; return true;
        default:
            return false;
    }
}
span<EnumItem<skr::gui::EPositionalFit>> EnumTraits<skr::gui::EPositionalFit>::items()
{
    static EnumItem<skr::gui::EPositionalFit> items[] = {
        {u8"Loose", skr::gui::EPositionalFit::Loose},
        {u8"Expand", skr::gui::EPositionalFit::Expand},
        {u8"PassThrough", skr::gui::EPositionalFit::PassThrough},
    };
    return items;
}
skr::StringView EnumTraits<skr::gui::EPositionalFit>::to_string(const skr::gui::EPositionalFit& value)
{
    switch (value)
    {
    case skr::gui::EPositionalFit::Loose: return u8"Loose";
    case skr::gui::EPositionalFit::Expand: return u8"Expand";
    case skr::gui::EPositionalFit::PassThrough: return u8"PassThrough";
    default: SKR_UNREACHABLE_CODE(); return u8"skr::gui::EPositionalFit::INVALID_ENUMERATOR";
    }
}
bool EnumTraits<skr::gui::EPositionalFit>::from_string(skr::StringView str, skr::gui::EPositionalFit& value)
{
    const auto hash = skr_hash64(str.raw().data(), str.size(), 0);
    switch(hash)
    {
        case skr::consteval_hash(u8"Loose"): if(str == u8"Loose") value = skr::gui::EPositionalFit::Loose; return true;
        case skr::consteval_hash(u8"Expand"): if(str == u8"Expand") value = skr::gui::EPositionalFit::Expand; return true;
        case skr::consteval_hash(u8"PassThrough"): if(str == u8"PassThrough") value = skr::gui::EPositionalFit::PassThrough; return true;
        default:
            return false;
    }
}
span<EnumItem<skr::gui::EStackSize>> EnumTraits<skr::gui::EStackSize>::items()
{
    static EnumItem<skr::gui::EStackSize> items[] = {
        {u8"Shrink", skr::gui::EStackSize::Shrink},
        {u8"Expand", skr::gui::EStackSize::Expand},
    };
    return items;
}
skr::StringView EnumTraits<skr::gui::EStackSize>::to_string(const skr::gui::EStackSize& value)
{
    switch (value)
    {
    case skr::gui::EStackSize::Shrink: return u8"Shrink";
    case skr::gui::EStackSize::Expand: return u8"Expand";
    default: SKR_UNREACHABLE_CODE(); return u8"skr::gui::EStackSize::INVALID_ENUMERATOR";
    }
}
bool EnumTraits<skr::gui::EStackSize>::from_string(skr::StringView str, skr::gui::EStackSize& value)
{
    const auto hash = skr_hash64(str.raw().data(), str.size(), 0);
    switch(hash)
    {
        case skr::consteval_hash(u8"Shrink"): if(str == u8"Shrink") value = skr::gui::EStackSize::Shrink; return true;
        case skr::consteval_hash(u8"Expand"): if(str == u8"Expand") value = skr::gui::EStackSize::Expand; return true;
        default:
            return false;
    }
}
span<EnumItem<skr::gui::EEventRoutePhase>> EnumTraits<skr::gui::EEventRoutePhase>::items()
{
    static EnumItem<skr::gui::EEventRoutePhase> items[] = {
        {u8"None", skr::gui::EEventRoutePhase::None},
        {u8"TrickleDown", skr::gui::EEventRoutePhase::TrickleDown},
        {u8"Reach", skr::gui::EEventRoutePhase::Reach},
        {u8"Broadcast", skr::gui::EEventRoutePhase::Broadcast},
        {u8"BubbleUp", skr::gui::EEventRoutePhase::BubbleUp},
        {u8"All", skr::gui::EEventRoutePhase::All},
        {u8"NoBroadcast", skr::gui::EEventRoutePhase::NoBroadcast},
    };
    return items;
}
skr::StringView EnumTraits<skr::gui::EEventRoutePhase>::to_string(const skr::gui::EEventRoutePhase& value)
{
    switch (value)
    {
    case skr::gui::EEventRoutePhase::None: return u8"None";
    case skr::gui::EEventRoutePhase::TrickleDown: return u8"TrickleDown";
    case skr::gui::EEventRoutePhase::Reach: return u8"Reach";
    case skr::gui::EEventRoutePhase::Broadcast: return u8"Broadcast";
    case skr::gui::EEventRoutePhase::BubbleUp: return u8"BubbleUp";
    case skr::gui::EEventRoutePhase::All: return u8"All";
    case skr::gui::EEventRoutePhase::NoBroadcast: return u8"NoBroadcast";
    default: SKR_UNREACHABLE_CODE(); return u8"skr::gui::EEventRoutePhase::INVALID_ENUMERATOR";
    }
}
bool EnumTraits<skr::gui::EEventRoutePhase>::from_string(skr::StringView str, skr::gui::EEventRoutePhase& value)
{
    const auto hash = skr_hash64(str.raw().data(), str.size(), 0);
    switch(hash)
    {
        case skr::consteval_hash(u8"None"): if(str == u8"None") value = skr::gui::EEventRoutePhase::None; return true;
        case skr::consteval_hash(u8"TrickleDown"): if(str == u8"TrickleDown") value = skr::gui::EEventRoutePhase::TrickleDown; return true;
        case skr::consteval_hash(u8"Reach"): if(str == u8"Reach") value = skr::gui::EEventRoutePhase::Reach; return true;
        case skr::consteval_hash(u8"Broadcast"): if(str == u8"Broadcast") value = skr::gui::EEventRoutePhase::Broadcast; return true;
        case skr::consteval_hash(u8"BubbleUp"): if(str == u8"BubbleUp") value = skr::gui::EEventRoutePhase::BubbleUp; return true;
        case skr::consteval_hash(u8"All"): if(str == u8"All") value = skr::gui::EEventRoutePhase::All; return true;
        case skr::consteval_hash(u8"NoBroadcast"): if(str == u8"NoBroadcast") value = skr::gui::EEventRoutePhase::NoBroadcast; return true;
        default:
            return false;
    }
}
span<EnumItem<skr::gui::EEventSource>> EnumTraits<skr::gui::EEventSource>::items()
{
    static EnumItem<skr::gui::EEventSource> items[] = {
        {u8"None", skr::gui::EEventSource::None},
        {u8"Device", skr::gui::EEventSource::Device},
        {u8"Gesture", skr::gui::EEventSource::Gesture},
        {u8"Framework", skr::gui::EEventSource::Framework},
    };
    return items;
}
skr::StringView EnumTraits<skr::gui::EEventSource>::to_string(const skr::gui::EEventSource& value)
{
    switch (value)
    {
    case skr::gui::EEventSource::None: return u8"None";
    case skr::gui::EEventSource::Device: return u8"Device";
    case skr::gui::EEventSource::Gesture: return u8"Gesture";
    case skr::gui::EEventSource::Framework: return u8"Framework";
    default: SKR_UNREACHABLE_CODE(); return u8"skr::gui::EEventSource::INVALID_ENUMERATOR";
    }
}
bool EnumTraits<skr::gui::EEventSource>::from_string(skr::StringView str, skr::gui::EEventSource& value)
{
    const auto hash = skr_hash64(str.raw().data(), str.size(), 0);
    switch(hash)
    {
        case skr::consteval_hash(u8"None"): if(str == u8"None") value = skr::gui::EEventSource::None; return true;
        case skr::consteval_hash(u8"Device"): if(str == u8"Device") value = skr::gui::EEventSource::Device; return true;
        case skr::consteval_hash(u8"Gesture"): if(str == u8"Gesture") value = skr::gui::EEventSource::Gesture; return true;
        case skr::consteval_hash(u8"Framework"): if(str == u8"Framework") value = skr::gui::EEventSource::Framework; return true;
        default:
            return false;
    }
}
span<EnumItem<skr::gui::EHitTestBehavior>> EnumTraits<skr::gui::EHitTestBehavior>::items()
{
    static EnumItem<skr::gui::EHitTestBehavior> items[] = {
        {u8"defer_to_child", skr::gui::EHitTestBehavior::defer_to_child},
        {u8"opaque", skr::gui::EHitTestBehavior::opaque},
        {u8"transparent", skr::gui::EHitTestBehavior::transparent},
    };
    return items;
}
skr::StringView EnumTraits<skr::gui::EHitTestBehavior>::to_string(const skr::gui::EHitTestBehavior& value)
{
    switch (value)
    {
    case skr::gui::EHitTestBehavior::defer_to_child: return u8"defer_to_child";
    case skr::gui::EHitTestBehavior::opaque: return u8"opaque";
    case skr::gui::EHitTestBehavior::transparent: return u8"transparent";
    default: SKR_UNREACHABLE_CODE(); return u8"skr::gui::EHitTestBehavior::INVALID_ENUMERATOR";
    }
}
bool EnumTraits<skr::gui::EHitTestBehavior>::from_string(skr::StringView str, skr::gui::EHitTestBehavior& value)
{
    const auto hash = skr_hash64(str.raw().data(), str.size(), 0);
    switch(hash)
    {
        case skr::consteval_hash(u8"defer_to_child"): if(str == u8"defer_to_child") value = skr::gui::EHitTestBehavior::defer_to_child; return true;
        case skr::consteval_hash(u8"opaque"): if(str == u8"opaque") value = skr::gui::EHitTestBehavior::opaque; return true;
        case skr::consteval_hash(u8"transparent"): if(str == u8"transparent") value = skr::gui::EHitTestBehavior::transparent; return true;
        default:
            return false;
    }
}
span<EnumItem<skr::gui::EPointerDeviceType>> EnumTraits<skr::gui::EPointerDeviceType>::items()
{
    static EnumItem<skr::gui::EPointerDeviceType> items[] = {
        {u8"Unknown", skr::gui::EPointerDeviceType::Unknown},
        {u8"Mouse", skr::gui::EPointerDeviceType::Mouse},
        {u8"TrackPad", skr::gui::EPointerDeviceType::TrackPad},
    };
    return items;
}
skr::StringView EnumTraits<skr::gui::EPointerDeviceType>::to_string(const skr::gui::EPointerDeviceType& value)
{
    switch (value)
    {
    case skr::gui::EPointerDeviceType::Unknown: return u8"Unknown";
    case skr::gui::EPointerDeviceType::Mouse: return u8"Mouse";
    case skr::gui::EPointerDeviceType::TrackPad: return u8"TrackPad";
    default: SKR_UNREACHABLE_CODE(); return u8"skr::gui::EPointerDeviceType::INVALID_ENUMERATOR";
    }
}
bool EnumTraits<skr::gui::EPointerDeviceType>::from_string(skr::StringView str, skr::gui::EPointerDeviceType& value)
{
    const auto hash = skr_hash64(str.raw().data(), str.size(), 0);
    switch(hash)
    {
        case skr::consteval_hash(u8"Unknown"): if(str == u8"Unknown") value = skr::gui::EPointerDeviceType::Unknown; return true;
        case skr::consteval_hash(u8"Mouse"): if(str == u8"Mouse") value = skr::gui::EPointerDeviceType::Mouse; return true;
        case skr::consteval_hash(u8"TrackPad"): if(str == u8"TrackPad") value = skr::gui::EPointerDeviceType::TrackPad; return true;
        default:
            return false;
    }
}
span<EnumItem<skr::gui::EPointerButton>> EnumTraits<skr::gui::EPointerButton>::items()
{
    static EnumItem<skr::gui::EPointerButton> items[] = {
        {u8"Unknown", skr::gui::EPointerButton::Unknown},
        {u8"Left", skr::gui::EPointerButton::Left},
        {u8"Right", skr::gui::EPointerButton::Right},
        {u8"Middle", skr::gui::EPointerButton::Middle},
        {u8"X1B", skr::gui::EPointerButton::X1B},
        {u8"X2B", skr::gui::EPointerButton::X2B},
        {u8"X3B", skr::gui::EPointerButton::X3B},
        {u8"X4B", skr::gui::EPointerButton::X4B},
        {u8"X5B", skr::gui::EPointerButton::X5B},
    };
    return items;
}
skr::StringView EnumTraits<skr::gui::EPointerButton>::to_string(const skr::gui::EPointerButton& value)
{
    switch (value)
    {
    case skr::gui::EPointerButton::Unknown: return u8"Unknown";
    case skr::gui::EPointerButton::Left: return u8"Left";
    case skr::gui::EPointerButton::Right: return u8"Right";
    case skr::gui::EPointerButton::Middle: return u8"Middle";
    case skr::gui::EPointerButton::X1B: return u8"X1B";
    case skr::gui::EPointerButton::X2B: return u8"X2B";
    case skr::gui::EPointerButton::X3B: return u8"X3B";
    case skr::gui::EPointerButton::X4B: return u8"X4B";
    case skr::gui::EPointerButton::X5B: return u8"X5B";
    default: SKR_UNREACHABLE_CODE(); return u8"skr::gui::EPointerButton::INVALID_ENUMERATOR";
    }
}
bool EnumTraits<skr::gui::EPointerButton>::from_string(skr::StringView str, skr::gui::EPointerButton& value)
{
    const auto hash = skr_hash64(str.raw().data(), str.size(), 0);
    switch(hash)
    {
        case skr::consteval_hash(u8"Unknown"): if(str == u8"Unknown") value = skr::gui::EPointerButton::Unknown; return true;
        case skr::consteval_hash(u8"Left"): if(str == u8"Left") value = skr::gui::EPointerButton::Left; return true;
        case skr::consteval_hash(u8"Right"): if(str == u8"Right") value = skr::gui::EPointerButton::Right; return true;
        case skr::consteval_hash(u8"Middle"): if(str == u8"Middle") value = skr::gui::EPointerButton::Middle; return true;
        case skr::consteval_hash(u8"X1B"): if(str == u8"X1B") value = skr::gui::EPointerButton::X1B; return true;
        case skr::consteval_hash(u8"X2B"): if(str == u8"X2B") value = skr::gui::EPointerButton::X2B; return true;
        case skr::consteval_hash(u8"X3B"): if(str == u8"X3B") value = skr::gui::EPointerButton::X3B; return true;
        case skr::consteval_hash(u8"X4B"): if(str == u8"X4B") value = skr::gui::EPointerButton::X4B; return true;
        case skr::consteval_hash(u8"X5B"): if(str == u8"X5B") value = skr::gui::EPointerButton::X5B; return true;
        default:
            return false;
    }
}
span<EnumItem<skr::gui::EPointerModifier>> EnumTraits<skr::gui::EPointerModifier>::items()
{
    static EnumItem<skr::gui::EPointerModifier> items[] = {
        {u8"Unknown", skr::gui::EPointerModifier::Unknown},
        {u8"LeftCtrl", skr::gui::EPointerModifier::LeftCtrl},
        {u8"RightCtrl", skr::gui::EPointerModifier::RightCtrl},
        {u8"LeftAlt", skr::gui::EPointerModifier::LeftAlt},
        {u8"RightAlt", skr::gui::EPointerModifier::RightAlt},
        {u8"LeftShift", skr::gui::EPointerModifier::LeftShift},
        {u8"RightShift", skr::gui::EPointerModifier::RightShift},
        {u8"LeftOption", skr::gui::EPointerModifier::LeftOption},
        {u8"RightOption", skr::gui::EPointerModifier::RightOption},
        {u8"LeftCommand", skr::gui::EPointerModifier::LeftCommand},
        {u8"RightCommand", skr::gui::EPointerModifier::RightCommand},
    };
    return items;
}
skr::StringView EnumTraits<skr::gui::EPointerModifier>::to_string(const skr::gui::EPointerModifier& value)
{
    switch (value)
    {
    case skr::gui::EPointerModifier::Unknown: return u8"Unknown";
    case skr::gui::EPointerModifier::LeftCtrl: return u8"LeftCtrl";
    case skr::gui::EPointerModifier::RightCtrl: return u8"RightCtrl";
    case skr::gui::EPointerModifier::LeftAlt: return u8"LeftAlt";
    case skr::gui::EPointerModifier::RightAlt: return u8"RightAlt";
    case skr::gui::EPointerModifier::LeftShift: return u8"LeftShift";
    case skr::gui::EPointerModifier::RightShift: return u8"RightShift";
    case skr::gui::EPointerModifier::LeftOption: return u8"LeftOption";
    case skr::gui::EPointerModifier::RightOption: return u8"RightOption";
    case skr::gui::EPointerModifier::LeftCommand: return u8"LeftCommand";
    case skr::gui::EPointerModifier::RightCommand: return u8"RightCommand";
    default: SKR_UNREACHABLE_CODE(); return u8"skr::gui::EPointerModifier::INVALID_ENUMERATOR";
    }
}
bool EnumTraits<skr::gui::EPointerModifier>::from_string(skr::StringView str, skr::gui::EPointerModifier& value)
{
    const auto hash = skr_hash64(str.raw().data(), str.size(), 0);
    switch(hash)
    {
        case skr::consteval_hash(u8"Unknown"): if(str == u8"Unknown") value = skr::gui::EPointerModifier::Unknown; return true;
        case skr::consteval_hash(u8"LeftCtrl"): if(str == u8"LeftCtrl") value = skr::gui::EPointerModifier::LeftCtrl; return true;
        case skr::consteval_hash(u8"RightCtrl"): if(str == u8"RightCtrl") value = skr::gui::EPointerModifier::RightCtrl; return true;
        case skr::consteval_hash(u8"LeftAlt"): if(str == u8"LeftAlt") value = skr::gui::EPointerModifier::LeftAlt; return true;
        case skr::consteval_hash(u8"RightAlt"): if(str == u8"RightAlt") value = skr::gui::EPointerModifier::RightAlt; return true;
        case skr::consteval_hash(u8"LeftShift"): if(str == u8"LeftShift") value = skr::gui::EPointerModifier::LeftShift; return true;
        case skr::consteval_hash(u8"RightShift"): if(str == u8"RightShift") value = skr::gui::EPointerModifier::RightShift; return true;
        case skr::consteval_hash(u8"LeftOption"): if(str == u8"LeftOption") value = skr::gui::EPointerModifier::LeftOption; return true;
        case skr::consteval_hash(u8"RightOption"): if(str == u8"RightOption") value = skr::gui::EPointerModifier::RightOption; return true;
        case skr::consteval_hash(u8"LeftCommand"): if(str == u8"LeftCommand") value = skr::gui::EPointerModifier::LeftCommand; return true;
        case skr::consteval_hash(u8"RightCommand"): if(str == u8"RightCommand") value = skr::gui::EPointerModifier::RightCommand; return true;
        default:
            return false;
    }
}
span<EnumItem<skr::gui::GestureArenaState>> EnumTraits<skr::gui::GestureArenaState>::items()
{
    static EnumItem<skr::gui::GestureArenaState> items[] = {
        {u8"Opened", skr::gui::GestureArenaState::Opened},
        {u8"Closed", skr::gui::GestureArenaState::Closed},
        {u8"Resolved", skr::gui::GestureArenaState::Resolved},
    };
    return items;
}
skr::StringView EnumTraits<skr::gui::GestureArenaState>::to_string(const skr::gui::GestureArenaState& value)
{
    switch (value)
    {
    case skr::gui::GestureArenaState::Opened: return u8"Opened";
    case skr::gui::GestureArenaState::Closed: return u8"Closed";
    case skr::gui::GestureArenaState::Resolved: return u8"Resolved";
    default: SKR_UNREACHABLE_CODE(); return u8"skr::gui::GestureArenaState::INVALID_ENUMERATOR";
    }
}
bool EnumTraits<skr::gui::GestureArenaState>::from_string(skr::StringView str, skr::gui::GestureArenaState& value)
{
    const auto hash = skr_hash64(str.raw().data(), str.size(), 0);
    switch(hash)
    {
        case skr::consteval_hash(u8"Opened"): if(str == u8"Opened") value = skr::gui::GestureArenaState::Opened; return true;
        case skr::consteval_hash(u8"Closed"): if(str == u8"Closed") value = skr::gui::GestureArenaState::Closed; return true;
        case skr::consteval_hash(u8"Resolved"): if(str == u8"Resolved") value = skr::gui::GestureArenaState::Resolved; return true;
        default:
            return false;
    }
}
}

SKR_RTTR_EXEC_STATIC
{
    using namespace ::skr::rttr;

    static struct InternalTypeLoader_skr_gui_INativeDevice : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::INativeDevice>::get_name(),
                RTTRTraits<::skr::gui::INativeDevice>::get_guid(),
                sizeof(skr::gui::INativeDevice),
                alignof(skr::gui::INativeDevice),
                make_record_basic_method_table<skr::gui::INativeDevice>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::rttr::IObject>::get_guid(), {RTTRTraits<skr::rttr::IObject>::get_type(), +[](void* p) -> void* { return static_cast<skr::rttr::IObject*>(reinterpret_cast<skr::gui::INativeDevice*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_INativeDevice;
    register_type_loader(RTTRTraits<skr::gui::INativeDevice>::get_guid(), &LOADER__skr_gui_INativeDevice);
    static struct InternalTypeLoader_skr_gui_INativeWindow : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::INativeWindow>::get_name(),
                RTTRTraits<::skr::gui::INativeWindow>::get_guid(),
                sizeof(skr::gui::INativeWindow),
                alignof(skr::gui::INativeWindow),
                make_record_basic_method_table<skr::gui::INativeWindow>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::rttr::IObject>::get_guid(), {RTTRTraits<skr::rttr::IObject>::get_type(), +[](void* p) -> void* { return static_cast<skr::rttr::IObject*>(reinterpret_cast<skr::gui::INativeWindow*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_INativeWindow;
    register_type_loader(RTTRTraits<skr::gui::INativeWindow>::get_guid(), &LOADER__skr_gui_INativeWindow);
    static struct InternalTypeLoader_skr_gui_IResource : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::IResource>::get_name(),
                RTTRTraits<::skr::gui::IResource>::get_guid(),
                sizeof(skr::gui::IResource),
                alignof(skr::gui::IResource),
                make_record_basic_method_table<skr::gui::IResource>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::rttr::IObject>::get_guid(), {RTTRTraits<skr::rttr::IObject>::get_type(), +[](void* p) -> void* { return static_cast<skr::rttr::IObject*>(reinterpret_cast<skr::gui::IResource*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_IResource;
    register_type_loader(RTTRTraits<skr::gui::IResource>::get_guid(), &LOADER__skr_gui_IResource);
    static struct InternalTypeLoader_skr_gui_ISurface : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::ISurface>::get_name(),
                RTTRTraits<::skr::gui::ISurface>::get_guid(),
                sizeof(skr::gui::ISurface),
                alignof(skr::gui::ISurface),
                make_record_basic_method_table<skr::gui::ISurface>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::IResource>::get_guid(), {RTTRTraits<skr::gui::IResource>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::IResource*>(reinterpret_cast<skr::gui::ISurface*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_ISurface;
    register_type_loader(RTTRTraits<skr::gui::ISurface>::get_guid(), &LOADER__skr_gui_ISurface);
    static struct InternalTypeLoader_skr_gui_IMaterial : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::IMaterial>::get_name(),
                RTTRTraits<::skr::gui::IMaterial>::get_guid(),
                sizeof(skr::gui::IMaterial),
                alignof(skr::gui::IMaterial),
                make_record_basic_method_table<skr::gui::IMaterial>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::ISurface>::get_guid(), {RTTRTraits<skr::gui::ISurface>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::ISurface*>(reinterpret_cast<skr::gui::IMaterial*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_IMaterial;
    register_type_loader(RTTRTraits<skr::gui::IMaterial>::get_guid(), &LOADER__skr_gui_IMaterial);
    static struct InternalTypeLoader_skr_gui_IImage : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::IImage>::get_name(),
                RTTRTraits<::skr::gui::IImage>::get_guid(),
                sizeof(skr::gui::IImage),
                alignof(skr::gui::IImage),
                make_record_basic_method_table<skr::gui::IImage>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::ISurface>::get_guid(), {RTTRTraits<skr::gui::ISurface>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::ISurface*>(reinterpret_cast<skr::gui::IImage*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_IImage;
    register_type_loader(RTTRTraits<skr::gui::IImage>::get_guid(), &LOADER__skr_gui_IImage);
    static struct InternalTypeLoader_skr_gui_IUpdatableImage : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::IUpdatableImage>::get_name(),
                RTTRTraits<::skr::gui::IUpdatableImage>::get_guid(),
                sizeof(skr::gui::IUpdatableImage),
                alignof(skr::gui::IUpdatableImage),
                make_record_basic_method_table<skr::gui::IUpdatableImage>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::IImage>::get_guid(), {RTTRTraits<skr::gui::IImage>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::IImage*>(reinterpret_cast<skr::gui::IUpdatableImage*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_IUpdatableImage;
    register_type_loader(RTTRTraits<skr::gui::IUpdatableImage>::get_guid(), &LOADER__skr_gui_IUpdatableImage);
    static struct InternalTypeLoader_skr_gui_IParagraph : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::IParagraph>::get_name(),
                RTTRTraits<::skr::gui::IParagraph>::get_guid(),
                sizeof(skr::gui::IParagraph),
                alignof(skr::gui::IParagraph),
                make_record_basic_method_table<skr::gui::IParagraph>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::rttr::IObject>::get_guid(), {RTTRTraits<skr::rttr::IObject>::get_type(), +[](void* p) -> void* { return static_cast<skr::rttr::IObject*>(reinterpret_cast<skr::gui::IParagraph*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_IParagraph;
    register_type_loader(RTTRTraits<skr::gui::IParagraph>::get_guid(), &LOADER__skr_gui_IParagraph);
    static struct InternalTypeLoader_skr_gui_IBuildContext : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::IBuildContext>::get_name(),
                RTTRTraits<::skr::gui::IBuildContext>::get_guid(),
                sizeof(skr::gui::IBuildContext),
                alignof(skr::gui::IBuildContext),
                make_record_basic_method_table<skr::gui::IBuildContext>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::rttr::IObject>::get_guid(), {RTTRTraits<skr::rttr::IObject>::get_type(), +[](void* p) -> void* { return static_cast<skr::rttr::IObject*>(reinterpret_cast<skr::gui::IBuildContext*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_IBuildContext;
    register_type_loader(RTTRTraits<skr::gui::IBuildContext>::get_guid(), &LOADER__skr_gui_IBuildContext);
    static struct InternalTypeLoader_skr_gui_TimerSignalData : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::TimerSignalData>::get_name(),
                RTTRTraits<::skr::gui::TimerSignalData>::get_guid(),
                sizeof(skr::gui::TimerSignalData),
                alignof(skr::gui::TimerSignalData),
                make_record_basic_method_table<skr::gui::TimerSignalData>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);



        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_TimerSignalData;
    register_type_loader(RTTRTraits<skr::gui::TimerSignalData>::get_guid(), &LOADER__skr_gui_TimerSignalData);
    static struct InternalTypeLoader_skr_gui_Timer : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::Timer>::get_name(),
                RTTRTraits<::skr::gui::Timer>::get_guid(),
                sizeof(skr::gui::Timer),
                alignof(skr::gui::Timer),
                make_record_basic_method_table<skr::gui::Timer>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);



        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_Timer;
    register_type_loader(RTTRTraits<skr::gui::Timer>::get_guid(), &LOADER__skr_gui_Timer);
    static struct InternalTypeLoader_skr_gui_TimerManager : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::TimerManager>::get_name(),
                RTTRTraits<::skr::gui::TimerManager>::get_guid(),
                sizeof(skr::gui::TimerManager),
                alignof(skr::gui::TimerManager),
                make_record_basic_method_table<skr::gui::TimerManager>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);



        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_TimerManager;
    register_type_loader(RTTRTraits<skr::gui::TimerManager>::get_guid(), &LOADER__skr_gui_TimerManager);
    static struct InternalTypeLoader_skr_gui_ComponentElement : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::ComponentElement>::get_name(),
                RTTRTraits<::skr::gui::ComponentElement>::get_guid(),
                sizeof(skr::gui::ComponentElement),
                alignof(skr::gui::ComponentElement),
                make_record_basic_method_table<skr::gui::ComponentElement>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::Element>::get_guid(), {RTTRTraits<skr::gui::Element>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::Element*>(reinterpret_cast<skr::gui::ComponentElement*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_ComponentElement;
    register_type_loader(RTTRTraits<skr::gui::ComponentElement>::get_guid(), &LOADER__skr_gui_ComponentElement);
    static struct InternalTypeLoader_skr_gui_Element : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::Element>::get_name(),
                RTTRTraits<::skr::gui::Element>::get_guid(),
                sizeof(skr::gui::Element),
                alignof(skr::gui::Element),
                make_record_basic_method_table<skr::gui::Element>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::rttr::IObject>::get_guid(), {RTTRTraits<skr::rttr::IObject>::get_type(), +[](void* p) -> void* { return static_cast<skr::rttr::IObject*>(reinterpret_cast<skr::gui::Element*>(p)); }}},
                {RTTRTraits<skr::gui::IBuildContext>::get_guid(), {RTTRTraits<skr::gui::IBuildContext>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::IBuildContext*>(reinterpret_cast<skr::gui::Element*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_Element;
    register_type_loader(RTTRTraits<skr::gui::Element>::get_guid(), &LOADER__skr_gui_Element);
    static struct InternalTypeLoader_skr_gui_LeafRenderObjectElement : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::LeafRenderObjectElement>::get_name(),
                RTTRTraits<::skr::gui::LeafRenderObjectElement>::get_guid(),
                sizeof(skr::gui::LeafRenderObjectElement),
                alignof(skr::gui::LeafRenderObjectElement),
                make_record_basic_method_table<skr::gui::LeafRenderObjectElement>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::RenderObjectElement>::get_guid(), {RTTRTraits<skr::gui::RenderObjectElement>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::RenderObjectElement*>(reinterpret_cast<skr::gui::LeafRenderObjectElement*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_LeafRenderObjectElement;
    register_type_loader(RTTRTraits<skr::gui::LeafRenderObjectElement>::get_guid(), &LOADER__skr_gui_LeafRenderObjectElement);
    static struct InternalTypeLoader_skr_gui_MultiChildRenderObjectElement : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::MultiChildRenderObjectElement>::get_name(),
                RTTRTraits<::skr::gui::MultiChildRenderObjectElement>::get_guid(),
                sizeof(skr::gui::MultiChildRenderObjectElement),
                alignof(skr::gui::MultiChildRenderObjectElement),
                make_record_basic_method_table<skr::gui::MultiChildRenderObjectElement>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::RenderObjectElement>::get_guid(), {RTTRTraits<skr::gui::RenderObjectElement>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::RenderObjectElement*>(reinterpret_cast<skr::gui::MultiChildRenderObjectElement*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_MultiChildRenderObjectElement;
    register_type_loader(RTTRTraits<skr::gui::MultiChildRenderObjectElement>::get_guid(), &LOADER__skr_gui_MultiChildRenderObjectElement);
    static struct InternalTypeLoader_skr_gui_ProxyElement : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::ProxyElement>::get_name(),
                RTTRTraits<::skr::gui::ProxyElement>::get_guid(),
                sizeof(skr::gui::ProxyElement),
                alignof(skr::gui::ProxyElement),
                make_record_basic_method_table<skr::gui::ProxyElement>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::ComponentElement>::get_guid(), {RTTRTraits<skr::gui::ComponentElement>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::ComponentElement*>(reinterpret_cast<skr::gui::ProxyElement*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_ProxyElement;
    register_type_loader(RTTRTraits<skr::gui::ProxyElement>::get_guid(), &LOADER__skr_gui_ProxyElement);
    static struct InternalTypeLoader_skr_gui_RenderNativeWindowElement : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::RenderNativeWindowElement>::get_name(),
                RTTRTraits<::skr::gui::RenderNativeWindowElement>::get_guid(),
                sizeof(skr::gui::RenderNativeWindowElement),
                alignof(skr::gui::RenderNativeWindowElement),
                make_record_basic_method_table<skr::gui::RenderNativeWindowElement>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::RenderWindowElement>::get_guid(), {RTTRTraits<skr::gui::RenderWindowElement>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::RenderWindowElement*>(reinterpret_cast<skr::gui::RenderNativeWindowElement*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_RenderNativeWindowElement;
    register_type_loader(RTTRTraits<skr::gui::RenderNativeWindowElement>::get_guid(), &LOADER__skr_gui_RenderNativeWindowElement);
    static struct InternalTypeLoader_skr_gui_RenderObjectElement : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::RenderObjectElement>::get_name(),
                RTTRTraits<::skr::gui::RenderObjectElement>::get_guid(),
                sizeof(skr::gui::RenderObjectElement),
                alignof(skr::gui::RenderObjectElement),
                make_record_basic_method_table<skr::gui::RenderObjectElement>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::Element>::get_guid(), {RTTRTraits<skr::gui::Element>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::Element*>(reinterpret_cast<skr::gui::RenderObjectElement*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_RenderObjectElement;
    register_type_loader(RTTRTraits<skr::gui::RenderObjectElement>::get_guid(), &LOADER__skr_gui_RenderObjectElement);
    static struct InternalTypeLoader_skr_gui_RenderWindowElement : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::RenderWindowElement>::get_name(),
                RTTRTraits<::skr::gui::RenderWindowElement>::get_guid(),
                sizeof(skr::gui::RenderWindowElement),
                alignof(skr::gui::RenderWindowElement),
                make_record_basic_method_table<skr::gui::RenderWindowElement>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::RenderObjectElement>::get_guid(), {RTTRTraits<skr::gui::RenderObjectElement>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::RenderObjectElement*>(reinterpret_cast<skr::gui::RenderWindowElement*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_RenderWindowElement;
    register_type_loader(RTTRTraits<skr::gui::RenderWindowElement>::get_guid(), &LOADER__skr_gui_RenderWindowElement);
    static struct InternalTypeLoader_skr_gui_SingleChildRenderObjectElement : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::SingleChildRenderObjectElement>::get_name(),
                RTTRTraits<::skr::gui::SingleChildRenderObjectElement>::get_guid(),
                sizeof(skr::gui::SingleChildRenderObjectElement),
                alignof(skr::gui::SingleChildRenderObjectElement),
                make_record_basic_method_table<skr::gui::SingleChildRenderObjectElement>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::RenderObjectElement>::get_guid(), {RTTRTraits<skr::gui::RenderObjectElement>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::RenderObjectElement*>(reinterpret_cast<skr::gui::SingleChildRenderObjectElement*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_SingleChildRenderObjectElement;
    register_type_loader(RTTRTraits<skr::gui::SingleChildRenderObjectElement>::get_guid(), &LOADER__skr_gui_SingleChildRenderObjectElement);
    static struct InternalTypeLoader_skr_gui_SlotElement : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::SlotElement>::get_name(),
                RTTRTraits<::skr::gui::SlotElement>::get_guid(),
                sizeof(skr::gui::SlotElement),
                alignof(skr::gui::SlotElement),
                make_record_basic_method_table<skr::gui::SlotElement>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::ProxyElement>::get_guid(), {RTTRTraits<skr::gui::ProxyElement>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::ProxyElement*>(reinterpret_cast<skr::gui::SlotElement*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_SlotElement;
    register_type_loader(RTTRTraits<skr::gui::SlotElement>::get_guid(), &LOADER__skr_gui_SlotElement);
    static struct InternalTypeLoader_skr_gui_StatefulElement : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::StatefulElement>::get_name(),
                RTTRTraits<::skr::gui::StatefulElement>::get_guid(),
                sizeof(skr::gui::StatefulElement),
                alignof(skr::gui::StatefulElement),
                make_record_basic_method_table<skr::gui::StatefulElement>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::ComponentElement>::get_guid(), {RTTRTraits<skr::gui::ComponentElement>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::ComponentElement*>(reinterpret_cast<skr::gui::StatefulElement*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_StatefulElement;
    register_type_loader(RTTRTraits<skr::gui::StatefulElement>::get_guid(), &LOADER__skr_gui_StatefulElement);
    static struct InternalTypeLoader_skr_gui_StatelessElement : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::StatelessElement>::get_name(),
                RTTRTraits<::skr::gui::StatelessElement>::get_guid(),
                sizeof(skr::gui::StatelessElement),
                alignof(skr::gui::StatelessElement),
                make_record_basic_method_table<skr::gui::StatelessElement>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::ComponentElement>::get_guid(), {RTTRTraits<skr::gui::ComponentElement>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::ComponentElement*>(reinterpret_cast<skr::gui::StatelessElement*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_StatelessElement;
    register_type_loader(RTTRTraits<skr::gui::StatelessElement>::get_guid(), &LOADER__skr_gui_StatelessElement);
    static struct InternalTypeLoader_skr_gui_ContainerLayer : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::ContainerLayer>::get_name(),
                RTTRTraits<::skr::gui::ContainerLayer>::get_guid(),
                sizeof(skr::gui::ContainerLayer),
                alignof(skr::gui::ContainerLayer),
                make_record_basic_method_table<skr::gui::ContainerLayer>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::Layer>::get_guid(), {RTTRTraits<skr::gui::Layer>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::Layer*>(reinterpret_cast<skr::gui::ContainerLayer*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_ContainerLayer;
    register_type_loader(RTTRTraits<skr::gui::ContainerLayer>::get_guid(), &LOADER__skr_gui_ContainerLayer);
    static struct InternalTypeLoader_skr_gui_GeometryLayer : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::GeometryLayer>::get_name(),
                RTTRTraits<::skr::gui::GeometryLayer>::get_guid(),
                sizeof(skr::gui::GeometryLayer),
                alignof(skr::gui::GeometryLayer),
                make_record_basic_method_table<skr::gui::GeometryLayer>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::Layer>::get_guid(), {RTTRTraits<skr::gui::Layer>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::Layer*>(reinterpret_cast<skr::gui::GeometryLayer*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_GeometryLayer;
    register_type_loader(RTTRTraits<skr::gui::GeometryLayer>::get_guid(), &LOADER__skr_gui_GeometryLayer);
    static struct InternalTypeLoader_skr_gui_Layer : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::Layer>::get_name(),
                RTTRTraits<::skr::gui::Layer>::get_guid(),
                sizeof(skr::gui::Layer),
                alignof(skr::gui::Layer),
                make_record_basic_method_table<skr::gui::Layer>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::rttr::IObject>::get_guid(), {RTTRTraits<skr::rttr::IObject>::get_type(), +[](void* p) -> void* { return static_cast<skr::rttr::IObject*>(reinterpret_cast<skr::gui::Layer*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_Layer;
    register_type_loader(RTTRTraits<skr::gui::Layer>::get_guid(), &LOADER__skr_gui_Layer);
    static struct InternalTypeLoader_skr_gui_NativeWindowLayer : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::NativeWindowLayer>::get_name(),
                RTTRTraits<::skr::gui::NativeWindowLayer>::get_guid(),
                sizeof(skr::gui::NativeWindowLayer),
                alignof(skr::gui::NativeWindowLayer),
                make_record_basic_method_table<skr::gui::NativeWindowLayer>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::WindowLayer>::get_guid(), {RTTRTraits<skr::gui::WindowLayer>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::WindowLayer*>(reinterpret_cast<skr::gui::NativeWindowLayer*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_NativeWindowLayer;
    register_type_loader(RTTRTraits<skr::gui::NativeWindowLayer>::get_guid(), &LOADER__skr_gui_NativeWindowLayer);
    static struct InternalTypeLoader_skr_gui_OffsetLayer : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::OffsetLayer>::get_name(),
                RTTRTraits<::skr::gui::OffsetLayer>::get_guid(),
                sizeof(skr::gui::OffsetLayer),
                alignof(skr::gui::OffsetLayer),
                make_record_basic_method_table<skr::gui::OffsetLayer>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::ContainerLayer>::get_guid(), {RTTRTraits<skr::gui::ContainerLayer>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::ContainerLayer*>(reinterpret_cast<skr::gui::OffsetLayer*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_OffsetLayer;
    register_type_loader(RTTRTraits<skr::gui::OffsetLayer>::get_guid(), &LOADER__skr_gui_OffsetLayer);
    static struct InternalTypeLoader_skr_gui_WindowLayer : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::WindowLayer>::get_name(),
                RTTRTraits<::skr::gui::WindowLayer>::get_guid(),
                sizeof(skr::gui::WindowLayer),
                alignof(skr::gui::WindowLayer),
                make_record_basic_method_table<skr::gui::WindowLayer>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::OffsetLayer>::get_guid(), {RTTRTraits<skr::gui::OffsetLayer>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::OffsetLayer*>(reinterpret_cast<skr::gui::WindowLayer*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_WindowLayer;
    register_type_loader(RTTRTraits<skr::gui::WindowLayer>::get_guid(), &LOADER__skr_gui_WindowLayer);
    static struct InternalTypeLoader_skr_gui_IMultiChildRenderObject : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::IMultiChildRenderObject>::get_name(),
                RTTRTraits<::skr::gui::IMultiChildRenderObject>::get_guid(),
                sizeof(skr::gui::IMultiChildRenderObject),
                alignof(skr::gui::IMultiChildRenderObject),
                make_record_basic_method_table<skr::gui::IMultiChildRenderObject>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::rttr::IObject>::get_guid(), {RTTRTraits<skr::rttr::IObject>::get_type(), +[](void* p) -> void* { return static_cast<skr::rttr::IObject*>(reinterpret_cast<skr::gui::IMultiChildRenderObject*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_IMultiChildRenderObject;
    register_type_loader(RTTRTraits<skr::gui::IMultiChildRenderObject>::get_guid(), &LOADER__skr_gui_IMultiChildRenderObject);
    static struct InternalTypeLoader_skr_gui_RenderBox : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::RenderBox>::get_name(),
                RTTRTraits<::skr::gui::RenderBox>::get_guid(),
                sizeof(skr::gui::RenderBox),
                alignof(skr::gui::RenderBox),
                make_record_basic_method_table<skr::gui::RenderBox>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::RenderObject>::get_guid(), {RTTRTraits<skr::gui::RenderObject>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::RenderObject*>(reinterpret_cast<skr::gui::RenderBox*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_RenderBox;
    register_type_loader(RTTRTraits<skr::gui::RenderBox>::get_guid(), &LOADER__skr_gui_RenderBox);
    static struct InternalTypeLoader_skr_gui_RenderNativeWindow : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::RenderNativeWindow>::get_name(),
                RTTRTraits<::skr::gui::RenderNativeWindow>::get_guid(),
                sizeof(skr::gui::RenderNativeWindow),
                alignof(skr::gui::RenderNativeWindow),
                make_record_basic_method_table<skr::gui::RenderNativeWindow>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::RenderWindow>::get_guid(), {RTTRTraits<skr::gui::RenderWindow>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::RenderWindow*>(reinterpret_cast<skr::gui::RenderNativeWindow*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_RenderNativeWindow;
    register_type_loader(RTTRTraits<skr::gui::RenderNativeWindow>::get_guid(), &LOADER__skr_gui_RenderNativeWindow);
    static struct InternalTypeLoader_skr_gui_RenderObject : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::RenderObject>::get_name(),
                RTTRTraits<::skr::gui::RenderObject>::get_guid(),
                sizeof(skr::gui::RenderObject),
                alignof(skr::gui::RenderObject),
                make_record_basic_method_table<skr::gui::RenderObject>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::rttr::IObject>::get_guid(), {RTTRTraits<skr::rttr::IObject>::get_type(), +[](void* p) -> void* { return static_cast<skr::rttr::IObject*>(reinterpret_cast<skr::gui::RenderObject*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_RenderObject;
    register_type_loader(RTTRTraits<skr::gui::RenderObject>::get_guid(), &LOADER__skr_gui_RenderObject);
    static struct InternalTypeLoader_skr_gui_RenderProxyBox : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::RenderProxyBox>::get_name(),
                RTTRTraits<::skr::gui::RenderProxyBox>::get_guid(),
                sizeof(skr::gui::RenderProxyBox),
                alignof(skr::gui::RenderProxyBox),
                make_record_basic_method_table<skr::gui::RenderProxyBox>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::RenderBox>::get_guid(), {RTTRTraits<skr::gui::RenderBox>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::RenderBox*>(reinterpret_cast<skr::gui::RenderProxyBox*>(p)); }}},
                {RTTRTraits<skr::gui::ISingleChildRenderObject>::get_guid(), {RTTRTraits<skr::gui::ISingleChildRenderObject>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::ISingleChildRenderObject*>(reinterpret_cast<skr::gui::RenderProxyBox*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_RenderProxyBox;
    register_type_loader(RTTRTraits<skr::gui::RenderProxyBox>::get_guid(), &LOADER__skr_gui_RenderProxyBox);
    static struct InternalTypeLoader_skr_gui_RenderProxyBoxWithHitTestBehavior : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::RenderProxyBoxWithHitTestBehavior>::get_name(),
                RTTRTraits<::skr::gui::RenderProxyBoxWithHitTestBehavior>::get_guid(),
                sizeof(skr::gui::RenderProxyBoxWithHitTestBehavior),
                alignof(skr::gui::RenderProxyBoxWithHitTestBehavior),
                make_record_basic_method_table<skr::gui::RenderProxyBoxWithHitTestBehavior>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::RenderProxyBox>::get_guid(), {RTTRTraits<skr::gui::RenderProxyBox>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::RenderProxyBox*>(reinterpret_cast<skr::gui::RenderProxyBoxWithHitTestBehavior*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_RenderProxyBoxWithHitTestBehavior;
    register_type_loader(RTTRTraits<skr::gui::RenderProxyBoxWithHitTestBehavior>::get_guid(), &LOADER__skr_gui_RenderProxyBoxWithHitTestBehavior);
    static struct InternalTypeLoader_skr_gui_RenderShiftedBox : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::RenderShiftedBox>::get_name(),
                RTTRTraits<::skr::gui::RenderShiftedBox>::get_guid(),
                sizeof(skr::gui::RenderShiftedBox),
                alignof(skr::gui::RenderShiftedBox),
                make_record_basic_method_table<skr::gui::RenderShiftedBox>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::RenderBox>::get_guid(), {RTTRTraits<skr::gui::RenderBox>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::RenderBox*>(reinterpret_cast<skr::gui::RenderShiftedBox*>(p)); }}},
                {RTTRTraits<skr::gui::ISingleChildRenderObject>::get_guid(), {RTTRTraits<skr::gui::ISingleChildRenderObject>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::ISingleChildRenderObject*>(reinterpret_cast<skr::gui::RenderShiftedBox*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_RenderShiftedBox;
    register_type_loader(RTTRTraits<skr::gui::RenderShiftedBox>::get_guid(), &LOADER__skr_gui_RenderShiftedBox);
    static struct InternalTypeLoader_skr_gui_RenderWindow : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::RenderWindow>::get_name(),
                RTTRTraits<::skr::gui::RenderWindow>::get_guid(),
                sizeof(skr::gui::RenderWindow),
                alignof(skr::gui::RenderWindow),
                make_record_basic_method_table<skr::gui::RenderWindow>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::RenderObject>::get_guid(), {RTTRTraits<skr::gui::RenderObject>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::RenderObject*>(reinterpret_cast<skr::gui::RenderWindow*>(p)); }}},
                {RTTRTraits<skr::gui::ISingleChildRenderObject>::get_guid(), {RTTRTraits<skr::gui::ISingleChildRenderObject>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::ISingleChildRenderObject*>(reinterpret_cast<skr::gui::RenderWindow*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_RenderWindow;
    register_type_loader(RTTRTraits<skr::gui::RenderWindow>::get_guid(), &LOADER__skr_gui_RenderWindow);
    static struct InternalTypeLoader_skr_gui_ISingleChildRenderObject : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::ISingleChildRenderObject>::get_name(),
                RTTRTraits<::skr::gui::ISingleChildRenderObject>::get_guid(),
                sizeof(skr::gui::ISingleChildRenderObject),
                alignof(skr::gui::ISingleChildRenderObject),
                make_record_basic_method_table<skr::gui::ISingleChildRenderObject>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::rttr::IObject>::get_guid(), {RTTRTraits<skr::rttr::IObject>::get_type(), +[](void* p) -> void* { return static_cast<skr::rttr::IObject*>(reinterpret_cast<skr::gui::ISingleChildRenderObject*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_ISingleChildRenderObject;
    register_type_loader(RTTRTraits<skr::gui::ISingleChildRenderObject>::get_guid(), &LOADER__skr_gui_ISingleChildRenderObject);
    static struct InternalTypeLoader_skr_gui_LeafRenderObjectWidget : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::LeafRenderObjectWidget>::get_name(),
                RTTRTraits<::skr::gui::LeafRenderObjectWidget>::get_guid(),
                sizeof(skr::gui::LeafRenderObjectWidget),
                alignof(skr::gui::LeafRenderObjectWidget),
                make_record_basic_method_table<skr::gui::LeafRenderObjectWidget>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::RenderObjectWidget>::get_guid(), {RTTRTraits<skr::gui::RenderObjectWidget>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::RenderObjectWidget*>(reinterpret_cast<skr::gui::LeafRenderObjectWidget*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_LeafRenderObjectWidget;
    register_type_loader(RTTRTraits<skr::gui::LeafRenderObjectWidget>::get_guid(), &LOADER__skr_gui_LeafRenderObjectWidget);
    static struct InternalTypeLoader_skr_gui_MultiChildRenderObjectWidget : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::MultiChildRenderObjectWidget>::get_name(),
                RTTRTraits<::skr::gui::MultiChildRenderObjectWidget>::get_guid(),
                sizeof(skr::gui::MultiChildRenderObjectWidget),
                alignof(skr::gui::MultiChildRenderObjectWidget),
                make_record_basic_method_table<skr::gui::MultiChildRenderObjectWidget>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::RenderObjectWidget>::get_guid(), {RTTRTraits<skr::gui::RenderObjectWidget>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::RenderObjectWidget*>(reinterpret_cast<skr::gui::MultiChildRenderObjectWidget*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_MultiChildRenderObjectWidget;
    register_type_loader(RTTRTraits<skr::gui::MultiChildRenderObjectWidget>::get_guid(), &LOADER__skr_gui_MultiChildRenderObjectWidget);
    static struct InternalTypeLoader_skr_gui_ProxyWidget : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::ProxyWidget>::get_name(),
                RTTRTraits<::skr::gui::ProxyWidget>::get_guid(),
                sizeof(skr::gui::ProxyWidget),
                alignof(skr::gui::ProxyWidget),
                make_record_basic_method_table<skr::gui::ProxyWidget>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::Widget>::get_guid(), {RTTRTraits<skr::gui::Widget>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::Widget*>(reinterpret_cast<skr::gui::ProxyWidget*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_ProxyWidget;
    register_type_loader(RTTRTraits<skr::gui::ProxyWidget>::get_guid(), &LOADER__skr_gui_ProxyWidget);
    static struct InternalTypeLoader_skr_gui_RenderNativeWindowWidget : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::RenderNativeWindowWidget>::get_name(),
                RTTRTraits<::skr::gui::RenderNativeWindowWidget>::get_guid(),
                sizeof(skr::gui::RenderNativeWindowWidget),
                alignof(skr::gui::RenderNativeWindowWidget),
                make_record_basic_method_table<skr::gui::RenderNativeWindowWidget>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::RenderWindowWidget>::get_guid(), {RTTRTraits<skr::gui::RenderWindowWidget>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::RenderWindowWidget*>(reinterpret_cast<skr::gui::RenderNativeWindowWidget*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_RenderNativeWindowWidget;
    register_type_loader(RTTRTraits<skr::gui::RenderNativeWindowWidget>::get_guid(), &LOADER__skr_gui_RenderNativeWindowWidget);
    static struct InternalTypeLoader_skr_gui_RenderObjectWidget : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::RenderObjectWidget>::get_name(),
                RTTRTraits<::skr::gui::RenderObjectWidget>::get_guid(),
                sizeof(skr::gui::RenderObjectWidget),
                alignof(skr::gui::RenderObjectWidget),
                make_record_basic_method_table<skr::gui::RenderObjectWidget>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::Widget>::get_guid(), {RTTRTraits<skr::gui::Widget>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::Widget*>(reinterpret_cast<skr::gui::RenderObjectWidget*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_RenderObjectWidget;
    register_type_loader(RTTRTraits<skr::gui::RenderObjectWidget>::get_guid(), &LOADER__skr_gui_RenderObjectWidget);
    static struct InternalTypeLoader_skr_gui_RenderWindowWidget : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::RenderWindowWidget>::get_name(),
                RTTRTraits<::skr::gui::RenderWindowWidget>::get_guid(),
                sizeof(skr::gui::RenderWindowWidget),
                alignof(skr::gui::RenderWindowWidget),
                make_record_basic_method_table<skr::gui::RenderWindowWidget>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::RenderObjectWidget>::get_guid(), {RTTRTraits<skr::gui::RenderObjectWidget>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::RenderObjectWidget*>(reinterpret_cast<skr::gui::RenderWindowWidget*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_RenderWindowWidget;
    register_type_loader(RTTRTraits<skr::gui::RenderWindowWidget>::get_guid(), &LOADER__skr_gui_RenderWindowWidget);
    static struct InternalTypeLoader_skr_gui_SingleChildRenderObjectWidget : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::SingleChildRenderObjectWidget>::get_name(),
                RTTRTraits<::skr::gui::SingleChildRenderObjectWidget>::get_guid(),
                sizeof(skr::gui::SingleChildRenderObjectWidget),
                alignof(skr::gui::SingleChildRenderObjectWidget),
                make_record_basic_method_table<skr::gui::SingleChildRenderObjectWidget>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::RenderObjectWidget>::get_guid(), {RTTRTraits<skr::gui::RenderObjectWidget>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::RenderObjectWidget*>(reinterpret_cast<skr::gui::SingleChildRenderObjectWidget*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_SingleChildRenderObjectWidget;
    register_type_loader(RTTRTraits<skr::gui::SingleChildRenderObjectWidget>::get_guid(), &LOADER__skr_gui_SingleChildRenderObjectWidget);
    static struct InternalTypeLoader_skr_gui_SlotWidget : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::SlotWidget>::get_name(),
                RTTRTraits<::skr::gui::SlotWidget>::get_guid(),
                sizeof(skr::gui::SlotWidget),
                alignof(skr::gui::SlotWidget),
                make_record_basic_method_table<skr::gui::SlotWidget>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::ProxyWidget>::get_guid(), {RTTRTraits<skr::gui::ProxyWidget>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::ProxyWidget*>(reinterpret_cast<skr::gui::SlotWidget*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_SlotWidget;
    register_type_loader(RTTRTraits<skr::gui::SlotWidget>::get_guid(), &LOADER__skr_gui_SlotWidget);
    static struct InternalTypeLoader_skr_gui_State : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::State>::get_name(),
                RTTRTraits<::skr::gui::State>::get_guid(),
                sizeof(skr::gui::State),
                alignof(skr::gui::State),
                make_record_basic_method_table<skr::gui::State>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::rttr::IObject>::get_guid(), {RTTRTraits<skr::rttr::IObject>::get_type(), +[](void* p) -> void* { return static_cast<skr::rttr::IObject*>(reinterpret_cast<skr::gui::State*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_State;
    register_type_loader(RTTRTraits<skr::gui::State>::get_guid(), &LOADER__skr_gui_State);
    static struct InternalTypeLoader_skr_gui_StatefulWidget : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::StatefulWidget>::get_name(),
                RTTRTraits<::skr::gui::StatefulWidget>::get_guid(),
                sizeof(skr::gui::StatefulWidget),
                alignof(skr::gui::StatefulWidget),
                make_record_basic_method_table<skr::gui::StatefulWidget>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::Widget>::get_guid(), {RTTRTraits<skr::gui::Widget>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::Widget*>(reinterpret_cast<skr::gui::StatefulWidget*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_StatefulWidget;
    register_type_loader(RTTRTraits<skr::gui::StatefulWidget>::get_guid(), &LOADER__skr_gui_StatefulWidget);
    static struct InternalTypeLoader_skr_gui_StatelessWidget : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::StatelessWidget>::get_name(),
                RTTRTraits<::skr::gui::StatelessWidget>::get_guid(),
                sizeof(skr::gui::StatelessWidget),
                alignof(skr::gui::StatelessWidget),
                make_record_basic_method_table<skr::gui::StatelessWidget>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::Widget>::get_guid(), {RTTRTraits<skr::gui::Widget>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::Widget*>(reinterpret_cast<skr::gui::StatelessWidget*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_StatelessWidget;
    register_type_loader(RTTRTraits<skr::gui::StatelessWidget>::get_guid(), &LOADER__skr_gui_StatelessWidget);
    static struct InternalTypeLoader_skr_gui_Widget : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::Widget>::get_name(),
                RTTRTraits<::skr::gui::Widget>::get_guid(),
                sizeof(skr::gui::Widget),
                alignof(skr::gui::Widget),
                make_record_basic_method_table<skr::gui::Widget>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::rttr::IObject>::get_guid(), {RTTRTraits<skr::rttr::IObject>::get_type(), +[](void* p) -> void* { return static_cast<skr::rttr::IObject*>(reinterpret_cast<skr::gui::Widget*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_Widget;
    register_type_loader(RTTRTraits<skr::gui::Widget>::get_guid(), &LOADER__skr_gui_Widget);
    static struct InternalTypeLoader_skr_gui_RenderColoredBox : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::RenderColoredBox>::get_name(),
                RTTRTraits<::skr::gui::RenderColoredBox>::get_guid(),
                sizeof(skr::gui::RenderColoredBox),
                alignof(skr::gui::RenderColoredBox),
                make_record_basic_method_table<skr::gui::RenderColoredBox>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::RenderProxyBoxWithHitTestBehavior>::get_guid(), {RTTRTraits<skr::gui::RenderProxyBoxWithHitTestBehavior>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::RenderProxyBoxWithHitTestBehavior*>(reinterpret_cast<skr::gui::RenderColoredBox*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_RenderColoredBox;
    register_type_loader(RTTRTraits<skr::gui::RenderColoredBox>::get_guid(), &LOADER__skr_gui_RenderColoredBox);
    static struct InternalTypeLoader_skr_gui_RenderColorPicker : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::RenderColorPicker>::get_name(),
                RTTRTraits<::skr::gui::RenderColorPicker>::get_guid(),
                sizeof(skr::gui::RenderColorPicker),
                alignof(skr::gui::RenderColorPicker),
                make_record_basic_method_table<skr::gui::RenderColorPicker>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::RenderBox>::get_guid(), {RTTRTraits<skr::gui::RenderBox>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::RenderBox*>(reinterpret_cast<skr::gui::RenderColorPicker*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_RenderColorPicker;
    register_type_loader(RTTRTraits<skr::gui::RenderColorPicker>::get_guid(), &LOADER__skr_gui_RenderColorPicker);
    static struct InternalTypeLoader_skr_gui_RenderConstrainedBox : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::RenderConstrainedBox>::get_name(),
                RTTRTraits<::skr::gui::RenderConstrainedBox>::get_guid(),
                sizeof(skr::gui::RenderConstrainedBox),
                alignof(skr::gui::RenderConstrainedBox),
                make_record_basic_method_table<skr::gui::RenderConstrainedBox>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::RenderProxyBox>::get_guid(), {RTTRTraits<skr::gui::RenderProxyBox>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::RenderProxyBox*>(reinterpret_cast<skr::gui::RenderConstrainedBox*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_RenderConstrainedBox;
    register_type_loader(RTTRTraits<skr::gui::RenderConstrainedBox>::get_guid(), &LOADER__skr_gui_RenderConstrainedBox);
    static struct InternalTypeLoader_skr_gui_RenderFlex : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::RenderFlex>::get_name(),
                RTTRTraits<::skr::gui::RenderFlex>::get_guid(),
                sizeof(skr::gui::RenderFlex),
                alignof(skr::gui::RenderFlex),
                make_record_basic_method_table<skr::gui::RenderFlex>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::RenderBox>::get_guid(), {RTTRTraits<skr::gui::RenderBox>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::RenderBox*>(reinterpret_cast<skr::gui::RenderFlex*>(p)); }}},
                {RTTRTraits<skr::gui::IMultiChildRenderObject>::get_guid(), {RTTRTraits<skr::gui::IMultiChildRenderObject>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::IMultiChildRenderObject*>(reinterpret_cast<skr::gui::RenderFlex*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_RenderFlex;
    register_type_loader(RTTRTraits<skr::gui::RenderFlex>::get_guid(), &LOADER__skr_gui_RenderFlex);
    static struct InternalTypeLoader_skr_gui_RenderGridPaper : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::RenderGridPaper>::get_name(),
                RTTRTraits<::skr::gui::RenderGridPaper>::get_guid(),
                sizeof(skr::gui::RenderGridPaper),
                alignof(skr::gui::RenderGridPaper),
                make_record_basic_method_table<skr::gui::RenderGridPaper>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::RenderBox>::get_guid(), {RTTRTraits<skr::gui::RenderBox>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::RenderBox*>(reinterpret_cast<skr::gui::RenderGridPaper*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_RenderGridPaper;
    register_type_loader(RTTRTraits<skr::gui::RenderGridPaper>::get_guid(), &LOADER__skr_gui_RenderGridPaper);
    static struct InternalTypeLoader_skr_gui_RenderMouseRegion : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::RenderMouseRegion>::get_name(),
                RTTRTraits<::skr::gui::RenderMouseRegion>::get_guid(),
                sizeof(skr::gui::RenderMouseRegion),
                alignof(skr::gui::RenderMouseRegion),
                make_record_basic_method_table<skr::gui::RenderMouseRegion>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::RenderProxyBoxWithHitTestBehavior>::get_guid(), {RTTRTraits<skr::gui::RenderProxyBoxWithHitTestBehavior>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::RenderProxyBoxWithHitTestBehavior*>(reinterpret_cast<skr::gui::RenderMouseRegion*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_RenderMouseRegion;
    register_type_loader(RTTRTraits<skr::gui::RenderMouseRegion>::get_guid(), &LOADER__skr_gui_RenderMouseRegion);
    static struct InternalTypeLoader_skr_gui_RenderPositioned : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::RenderPositioned>::get_name(),
                RTTRTraits<::skr::gui::RenderPositioned>::get_guid(),
                sizeof(skr::gui::RenderPositioned),
                alignof(skr::gui::RenderPositioned),
                make_record_basic_method_table<skr::gui::RenderPositioned>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::RenderShiftedBox>::get_guid(), {RTTRTraits<skr::gui::RenderShiftedBox>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::RenderShiftedBox*>(reinterpret_cast<skr::gui::RenderPositioned*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_RenderPositioned;
    register_type_loader(RTTRTraits<skr::gui::RenderPositioned>::get_guid(), &LOADER__skr_gui_RenderPositioned);
    static struct InternalTypeLoader_skr_gui_RenderStack : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::RenderStack>::get_name(),
                RTTRTraits<::skr::gui::RenderStack>::get_guid(),
                sizeof(skr::gui::RenderStack),
                alignof(skr::gui::RenderStack),
                make_record_basic_method_table<skr::gui::RenderStack>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::RenderBox>::get_guid(), {RTTRTraits<skr::gui::RenderBox>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::RenderBox*>(reinterpret_cast<skr::gui::RenderStack*>(p)); }}},
                {RTTRTraits<skr::gui::IMultiChildRenderObject>::get_guid(), {RTTRTraits<skr::gui::IMultiChildRenderObject>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::IMultiChildRenderObject*>(reinterpret_cast<skr::gui::RenderStack*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_RenderStack;
    register_type_loader(RTTRTraits<skr::gui::RenderStack>::get_guid(), &LOADER__skr_gui_RenderStack);
    static struct InternalTypeLoader_skr_gui_RenderText : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::RenderText>::get_name(),
                RTTRTraits<::skr::gui::RenderText>::get_guid(),
                sizeof(skr::gui::RenderText),
                alignof(skr::gui::RenderText),
                make_record_basic_method_table<skr::gui::RenderText>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::RenderBox>::get_guid(), {RTTRTraits<skr::gui::RenderBox>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::RenderBox*>(reinterpret_cast<skr::gui::RenderText*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_RenderText;
    register_type_loader(RTTRTraits<skr::gui::RenderText>::get_guid(), &LOADER__skr_gui_RenderText);
    static struct InternalTypeLoader_skr_gui_Event : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::Event>::get_name(),
                RTTRTraits<::skr::gui::Event>::get_guid(),
                sizeof(skr::gui::Event),
                alignof(skr::gui::Event),
                make_record_basic_method_table<skr::gui::Event>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::rttr::IObject>::get_guid(), {RTTRTraits<skr::rttr::IObject>::get_type(), +[](void* p) -> void* { return static_cast<skr::rttr::IObject*>(reinterpret_cast<skr::gui::Event*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_Event;
    register_type_loader(RTTRTraits<skr::gui::Event>::get_guid(), &LOADER__skr_gui_Event);
    static struct InternalTypeLoader_skr_gui_InputManager : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::InputManager>::get_name(),
                RTTRTraits<::skr::gui::InputManager>::get_guid(),
                sizeof(skr::gui::InputManager),
                alignof(skr::gui::InputManager),
                make_record_basic_method_table<skr::gui::InputManager>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);



        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_InputManager;
    register_type_loader(RTTRTraits<skr::gui::InputManager>::get_guid(), &LOADER__skr_gui_InputManager);
    static struct InternalTypeLoader_skr_gui_PointerEvent : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::PointerEvent>::get_name(),
                RTTRTraits<::skr::gui::PointerEvent>::get_guid(),
                sizeof(skr::gui::PointerEvent),
                alignof(skr::gui::PointerEvent),
                make_record_basic_method_table<skr::gui::PointerEvent>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::Event>::get_guid(), {RTTRTraits<skr::gui::Event>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::Event*>(reinterpret_cast<skr::gui::PointerEvent*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_PointerEvent;
    register_type_loader(RTTRTraits<skr::gui::PointerEvent>::get_guid(), &LOADER__skr_gui_PointerEvent);
    static struct InternalTypeLoader_skr_gui_PointerDownEvent : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::PointerDownEvent>::get_name(),
                RTTRTraits<::skr::gui::PointerDownEvent>::get_guid(),
                sizeof(skr::gui::PointerDownEvent),
                alignof(skr::gui::PointerDownEvent),
                make_record_basic_method_table<skr::gui::PointerDownEvent>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::PointerEvent>::get_guid(), {RTTRTraits<skr::gui::PointerEvent>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::PointerEvent*>(reinterpret_cast<skr::gui::PointerDownEvent*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_PointerDownEvent;
    register_type_loader(RTTRTraits<skr::gui::PointerDownEvent>::get_guid(), &LOADER__skr_gui_PointerDownEvent);
    static struct InternalTypeLoader_skr_gui_PointerUpEvent : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::PointerUpEvent>::get_name(),
                RTTRTraits<::skr::gui::PointerUpEvent>::get_guid(),
                sizeof(skr::gui::PointerUpEvent),
                alignof(skr::gui::PointerUpEvent),
                make_record_basic_method_table<skr::gui::PointerUpEvent>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::PointerEvent>::get_guid(), {RTTRTraits<skr::gui::PointerEvent>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::PointerEvent*>(reinterpret_cast<skr::gui::PointerUpEvent*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_PointerUpEvent;
    register_type_loader(RTTRTraits<skr::gui::PointerUpEvent>::get_guid(), &LOADER__skr_gui_PointerUpEvent);
    static struct InternalTypeLoader_skr_gui_PointerMoveEvent : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::PointerMoveEvent>::get_name(),
                RTTRTraits<::skr::gui::PointerMoveEvent>::get_guid(),
                sizeof(skr::gui::PointerMoveEvent),
                alignof(skr::gui::PointerMoveEvent),
                make_record_basic_method_table<skr::gui::PointerMoveEvent>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::PointerEvent>::get_guid(), {RTTRTraits<skr::gui::PointerEvent>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::PointerEvent*>(reinterpret_cast<skr::gui::PointerMoveEvent*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_PointerMoveEvent;
    register_type_loader(RTTRTraits<skr::gui::PointerMoveEvent>::get_guid(), &LOADER__skr_gui_PointerMoveEvent);
    static struct InternalTypeLoader_skr_gui_PointerEnterEvent : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::PointerEnterEvent>::get_name(),
                RTTRTraits<::skr::gui::PointerEnterEvent>::get_guid(),
                sizeof(skr::gui::PointerEnterEvent),
                alignof(skr::gui::PointerEnterEvent),
                make_record_basic_method_table<skr::gui::PointerEnterEvent>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::PointerEvent>::get_guid(), {RTTRTraits<skr::gui::PointerEvent>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::PointerEvent*>(reinterpret_cast<skr::gui::PointerEnterEvent*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_PointerEnterEvent;
    register_type_loader(RTTRTraits<skr::gui::PointerEnterEvent>::get_guid(), &LOADER__skr_gui_PointerEnterEvent);
    static struct InternalTypeLoader_skr_gui_PointerExitEvent : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::PointerExitEvent>::get_name(),
                RTTRTraits<::skr::gui::PointerExitEvent>::get_guid(),
                sizeof(skr::gui::PointerExitEvent),
                alignof(skr::gui::PointerExitEvent),
                make_record_basic_method_table<skr::gui::PointerExitEvent>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::PointerEvent>::get_guid(), {RTTRTraits<skr::gui::PointerEvent>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::PointerEvent*>(reinterpret_cast<skr::gui::PointerExitEvent*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_PointerExitEvent;
    register_type_loader(RTTRTraits<skr::gui::PointerExitEvent>::get_guid(), &LOADER__skr_gui_PointerExitEvent);
    static struct InternalTypeLoader_skr_gui_PointerScrollEvent : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::PointerScrollEvent>::get_name(),
                RTTRTraits<::skr::gui::PointerScrollEvent>::get_guid(),
                sizeof(skr::gui::PointerScrollEvent),
                alignof(skr::gui::PointerScrollEvent),
                make_record_basic_method_table<skr::gui::PointerScrollEvent>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::PointerEvent>::get_guid(), {RTTRTraits<skr::gui::PointerEvent>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::PointerEvent*>(reinterpret_cast<skr::gui::PointerScrollEvent*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_PointerScrollEvent;
    register_type_loader(RTTRTraits<skr::gui::PointerScrollEvent>::get_guid(), &LOADER__skr_gui_PointerScrollEvent);
    static struct InternalTypeLoader_skr_gui_PointerScaleEvent : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::PointerScaleEvent>::get_name(),
                RTTRTraits<::skr::gui::PointerScaleEvent>::get_guid(),
                sizeof(skr::gui::PointerScaleEvent),
                alignof(skr::gui::PointerScaleEvent),
                make_record_basic_method_table<skr::gui::PointerScaleEvent>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::PointerEvent>::get_guid(), {RTTRTraits<skr::gui::PointerEvent>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::PointerEvent*>(reinterpret_cast<skr::gui::PointerScaleEvent*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_PointerScaleEvent;
    register_type_loader(RTTRTraits<skr::gui::PointerScaleEvent>::get_guid(), &LOADER__skr_gui_PointerScaleEvent);
    static struct InternalTypeLoader_skr_gui_PointerPanZoomStartEvent : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::PointerPanZoomStartEvent>::get_name(),
                RTTRTraits<::skr::gui::PointerPanZoomStartEvent>::get_guid(),
                sizeof(skr::gui::PointerPanZoomStartEvent),
                alignof(skr::gui::PointerPanZoomStartEvent),
                make_record_basic_method_table<skr::gui::PointerPanZoomStartEvent>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::PointerEvent>::get_guid(), {RTTRTraits<skr::gui::PointerEvent>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::PointerEvent*>(reinterpret_cast<skr::gui::PointerPanZoomStartEvent*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_PointerPanZoomStartEvent;
    register_type_loader(RTTRTraits<skr::gui::PointerPanZoomStartEvent>::get_guid(), &LOADER__skr_gui_PointerPanZoomStartEvent);
    static struct InternalTypeLoader_skr_gui_PointerPanZoomUpdateEvent : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::PointerPanZoomUpdateEvent>::get_name(),
                RTTRTraits<::skr::gui::PointerPanZoomUpdateEvent>::get_guid(),
                sizeof(skr::gui::PointerPanZoomUpdateEvent),
                alignof(skr::gui::PointerPanZoomUpdateEvent),
                make_record_basic_method_table<skr::gui::PointerPanZoomUpdateEvent>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::PointerEvent>::get_guid(), {RTTRTraits<skr::gui::PointerEvent>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::PointerEvent*>(reinterpret_cast<skr::gui::PointerPanZoomUpdateEvent*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_PointerPanZoomUpdateEvent;
    register_type_loader(RTTRTraits<skr::gui::PointerPanZoomUpdateEvent>::get_guid(), &LOADER__skr_gui_PointerPanZoomUpdateEvent);
    static struct InternalTypeLoader_skr_gui_PointerPanZoomEndEvent : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::PointerPanZoomEndEvent>::get_name(),
                RTTRTraits<::skr::gui::PointerPanZoomEndEvent>::get_guid(),
                sizeof(skr::gui::PointerPanZoomEndEvent),
                alignof(skr::gui::PointerPanZoomEndEvent),
                make_record_basic_method_table<skr::gui::PointerPanZoomEndEvent>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::PointerEvent>::get_guid(), {RTTRTraits<skr::gui::PointerEvent>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::PointerEvent*>(reinterpret_cast<skr::gui::PointerPanZoomEndEvent*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_PointerPanZoomEndEvent;
    register_type_loader(RTTRTraits<skr::gui::PointerPanZoomEndEvent>::get_guid(), &LOADER__skr_gui_PointerPanZoomEndEvent);
    static struct InternalTypeLoader_skr_gui_ClickGestureRecognizer : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::ClickGestureRecognizer>::get_name(),
                RTTRTraits<::skr::gui::ClickGestureRecognizer>::get_guid(),
                sizeof(skr::gui::ClickGestureRecognizer),
                alignof(skr::gui::ClickGestureRecognizer),
                make_record_basic_method_table<skr::gui::ClickGestureRecognizer>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::PointerGestureRecognizer>::get_guid(), {RTTRTraits<skr::gui::PointerGestureRecognizer>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::PointerGestureRecognizer*>(reinterpret_cast<skr::gui::ClickGestureRecognizer*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_ClickGestureRecognizer;
    register_type_loader(RTTRTraits<skr::gui::ClickGestureRecognizer>::get_guid(), &LOADER__skr_gui_ClickGestureRecognizer);
    static struct InternalTypeLoader_skr_gui_GestureArena : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::GestureArena>::get_name(),
                RTTRTraits<::skr::gui::GestureArena>::get_guid(),
                sizeof(skr::gui::GestureArena),
                alignof(skr::gui::GestureArena),
                make_record_basic_method_table<skr::gui::GestureArena>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);



        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_GestureArena;
    register_type_loader(RTTRTraits<skr::gui::GestureArena>::get_guid(), &LOADER__skr_gui_GestureArena);
    static struct InternalTypeLoader_skr_gui_GestureArenaManager : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::GestureArenaManager>::get_name(),
                RTTRTraits<::skr::gui::GestureArenaManager>::get_guid(),
                sizeof(skr::gui::GestureArenaManager),
                alignof(skr::gui::GestureArenaManager),
                make_record_basic_method_table<skr::gui::GestureArenaManager>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);



        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_GestureArenaManager;
    register_type_loader(RTTRTraits<skr::gui::GestureArenaManager>::get_guid(), &LOADER__skr_gui_GestureArenaManager);
    static struct InternalTypeLoader_skr_gui_RawGestureDetector : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::RawGestureDetector>::get_name(),
                RTTRTraits<::skr::gui::RawGestureDetector>::get_guid(),
                sizeof(skr::gui::RawGestureDetector),
                alignof(skr::gui::RawGestureDetector),
                make_record_basic_method_table<skr::gui::RawGestureDetector>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::StatefulWidget>::get_guid(), {RTTRTraits<skr::gui::StatefulWidget>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::StatefulWidget*>(reinterpret_cast<skr::gui::RawGestureDetector*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_RawGestureDetector;
    register_type_loader(RTTRTraits<skr::gui::RawGestureDetector>::get_guid(), &LOADER__skr_gui_RawGestureDetector);
    static struct InternalTypeLoader_skr_gui_GestureRecognizer : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::GestureRecognizer>::get_name(),
                RTTRTraits<::skr::gui::GestureRecognizer>::get_guid(),
                sizeof(skr::gui::GestureRecognizer),
                alignof(skr::gui::GestureRecognizer),
                make_record_basic_method_table<skr::gui::GestureRecognizer>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::rttr::IObject>::get_guid(), {RTTRTraits<skr::rttr::IObject>::get_type(), +[](void* p) -> void* { return static_cast<skr::rttr::IObject*>(reinterpret_cast<skr::gui::GestureRecognizer*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_GestureRecognizer;
    register_type_loader(RTTRTraits<skr::gui::GestureRecognizer>::get_guid(), &LOADER__skr_gui_GestureRecognizer);
    static struct InternalTypeLoader_skr_gui_PointerGestureRecognizer : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::PointerGestureRecognizer>::get_name(),
                RTTRTraits<::skr::gui::PointerGestureRecognizer>::get_guid(),
                sizeof(skr::gui::PointerGestureRecognizer),
                alignof(skr::gui::PointerGestureRecognizer),
                make_record_basic_method_table<skr::gui::PointerGestureRecognizer>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::GestureRecognizer>::get_guid(), {RTTRTraits<skr::gui::GestureRecognizer>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::GestureRecognizer*>(reinterpret_cast<skr::gui::PointerGestureRecognizer*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_PointerGestureRecognizer;
    register_type_loader(RTTRTraits<skr::gui::PointerGestureRecognizer>::get_guid(), &LOADER__skr_gui_PointerGestureRecognizer);
    static struct InternalTypeLoader_skr_gui_ColoredBox : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::ColoredBox>::get_name(),
                RTTRTraits<::skr::gui::ColoredBox>::get_guid(),
                sizeof(skr::gui::ColoredBox),
                alignof(skr::gui::ColoredBox),
                make_record_basic_method_table<skr::gui::ColoredBox>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::SingleChildRenderObjectWidget>::get_guid(), {RTTRTraits<skr::gui::SingleChildRenderObjectWidget>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::SingleChildRenderObjectWidget*>(reinterpret_cast<skr::gui::ColoredBox*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_ColoredBox;
    register_type_loader(RTTRTraits<skr::gui::ColoredBox>::get_guid(), &LOADER__skr_gui_ColoredBox);
    static struct InternalTypeLoader_skr_gui_ColorPicker : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::ColorPicker>::get_name(),
                RTTRTraits<::skr::gui::ColorPicker>::get_guid(),
                sizeof(skr::gui::ColorPicker),
                alignof(skr::gui::ColorPicker),
                make_record_basic_method_table<skr::gui::ColorPicker>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::LeafRenderObjectWidget>::get_guid(), {RTTRTraits<skr::gui::LeafRenderObjectWidget>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::LeafRenderObjectWidget*>(reinterpret_cast<skr::gui::ColorPicker*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_ColorPicker;
    register_type_loader(RTTRTraits<skr::gui::ColorPicker>::get_guid(), &LOADER__skr_gui_ColorPicker);
    static struct InternalTypeLoader_skr_gui_Flex : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::Flex>::get_name(),
                RTTRTraits<::skr::gui::Flex>::get_guid(),
                sizeof(skr::gui::Flex),
                alignof(skr::gui::Flex),
                make_record_basic_method_table<skr::gui::Flex>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::MultiChildRenderObjectWidget>::get_guid(), {RTTRTraits<skr::gui::MultiChildRenderObjectWidget>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::MultiChildRenderObjectWidget*>(reinterpret_cast<skr::gui::Flex*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_Flex;
    register_type_loader(RTTRTraits<skr::gui::Flex>::get_guid(), &LOADER__skr_gui_Flex);
    static struct InternalTypeLoader_skr_gui_FlexSlot : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::FlexSlot>::get_name(),
                RTTRTraits<::skr::gui::FlexSlot>::get_guid(),
                sizeof(skr::gui::FlexSlot),
                alignof(skr::gui::FlexSlot),
                make_record_basic_method_table<skr::gui::FlexSlot>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::SlotWidget>::get_guid(), {RTTRTraits<skr::gui::SlotWidget>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::SlotWidget*>(reinterpret_cast<skr::gui::FlexSlot*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_FlexSlot;
    register_type_loader(RTTRTraits<skr::gui::FlexSlot>::get_guid(), &LOADER__skr_gui_FlexSlot);
    static struct InternalTypeLoader_skr_gui_GridPaper : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::GridPaper>::get_name(),
                RTTRTraits<::skr::gui::GridPaper>::get_guid(),
                sizeof(skr::gui::GridPaper),
                alignof(skr::gui::GridPaper),
                make_record_basic_method_table<skr::gui::GridPaper>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::LeafRenderObjectWidget>::get_guid(), {RTTRTraits<skr::gui::LeafRenderObjectWidget>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::LeafRenderObjectWidget*>(reinterpret_cast<skr::gui::GridPaper*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_GridPaper;
    register_type_loader(RTTRTraits<skr::gui::GridPaper>::get_guid(), &LOADER__skr_gui_GridPaper);
    static struct InternalTypeLoader_skr_gui_MouseRegin : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::MouseRegin>::get_name(),
                RTTRTraits<::skr::gui::MouseRegin>::get_guid(),
                sizeof(skr::gui::MouseRegin),
                alignof(skr::gui::MouseRegin),
                make_record_basic_method_table<skr::gui::MouseRegin>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::SingleChildRenderObjectWidget>::get_guid(), {RTTRTraits<skr::gui::SingleChildRenderObjectWidget>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::SingleChildRenderObjectWidget*>(reinterpret_cast<skr::gui::MouseRegin*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_MouseRegin;
    register_type_loader(RTTRTraits<skr::gui::MouseRegin>::get_guid(), &LOADER__skr_gui_MouseRegin);
    static struct InternalTypeLoader_skr_gui_Positioned : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::Positioned>::get_name(),
                RTTRTraits<::skr::gui::Positioned>::get_guid(),
                sizeof(skr::gui::Positioned),
                alignof(skr::gui::Positioned),
                make_record_basic_method_table<skr::gui::Positioned>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::SingleChildRenderObjectWidget>::get_guid(), {RTTRTraits<skr::gui::SingleChildRenderObjectWidget>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::SingleChildRenderObjectWidget*>(reinterpret_cast<skr::gui::Positioned*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_Positioned;
    register_type_loader(RTTRTraits<skr::gui::Positioned>::get_guid(), &LOADER__skr_gui_Positioned);
    static struct InternalTypeLoader_skr_gui_SizedBox : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::SizedBox>::get_name(),
                RTTRTraits<::skr::gui::SizedBox>::get_guid(),
                sizeof(skr::gui::SizedBox),
                alignof(skr::gui::SizedBox),
                make_record_basic_method_table<skr::gui::SizedBox>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::SingleChildRenderObjectWidget>::get_guid(), {RTTRTraits<skr::gui::SingleChildRenderObjectWidget>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::SingleChildRenderObjectWidget*>(reinterpret_cast<skr::gui::SizedBox*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_SizedBox;
    register_type_loader(RTTRTraits<skr::gui::SizedBox>::get_guid(), &LOADER__skr_gui_SizedBox);
    static struct InternalTypeLoader_skr_gui_Stack : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::Stack>::get_name(),
                RTTRTraits<::skr::gui::Stack>::get_guid(),
                sizeof(skr::gui::Stack),
                alignof(skr::gui::Stack),
                make_record_basic_method_table<skr::gui::Stack>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::MultiChildRenderObjectWidget>::get_guid(), {RTTRTraits<skr::gui::MultiChildRenderObjectWidget>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::MultiChildRenderObjectWidget*>(reinterpret_cast<skr::gui::Stack*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_Stack;
    register_type_loader(RTTRTraits<skr::gui::Stack>::get_guid(), &LOADER__skr_gui_Stack);
    static struct InternalTypeLoader_skr_gui_Text : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::Text>::get_name(),
                RTTRTraits<::skr::gui::Text>::get_guid(),
                sizeof(skr::gui::Text),
                alignof(skr::gui::Text),
                make_record_basic_method_table<skr::gui::Text>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<skr::gui::LeafRenderObjectWidget>::get_guid(), {RTTRTraits<skr::gui::LeafRenderObjectWidget>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::LeafRenderObjectWidget*>(reinterpret_cast<skr::gui::Text*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui_Text;
    register_type_loader(RTTRTraits<skr::gui::Text>::get_guid(), &LOADER__skr_gui_Text);
    static struct InternalTypeLoader_skr_gui__EmbeddedParagraph : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::skr::gui::_EmbeddedParagraph>::get_name(),
                RTTRTraits<::skr::gui::_EmbeddedParagraph>::get_guid(),
                sizeof(skr::gui::_EmbeddedParagraph),
                alignof(skr::gui::_EmbeddedParagraph),
                make_record_basic_method_table<skr::gui::_EmbeddedParagraph>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

            result->set_base_types({
                {RTTRTraits<godot::TextParagraph>::get_guid(), {RTTRTraits<godot::TextParagraph>::get_type(), +[](void* p) -> void* { return static_cast<godot::TextParagraph*>(reinterpret_cast<skr::gui::_EmbeddedParagraph*>(p)); }}},
                {RTTRTraits<skr::gui::IParagraph>::get_guid(), {RTTRTraits<skr::gui::IParagraph>::get_type(), +[](void* p) -> void* { return static_cast<skr::gui::IParagraph*>(reinterpret_cast<skr::gui::_EmbeddedParagraph*>(p)); }}},
            });


        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__skr_gui__EmbeddedParagraph;
    register_type_loader(RTTRTraits<skr::gui::_EmbeddedParagraph>::get_guid(), &LOADER__skr_gui__EmbeddedParagraph);

    static EnumTypeFromTraitsLoader<skr::gui::EFlexDirection> LOADER__skr_gui_EFlexDirection;
    register_type_loader(RTTRTraits<skr::gui::EFlexDirection>::get_guid(), &LOADER__skr_gui_EFlexDirection);
    static EnumTypeFromTraitsLoader<skr::gui::EMainAxisAlignment> LOADER__skr_gui_EMainAxisAlignment;
    register_type_loader(RTTRTraits<skr::gui::EMainAxisAlignment>::get_guid(), &LOADER__skr_gui_EMainAxisAlignment);
    static EnumTypeFromTraitsLoader<skr::gui::ECrossAxisAlignment> LOADER__skr_gui_ECrossAxisAlignment;
    register_type_loader(RTTRTraits<skr::gui::ECrossAxisAlignment>::get_guid(), &LOADER__skr_gui_ECrossAxisAlignment);
    static EnumTypeFromTraitsLoader<skr::gui::EMainAxisSize> LOADER__skr_gui_EMainAxisSize;
    register_type_loader(RTTRTraits<skr::gui::EMainAxisSize>::get_guid(), &LOADER__skr_gui_EMainAxisSize);
    static EnumTypeFromTraitsLoader<skr::gui::EFlexFit> LOADER__skr_gui_EFlexFit;
    register_type_loader(RTTRTraits<skr::gui::EFlexFit>::get_guid(), &LOADER__skr_gui_EFlexFit);
    static EnumTypeFromTraitsLoader<skr::gui::EPositionalFit> LOADER__skr_gui_EPositionalFit;
    register_type_loader(RTTRTraits<skr::gui::EPositionalFit>::get_guid(), &LOADER__skr_gui_EPositionalFit);
    static EnumTypeFromTraitsLoader<skr::gui::EStackSize> LOADER__skr_gui_EStackSize;
    register_type_loader(RTTRTraits<skr::gui::EStackSize>::get_guid(), &LOADER__skr_gui_EStackSize);
    static EnumTypeFromTraitsLoader<skr::gui::EEventRoutePhase> LOADER__skr_gui_EEventRoutePhase;
    register_type_loader(RTTRTraits<skr::gui::EEventRoutePhase>::get_guid(), &LOADER__skr_gui_EEventRoutePhase);
    static EnumTypeFromTraitsLoader<skr::gui::EEventSource> LOADER__skr_gui_EEventSource;
    register_type_loader(RTTRTraits<skr::gui::EEventSource>::get_guid(), &LOADER__skr_gui_EEventSource);
    static EnumTypeFromTraitsLoader<skr::gui::EHitTestBehavior> LOADER__skr_gui_EHitTestBehavior;
    register_type_loader(RTTRTraits<skr::gui::EHitTestBehavior>::get_guid(), &LOADER__skr_gui_EHitTestBehavior);
    static EnumTypeFromTraitsLoader<skr::gui::EPointerDeviceType> LOADER__skr_gui_EPointerDeviceType;
    register_type_loader(RTTRTraits<skr::gui::EPointerDeviceType>::get_guid(), &LOADER__skr_gui_EPointerDeviceType);
    static EnumTypeFromTraitsLoader<skr::gui::EPointerButton> LOADER__skr_gui_EPointerButton;
    register_type_loader(RTTRTraits<skr::gui::EPointerButton>::get_guid(), &LOADER__skr_gui_EPointerButton);
    static EnumTypeFromTraitsLoader<skr::gui::EPointerModifier> LOADER__skr_gui_EPointerModifier;
    register_type_loader(RTTRTraits<skr::gui::EPointerModifier>::get_guid(), &LOADER__skr_gui_EPointerModifier);
    static EnumTypeFromTraitsLoader<skr::gui::GestureArenaState> LOADER__skr_gui_GestureArenaState;
    register_type_loader(RTTRTraits<skr::gui::GestureArenaState>::get_guid(), &LOADER__skr_gui_GestureArenaState);
};
// END RTTR GENERATED

// BEGIN pop diagnostic
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
// END pop diagnostic
