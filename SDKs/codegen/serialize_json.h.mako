#pragma once
#include "platform/configure.h"
#ifdef __cplusplus
extern "C" {
#endif

%for enum in db.enums:
%if enum.export_to_c:
RUNTIME_API void skr_serialize_json_${enum.name}(uint64_t e, struct skr_json_writer_t* writer);
%endif
%endfor

%for record in db.records:
%if record.export_to_c:
RUNTIME_API void skr_serialize_json_${record.name}(struct ${record.name}* record, struct skr_json_writer_t* writer);
%endif
%endfor

#ifdef __cplusplus
}
#endif