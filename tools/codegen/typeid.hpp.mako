//DO NOT MODIFY THIS FILE
#pragma once
#include "type/type_id.hpp"

%for record in db.records:
%if hasattr(record, "namespace"):
namespace ${record.namespace} { struct ${record.short_name}; }
%else:
struct ${record.short_name};
%endif
%endfor
%for enum in db.enums:
%if hasattr(enum, "namespace"):
namespace ${enum.namespace} { enum ${enum.short_name} ${enum.postfix}; }
%else:
enum ${enum.short_name} ${enum.postfix};
%endif
%endfor

namespace skr::type
{
%for record in db.records:
    template<>
    struct type_id<::${record.name}>
    {
        inline static SKR_CONSTEXPR skr_guid_t get()
        {
            return {${record.guidConstant}}; 
        }
    };
%endfor
%for enum in db.enums:
    template<>
    struct type_id <::${enum.name}>
    {
        inline static SKR_CONSTEXPR skr_guid_t get()
        {
            return {${enum.guidConstant}}; 
        }
    };
%endfor
}