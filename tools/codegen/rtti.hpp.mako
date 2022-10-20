// BEGIN RTTI GENERATED
#include "type/type_registry.h"

namespace skr::type
{
%for record in generator.filter_rtti(db.records):
    template<>
    struct type_of<::${record.name}>
    {
        ${api} static const skr_type_t* get();
    };
%if hasattr(record.attrs, "hashable"):
    size_t Hash(const ${record.name}& value, size_t base);
%endif
%endfor

%for enum in generator.filter_rtti(db.enums):
    template<>
    struct type_of <::${enum.name}>
    {
        ${api} static const skr_type_t* get();
    };
%endfor
}
// END RTTI GENERATED