// BEGIN SERIALIZE GENERATED
#include "SkrBase/types.h"
#include "SkrCore/blob.hpp"
#include "SkrSerde/json/reader.h"
#include "SkrSerde/json/writer.h"

// json serde traits
namespace skr::json
{
// record serde
%for json_record in json_records:
template <>
struct ${api} ReadTrait<${json_record.name}>
{
    static bool Read(skr::archive::JsonReader* json, ${json_record.name}& v);
    static bool ReadFields(skr::archive::JsonReader* json, ${json_record.name}& v);
};
template <>
struct ${api} WriteTrait<${json_record.name}>
{
    static bool Write(skr::archive::JsonWriter* writer, const ${json_record.name}& v);
    static bool WriteFields(skr::archive::JsonWriter* writer, const ${json_record.name}& v);
};
%endfor

// enum serde
%for json_enum in json_enums:
template <>
struct ${api} ReadTrait<${json_enum.name}>
{
    static bool Read(skr::archive::JsonReader* json, ${json_enum.name}& v);
};
template <>
struct ${api} WriteTrait<${json_enum.name}>
{
    static bool Write(skr::archive::JsonWriter* writer, ${json_enum.name} v);
};
%endfor
}

// binary serde traits
namespace skr::binary
{
%for bin_record in bin_records:
template<>
struct ${api} ReadTrait<${bin_record.name}>
{
    static bool Read(SBinaryReader* archive, ${bin_record.name}& value);
};
template<>
struct ${api} WriteTrait<${bin_record.name}>
{
    static bool Write(SBinaryWriter* archive, const ${bin_record.name}& value);
};
%endfor
}

// END SERIALIZE GENERATED