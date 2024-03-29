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
        static error_code Read(value_t&& json, ${record.name}& v);
    };
%endif
    template <>
    struct ${api} WriteTrait<${record.name}>
    {
        static void Write(skr_json_writer_t* writer, const ${record.name}& v);
        static void WriteFields(skr_json_writer_t* writer, const ${record.name}& v);
    };
%endfor
%for enum in generator.filter_types(db.enums):
    template <>
    struct ${api} ReadTrait<${enum.name}>
    {
        static error_code Read(value_t&& json, ${enum.name}& v);
    };

    template <>
    struct ${api} WriteTrait<${enum.name}>
    {
        static void Write(skr_json_writer_t* writer, ${enum.name} v);
    };
%endfor
}
#endif
// END JSON GENERATED
