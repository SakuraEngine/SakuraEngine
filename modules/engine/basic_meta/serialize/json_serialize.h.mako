// BEGIN JSON GENERATED
#ifdef __cplusplus
#include "SkrBase/types.h"
#include "SkrSerde/json/reader.h"
#include "SkrSerde/json/writer.h"

namespace skr::json
{
%for record in generator.filter_types(db.records):
%if not generator.filter_debug_type(record):
    template <>
    struct ${api} ReadTrait<${record.name}>
    {
        static bool Read(skr::archive::JsonReader* json, ${record.name}& v);
        static bool ReadFields(skr::archive::JsonReader* json, ${record.name}& v);
    };
%endif
    template <>
    struct ${api} WriteTrait<${record.name}>
    {
        static bool Write(skr::archive::JsonWriter* writer, const ${record.name}& v);
        static bool WriteFields(skr::archive::JsonWriter* writer, const ${record.name}& v);
    };
%endfor
%for enum in generator.filter_types(db.enums):
    template <>
    struct ${api} ReadTrait<${enum.name}>
    {
        static bool Read(skr::archive::JsonReader* json, ${enum.name}& v);
    };

    template <>
    struct ${api} WriteTrait<${enum.name}>
    {
        static bool Write(skr::archive::JsonWriter* writer, ${enum.name} v);
    };
%endfor
}
#endif
// END JSON GENERATED
