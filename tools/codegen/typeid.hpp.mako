//BEGIN TYPEID GENERATED
#include "type/type_id.hpp"

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
    };
%endfor
}
//END TYPEID GENERATED