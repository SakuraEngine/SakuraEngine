// BEGIN JSON GENERATED
#ifdef __cplusplus
#include "json/reader.h"
#include "json/writer.h"
namespace skr::json
{
%for record in generator.filter_types(db.records):
    template <>
    ${api} error_code ReadValue(simdjson::ondemand::value&& json, ${record.name}& v);
    template <>
    ${api} void WriteValue(skr_json_writer_t* writer, const ${record.name}& v);
    template <>
    ${api} void WriteFields(skr_json_writer_t* writer, const ${record.name}& v);
%endfor
%for enum in generator.filter_types(db.enums):
    template <>
    ${api} error_code ReadValue(simdjson::ondemand::value&& json, ${enum.name}& v);
    template <>
    ${api} void WriteValue(skr_json_writer_t* writer, ${enum.name} v);
%endfor
}
#endif
// END JSON GENERATED
