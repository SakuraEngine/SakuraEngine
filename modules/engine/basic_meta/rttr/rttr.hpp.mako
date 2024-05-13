// BEGIN RTTR GENERATED
#include "SkrRTTR/enum_traits.hpp"
#include "SkrRTTR/rttr_traits.hpp"
%if len(generator.enums) > 0:
namespace skr::rttr
{
// enum traits
%for enum in generator.enums:
template <>
struct ${api} EnumTraits<${enum.name}>
{
    static skr::span<EnumItem<${enum.name}>> items();
    static skr::StringView                  to_string(const ${enum.name}& value);
    static bool                         from_string(skr::StringView str, ${enum.name}& value);
};
%endfor
}
%endif

// rttr traits
%for record in generator.records:
SKR_RTTR_TYPE(${record.name}, "${record.attrs.guid}")
%endfor
%for enum in generator.enums:
SKR_RTTR_TYPE(${enum.name}, "${enum.attrs.guid}")
%endfor
// END RTTR GENERATED