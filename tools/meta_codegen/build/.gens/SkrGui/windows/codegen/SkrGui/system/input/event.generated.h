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
#define SKR_FILE_ID FID_SkrGui_system_input_event_h_meta

// BEGIN forward declarations
namespace skr::gui { struct Event; }


namespace skr::gui { enum class EEventRoutePhase : uint32_t; }

namespace skr::gui { enum class EEventSource : uint32_t; }
// END forward declarations
// BEGIN RTTR GENERATED
#include "SkrRT/rttr/enum_traits.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"
namespace skr::rttr
{
// enum traits
template <>
struct SKR_GUI_API EnumTraits<skr::gui::EEventRoutePhase>
{
    static skr::span<EnumItem<skr::gui::EEventRoutePhase>> items();
    static skr::StringView                  to_string(const skr::gui::EEventRoutePhase& value);
    static bool                         from_string(skr::StringView str, skr::gui::EEventRoutePhase& value);
};
template <>
struct SKR_GUI_API EnumTraits<skr::gui::EEventSource>
{
    static skr::span<EnumItem<skr::gui::EEventSource>> items();
    static skr::StringView                  to_string(const skr::gui::EEventSource& value);
    static bool                         from_string(skr::StringView str, skr::gui::EEventSource& value);
};
}

// rttr traits
SKR_RTTR_TYPE(::skr::gui::Event, "06ecf250-43e8-44a3-b1e9-b52b1ab53e05")
SKR_RTTR_TYPE(::skr::gui::EEventRoutePhase, "51ed008c-6f63-495c-b229-204986d45cd5")
SKR_RTTR_TYPE(::skr::gui::EEventSource, "03ff08f9-ba01-465a-991c-a6cfa294ddc4")
// END RTTR GENERATED