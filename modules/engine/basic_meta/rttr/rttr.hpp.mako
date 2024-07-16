// BEGIN RTTR GENERATED
#include "SkrRTTR/enum_tools.hpp"
#include "SkrRTTR/rttr_traits.hpp"
%if len(enums) > 0:
namespace skr::rttr
{
// enum traits
%for enum in enums:
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
%for record in records:
SKR_RTTR_TYPE(${record.name}, "${record.generator_data["guid"]}")
%endfor
%for enum in enums:
SKR_RTTR_TYPE(${enum.name}, "${enum.generator_data["guid"]}")
%endfor
// END RTTR GENERATED