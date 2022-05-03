#pragma once
#include "platform/configure.h"#ifdef __cplusplus
extern "C" {
#endif
%for enum in db.enums:
%if enum.export_to_c:
RUNTIME_API skr_deserialize_json_${enum.name}(uint64_t* e, skr_json_reader_t* reader);
%endif
%endfor
%for record in db.records:
%if record.export_to_c:
RUNTIME_API void skr_deserialize_json_${record.name}(struct ${record.name}* record, skr_json_reader_t* reader);
#endif
%endif
%endfor
#ifdef __cplusplus
}
#endif