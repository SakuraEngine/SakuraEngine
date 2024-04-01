//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!! THIS FILE IS GENERATED, ANY CHANGES WILL BE LOST !!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#pragma once
#include "SkrBase/config.h"
#include <inttypes.h>

#ifdef __meta__
#error "this file should not be inspected by meta"
#endif

#ifdef SKR_FILE_ID
    #undef SKR_FILE_ID
#endif
#define SKR_FILE_ID FID_SkrGui_system_input_pointer_event_h_meta

// BEGIN forward declarations
namespace skr::gui { struct PointerEvent; }
namespace skr::gui { struct PointerDownEvent; }
namespace skr::gui { struct PointerUpEvent; }
namespace skr::gui { struct PointerMoveEvent; }
namespace skr::gui { struct PointerEnterEvent; }
namespace skr::gui { struct PointerExitEvent; }
namespace skr::gui { struct PointerScrollEvent; }
namespace skr::gui { struct PointerScaleEvent; }
namespace skr::gui { struct PointerPanZoomStartEvent; }
namespace skr::gui { struct PointerPanZoomUpdateEvent; }
namespace skr::gui { struct PointerPanZoomEndEvent; }


namespace skr::gui { enum class EPointerDeviceType : uint32_t; }

namespace skr::gui { enum class EPointerButton : uint32_t; }

namespace skr::gui { enum class EPointerModifier : uint32_t; }
// END forward declarations
// BEGIN RTTR GENERATED
#include "SkrRT/rttr/enum_traits.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"
namespace skr::rttr
{
// enum traits
template <>
struct SKR_GUI_API EnumTraits<skr::gui::EPointerDeviceType>
{
    static skr::span<EnumItem<skr::gui::EPointerDeviceType>> items();
    static skr::StringView                  to_string(const skr::gui::EPointerDeviceType& value);
    static bool                         from_string(skr::StringView str, skr::gui::EPointerDeviceType& value);
};
template <>
struct SKR_GUI_API EnumTraits<skr::gui::EPointerButton>
{
    static skr::span<EnumItem<skr::gui::EPointerButton>> items();
    static skr::StringView                  to_string(const skr::gui::EPointerButton& value);
    static bool                         from_string(skr::StringView str, skr::gui::EPointerButton& value);
};
template <>
struct SKR_GUI_API EnumTraits<skr::gui::EPointerModifier>
{
    static skr::span<EnumItem<skr::gui::EPointerModifier>> items();
    static skr::StringView                  to_string(const skr::gui::EPointerModifier& value);
    static bool                         from_string(skr::StringView str, skr::gui::EPointerModifier& value);
};
}

// rttr traits
SKR_RTTR_TYPE(::skr::gui::PointerEvent, "fa4706c4-5982-4056-a5b5-b12098c0963a")
SKR_RTTR_TYPE(::skr::gui::PointerDownEvent, "91343bb8-4b20-4734-b491-23362b76aa17")
SKR_RTTR_TYPE(::skr::gui::PointerUpEvent, "8817dd2b-7e0d-47f5-8ee5-72790bbf3f09")
SKR_RTTR_TYPE(::skr::gui::PointerMoveEvent, "453f7052-9740-4136-9831-55e8188827d2")
SKR_RTTR_TYPE(::skr::gui::PointerEnterEvent, "6cddd04b-749c-4a5e-99b0-27396ef84d50")
SKR_RTTR_TYPE(::skr::gui::PointerExitEvent, "f41e2f74-b813-411d-b065-5df10f5edaeb")
SKR_RTTR_TYPE(::skr::gui::PointerScrollEvent, "1da2a830-544a-44c4-8ba7-2f313194bced")
SKR_RTTR_TYPE(::skr::gui::PointerScaleEvent, "51778097-47ab-4eb3-9193-3cecffedf8a0")
SKR_RTTR_TYPE(::skr::gui::PointerPanZoomStartEvent, "5d3aeff7-4cd5-41d2-93ba-0d0cdd14b9b0")
SKR_RTTR_TYPE(::skr::gui::PointerPanZoomUpdateEvent, "f25e3d34-4fae-4be8-8f17-385b960a95f8")
SKR_RTTR_TYPE(::skr::gui::PointerPanZoomEndEvent, "d8ee52fd-2f4e-4a62-a922-d3cb2467883b")
SKR_RTTR_TYPE(::skr::gui::EPointerDeviceType, "7223961b-5309-4fac-8207-8476bf7f3b05")
SKR_RTTR_TYPE(::skr::gui::EPointerButton, "3f6c7998-a93e-424e-9d95-c75800309a16")
SKR_RTTR_TYPE(::skr::gui::EPointerModifier, "f8f9a770-aa9a-4ce3-9ecf-613c8bda7c9a")
// END RTTR GENERATED