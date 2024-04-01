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
#define SKR_FILE_ID FID_SkrGui_math_flex_layout_h_meta

// BEGIN forward declarations


namespace skr::gui { enum class EFlexDirection : uint8_t; }

namespace skr::gui { enum class EMainAxisAlignment : uint8_t; }

namespace skr::gui { enum class ECrossAxisAlignment : uint8_t; }

namespace skr::gui { enum class EMainAxisSize : uint8_t; }

namespace skr::gui { enum class EFlexFit : uint8_t; }
// END forward declarations
// BEGIN RTTR GENERATED
#include "SkrRT/rttr/enum_traits.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"
namespace skr::rttr
{
// enum traits
template <>
struct SKR_GUI_API EnumTraits<skr::gui::EFlexDirection>
{
    static skr::span<EnumItem<skr::gui::EFlexDirection>> items();
    static skr::StringView                  to_string(const skr::gui::EFlexDirection& value);
    static bool                         from_string(skr::StringView str, skr::gui::EFlexDirection& value);
};
template <>
struct SKR_GUI_API EnumTraits<skr::gui::EMainAxisAlignment>
{
    static skr::span<EnumItem<skr::gui::EMainAxisAlignment>> items();
    static skr::StringView                  to_string(const skr::gui::EMainAxisAlignment& value);
    static bool                         from_string(skr::StringView str, skr::gui::EMainAxisAlignment& value);
};
template <>
struct SKR_GUI_API EnumTraits<skr::gui::ECrossAxisAlignment>
{
    static skr::span<EnumItem<skr::gui::ECrossAxisAlignment>> items();
    static skr::StringView                  to_string(const skr::gui::ECrossAxisAlignment& value);
    static bool                         from_string(skr::StringView str, skr::gui::ECrossAxisAlignment& value);
};
template <>
struct SKR_GUI_API EnumTraits<skr::gui::EMainAxisSize>
{
    static skr::span<EnumItem<skr::gui::EMainAxisSize>> items();
    static skr::StringView                  to_string(const skr::gui::EMainAxisSize& value);
    static bool                         from_string(skr::StringView str, skr::gui::EMainAxisSize& value);
};
template <>
struct SKR_GUI_API EnumTraits<skr::gui::EFlexFit>
{
    static skr::span<EnumItem<skr::gui::EFlexFit>> items();
    static skr::StringView                  to_string(const skr::gui::EFlexFit& value);
    static bool                         from_string(skr::StringView str, skr::gui::EFlexFit& value);
};
}

// rttr traits
SKR_RTTR_TYPE(::skr::gui::EFlexDirection, "a87aa2f4-fafd-48cc-a224-8429f5bd771f")
SKR_RTTR_TYPE(::skr::gui::EMainAxisAlignment, "016b9be8-5d1d-476e-9078-ab8ea432f2b2")
SKR_RTTR_TYPE(::skr::gui::ECrossAxisAlignment, "9ad7ec08-d31e-43eb-bb86-2dbecb34f130")
SKR_RTTR_TYPE(::skr::gui::EMainAxisSize, "4f356a69-c034-410a-a9e1-a8d7469d977e")
SKR_RTTR_TYPE(::skr::gui::EFlexFit, "b8f94956-d449-430a-876e-243fda848d58")
// END RTTR GENERATED