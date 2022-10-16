#pragma once
#include "${config}"
#include "json/reader.h"
#ifdef __cplusplus
extern "C" {
#endif
%for enum in db.enums:
%if enum.export_to_c:
${api} void skr_deserialize_json_${enum.name}(uint64_t* e, skr_json_reader_t* reader);
%endif
%endfor
%for record in db.records:
%if record.export_to_c:
${api} void skr_deserialize_json_${record.name}(struct ${record.name}* record, skr_json_reader_t* reader);
%endif
%endfor
#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
%for record in db.records:
%if hasattr(record, "namespace"):
namespace ${record.namespace} { struct ${record.short_name}; }
%else:
struct ${record.short_name};
%endif
%endfor
%for enum in db.enums:
%if hasattr(enum, "namespace"):
namespace ${enum.namespace} { enum ${enum.short_name} ${enum.postfix}; }
%else:
enum ${enum.short_name} ${enum.postfix};
%endif
%endfor
namespace skr::json
{
%for record in db.records:
    template <>
    ${api} error_code ReadValue(simdjson::ondemand::value&& json, ${record.name}& v);
%endfor
%for enum in db.enums:
    template <>
    ${api} error_code ReadValue(simdjson::ondemand::value&& json, ${enum.name}& v);
%endfor
}
#endif