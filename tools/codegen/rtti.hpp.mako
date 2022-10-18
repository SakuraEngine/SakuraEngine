//DO NOT MODIFY THIS FILE
#pragma once
#include "${config}"
#include "type/type_registry.h"

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
    struct type_of<::${record.name}>
    {
        ${api} static const skr_type_t* get();
    };
%endfor
%for enum in db.enums:
    template<>
    struct type_of <::${enum.name}>
    {
        ${api} static const skr_type_t* get();
    };
%endfor
}