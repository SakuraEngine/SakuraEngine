//DO NOT MODIFY THIS FILE
#include "json/writer.h"
%for header in db.headers:
#include "${header}"
%endfor

%for enum in db.enums:
namespace skr::json {
template<>
void Write(skr_json_writer_t* writer, ${enum.name} e)
{
    switch(e)
    {
    %for enumerator in enum.enumerators:
        case ${enumerator.name}: writer->String("${enumerator.name}", ${len(enumerator.name)}); break;
    %endfor
    }
} 
}

%if enum.export_to_c:
#ifdef __cplusplus
extern "C" {
#endif
void skr_serialize_json_${enum.name}(uint64_t e, skr_json_writer_t* writer) { skr::json::Write(writer, (${enum.name})e); }
#ifdef __cplusplus
}
#endif
%endif
%endfor

%for record in db.records:
namespace skr::json {
template<>
void Write(skr_json_writer_t* writer, const ${record.name}& record)
{
    writer->StartObject();
    %for field in record.fields:
    writer->Key("${field.name}", ${len(field.name)});
    skr::json::Write<TParamType<${field.type}>>(writer, record.${field.name});
    %endfor
    writer->EndObject();
} 
}

%if record.export_to_c:
#ifdef __cplusplus
extern "C" {
#endif
void skr_serialize_json_${record.name}(${record.name}* record, skr_json_writer_t* writer) { skr::json::Write<const ${record.name}&>(writer, *record); }
#ifdef __cplusplus
}
#endif
%endif
%endfor