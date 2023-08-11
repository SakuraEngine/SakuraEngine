// BEGIN RTTI GENERATED
#include "SkrRT/type/type.hpp"

namespace skr::type
{
%for record in generator.filter_rtti(db.records):
    template<>
    struct type_register<::${record.name}>
    {
        ${api} static void instantiate_type(RecordType* type);
    };
%if hasattr(record.attrs, "hashable"):
    uint64_t Hash(const ${record.name}& value, uint64_t base);
%endif
%endfor

%for enum in generator.filter_rtti(db.enums):
    template<>
    struct type_register<::${enum.name}>
    {
        ${api} static void instantiate_type(EnumType* type);
    };
%endfor
}
//${api} skr::span<const skr_type_t*> skr_get_all_records_${module}();
//${api} skr::span<const skr_type_t*> skr_get_all_enums_${module}();
// END RTTI GENERATED