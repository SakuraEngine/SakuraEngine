// BEGIN JSON GENERATED
#ifdef __cplusplus
#include "json/reader_fwd.h"
#include "json/writer_fwd.h"
#include "type/enum_to_string.hpp"
namespace skr::json
{
%for record in generator.filter_types(db.records):
    template <>
    struct ${api} ReadTrait<${record.name}>
    {
        static error_code Read(value_t&& json, ${record.name}& v);
    };

    template <>
    struct ${api} WriteTrait<const ${record.name}&>
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
    struct ${api} WriteTrait<const ${enum.name}&>
    {
        static void Write(skr_json_writer_t* writer, ${enum.name} v);
    };
%endfor
}
namespace skr::type
{
%for enum in generator.filter_types(db.enums):
    template <>
    struct ${api} EnumToStringTrait<${enum.name}>
    {
        static std::string_view ToString(${enum.name} v);
        static bool FromString(std::string_view str, ${enum.name}& v);
    };
%endfor
}
#endif
// END JSON GENERATED
