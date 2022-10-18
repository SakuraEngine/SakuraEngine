//DO NOT MODIFY THIS FILE
#pragma once
#include "${config}"
#include "ecs/dual.h"

%for type in db.types:
%if hasattr(type, "namespace"):
namespace ${type.namespace} { struct ${type.short_name}; }
%else:
struct ${type.short_name};
%endif
%endfor

%for type in db.types:
template<>
struct dual_id_of<::${type.name}>
{
    ${api} static dual_type_index_t get();
};
%endfor