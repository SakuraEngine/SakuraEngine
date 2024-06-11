// BEGIN JSON GENERATED
#ifdef __cplusplus
#include "SkrBase/types.h"

namespace skr::json
{
%for record in generator.filter_types(db.records):
%if not generator.filter_debug_type(record):
    template <>
    struct ${api} ReadTrait<${record.name}>
    {
        static bool Read(skr::json::Reader* json, ${record.name}& v);
    };
%endif
    template <>
    struct ${api} WriteTrait<${record.name}>
    {
        static bool Write(SJsonWriter* writer, const ${record.name}& v);
        static bool WriteFields(SJsonWriter* writer, const ${record.name}& v);
    };
%endfor
%for enum in generator.filter_types(db.enums):
    template <>
    struct ${api} ReadTrait<${enum.name}>
    {
        static bool Read(skr::json::Reader* json, ${enum.name}& v);
    };

    template <>
    struct ${api} WriteTrait<${enum.name}>
    {
        static bool Write(SJsonWriter* writer, ${enum.name} v);
    };
%endfor
}
#endif
// END JSON GENERATED
