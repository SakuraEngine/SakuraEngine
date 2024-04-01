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
#define SKR_FILE_ID FID_SkrGui_system_input_gesture_gesture_arena_h_meta

// BEGIN forward declarations
namespace skr::gui { struct GestureArena; }


namespace skr::gui { enum class GestureArenaState : int; }
// END forward declarations
// BEGIN RTTR GENERATED
#include "SkrRT/rttr/enum_traits.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"
namespace skr::rttr
{
// enum traits
template <>
struct SKR_GUI_API EnumTraits<skr::gui::GestureArenaState>
{
    static skr::span<EnumItem<skr::gui::GestureArenaState>> items();
    static skr::StringView                  to_string(const skr::gui::GestureArenaState& value);
    static bool                         from_string(skr::StringView str, skr::gui::GestureArenaState& value);
};
}

// rttr traits
SKR_RTTR_TYPE(::skr::gui::GestureArena, "952528b4-510f-450f-81fa-cbdb3d8b2d4a")
SKR_RTTR_TYPE(::skr::gui::GestureArenaState, "64d988e6-5d8c-4418-b070-2ae6ec3ddf08")
// END RTTR GENERATED