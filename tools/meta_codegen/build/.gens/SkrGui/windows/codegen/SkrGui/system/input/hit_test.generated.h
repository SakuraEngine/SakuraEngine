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
#define SKR_FILE_ID FID_SkrGui_system_input_hit_test_h_meta

// BEGIN forward declarations


namespace skr::gui { enum class EHitTestBehavior : uint32_t; }
// END forward declarations
// BEGIN RTTR GENERATED
#include "SkrRT/rttr/enum_traits.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"
namespace skr::rttr
{
// enum traits
template <>
struct SKR_GUI_API EnumTraits<skr::gui::EHitTestBehavior>
{
    static skr::span<EnumItem<skr::gui::EHitTestBehavior>> items();
    static skr::StringView                  to_string(const skr::gui::EHitTestBehavior& value);
    static bool                         from_string(skr::StringView str, skr::gui::EHitTestBehavior& value);
};
}

// rttr traits
SKR_RTTR_TYPE(::skr::gui::EHitTestBehavior, "e59baca9-43d6-4368-869b-e122ee58d53d")
// END RTTR GENERATED