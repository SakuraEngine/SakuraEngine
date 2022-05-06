#pragma once
%for header in db.headers:
#include "${header}"
%endfor

%for type in db.types:
%if hasattr(type, 'namespace'):
namespace ${type.namespace} {
%endif
constexpr skr_guid_t get_type_id_${type.short_name}()
{ return {${type.guidConstant}}; }
%if hasattr(type, 'namespace'):
}
%endif
%endfor