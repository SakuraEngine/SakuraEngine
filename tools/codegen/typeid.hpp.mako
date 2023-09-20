//BEGIN TYPEID GENERATED
#include "SkrRT/type/type_id.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"

namespace skr::type
{
%for record in generator.filter_types(db.records):
    template<>
    struct type_id<::${record.name}>
    {
        inline static SKR_CONSTEXPR skr_guid_t get()
        {
            return {${db.guid_constant(record)}}; 
        }
        inline static SKR_CONSTEXPR std::string_view str()
        {
            return "${record.attrs.guid}";
        }
    };
%endfor
%for enum in generator.filter_types(db.enums):
    template<>
    struct type_id <::${enum.name}>
    {
        inline static SKR_CONSTEXPR skr_guid_t get()
        {
            return {${db.guid_constant(enum)}}; 
        }
        inline static SKR_CONSTEXPR std::string_view str()
        {
            return "${enum.attrs.guid}";
        }
    };
%endfor
}
//END TYPEID GENERATED

// BEGIN RTTR TRAITS GENERATED
%for record in generator.filter_types(db.records):
SKR_RTTR_TYPE(::${record.name}, "${record.attrs.guid}")
%endfor
%for enum in generator.filter_types(db.enums):
SKR_RTTR_TYPE(::${enum.name}, "${enum.attrs.guid}")
%endfor
// END RTTR TRAITS GENERATED