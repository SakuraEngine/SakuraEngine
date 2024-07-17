// BEGIN JSON GENERATED
#include "SkrSerde/json_serde.hpp"
#include "SkrSerde/enum_serde_traits.hpp"

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
    struct ${api} EnumSerdeTraits<${enum.name}>
    {
        static skr::StringView to_string(const ${enum.name}& value);
        static bool from_string(skr::StringView str, ${enum.name}& value);
    };
%endfor
}
// END JSON GENERATED
