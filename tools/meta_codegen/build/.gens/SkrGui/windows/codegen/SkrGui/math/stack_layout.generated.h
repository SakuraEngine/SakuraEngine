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
#define SKR_FILE_ID FID_SkrGui_math_stack_layout_h_meta

// BEGIN forward declarations


namespace skr::gui { enum class EPositionalFit : uint8_t; }

namespace skr::gui { enum class EStackSize : uint8_t; }
// END forward declarations
// BEGIN RTTR GENERATED
#include "SkrRT/rttr/enum_traits.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"
namespace skr::rttr
{
// enum traits
template <>
struct SKR_GUI_API EnumTraits<skr::gui::EPositionalFit>
{
    static skr::span<EnumItem<skr::gui::EPositionalFit>> items();
    static skr::StringView                  to_string(const skr::gui::EPositionalFit& value);
    static bool                         from_string(skr::StringView str, skr::gui::EPositionalFit& value);
};
template <>
struct SKR_GUI_API EnumTraits<skr::gui::EStackSize>
{
    static skr::span<EnumItem<skr::gui::EStackSize>> items();
    static skr::StringView                  to_string(const skr::gui::EStackSize& value);
    static bool                         from_string(skr::StringView str, skr::gui::EStackSize& value);
};
}

// rttr traits
SKR_RTTR_TYPE(::skr::gui::EPositionalFit, "01092beb-ecd0-4292-8217-7998997a8746")
SKR_RTTR_TYPE(::skr::gui::EStackSize, "1a43aede-a6f7-4750-8f29-da2965b9abcb")
// END RTTR GENERATED