// BEGIN JSON GENERATED
#ifdef __cplusplus
#include "json/reader.h"
#include "json/writer.h"
namespace skr::json
{
%for record in db.records:
%if generator.filter_type(record):
    template <>
    ${api} error_code ReadValue(simdjson::ondemand::value&& json, ${record.name}& v);
    template <>
    ${api} void WriteValue(skr_json_writer_t* writer, const ${record.name}& v);
    template <>
    ${api} void WriteFields(skr_json_writer_t* writer, const ${record.name}& v);
%endif
%endfor
%for enum in db.enums:
%if generator.filter_type(enum):
    template <>
    ${api} error_code ReadValue(simdjson::ondemand::value&& json, ${enum.name}& v);
    template <>
    ${api} void WriteValue(skr_json_writer_t* writer, ${enum.name} v);
%endif
%endfor
}
#endif
// END JSON GENERATED
