#pragma once
#include "type/type_id.hpp"

%for type in db.types:
%if hasattr(type, "namespace"):
namespace ${type.namespace} { struct ${type.short_name}; }
%else:
struct ${type.short_name};
%endif
%endfor

namespace skr::type
{
%for type in db.types:
    template<>
    struct type_id<::${type.name}>
    {
        inline static SKR_CONSTEXPR skr_guid_t get()
        {
            return {${type.guidConstant}}; 
        }
    };
%endfor
}