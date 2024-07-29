// BEGIN SERIALIZE GENERATED
#include "SkrSerde/bin_serde.hpp"
#include "SkrSerde/json_serde.hpp"

// json serde
namespace skr
{
%for enum in json_enums:
template <>
struct ${api} EnumSerdeTraits<${enum.name}>
{
    static skr::StringView to_string(const ${enum.name}& value);
    static bool from_string(skr::StringView str, ${enum.name}& value);
};
%endfor
%for record in json_records:
template <>
struct ${api} JsonSerde<${record.name}>
{
    static bool read_fields(skr::archive::JsonReader* r, ${record.name}& v);
    static bool write_fields(skr::archive::JsonWriter* w, const ${record.name}& v);

    static bool read(skr::archive::JsonReader* r, ${record.name}& v);
    static bool write(skr::archive::JsonWriter* w, const ${record.name}& v);
};
%endfor
}

// bin serde
namespace skr
{
%for record in bin_records:
template<>
struct ${api} BinSerde<${record.name}>
{
    static bool read(SBinaryReader* r, ${record.name}& v);
    static bool write(SBinaryWriter* w, const ${record.name}& v);
};
%endfor
}

// END SERIALIZE GENERATED
