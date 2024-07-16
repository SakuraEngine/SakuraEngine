// BEGIN JSON GENERATED
#include "SkrSerde/json_serde.hpp"

namespace skr
{
%for record in generator.filter_types(db.records):
    template <>
    struct ${api} JsonSerde<${record.name}>
    {
        static bool read_fields(skr::archive::JsonReader* r, ${record.name}& v);
        static bool write_fields(skr::archive::JsonWriter* w, const ${record.name}& v);
    
        static bool read(skr::archive::JsonReader* r, ${record.name}& v);
        static bool write(skr::archive::JsonWriter* w, const ${record.name}& v);
    };
%endfor
%for enum in generator.filter_types(db.enums):
    template <>
    struct ${api} JsonSerde<${enum.name}>
    {
        static bool read(skr::archive::JsonReader* r, ${enum.name}& v);
        static bool write(skr::archive::JsonWriter* w, ${enum.name} v);
    };
%endfor
}
// END JSON GENERATED
