//DO NOT MODIFY THIS FILE
#include "json_writer.generated.h"
%for header in db.headers:
#include "${header}"
%endfor

namespace skr::json {
%for enum in db.enums:
template<>
void Write(skr_json_writer_t* writer, ${enum.name} e)
{
    switch(e)
    {
    %for enumerator in enum.enumerators:
        case ${enumerator.name}: writer->String("${enumerator.name}", ${len(enumerator.name)}); return;
    %endfor
    }
} 
%endfor

%for record in db.records:
template<>
void Write(skr_json_writer_t* writer, const ${record.name}& record)
{
    writer->StartObject();
    %for field in record.allFields():
    writer->Key("${field.name}", ${len(field.name)});
    skr::json::Write<TParamType<${field.type}>>(writer, record.${field.name});
    %endfor
    writer->EndObject();
} 
%endfor
}

#ifdef __cplusplus
extern "C" {
#endif
%for enum in db.enums:
%if enum.export_to_c:
void skr_serialize_json_${enum.name}(uint64_t e, skr_json_writer_t* writer) { skr::json::Write(writer, (${enum.name})e); }
%endif
%endfor
%for record in db.records:
%if record.export_to_c:
void skr_serialize_json_${record.name}(${record.name}* record, skr_json_writer_t* writer) { skr::json::Write<const ${record.name}&>(writer, *record); }
%endif
%endfor
#ifdef __cplusplus
}
#endif