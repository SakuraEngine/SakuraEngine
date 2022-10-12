// DO NOT MODIFY THIS FILE
// START READER IMPLEMENTATION
#include "utils/hash.h"
#include "platform/debug.h"
#include "json_reader.generated.h"
%for header in db.headers:
#include "${header}"
%endfor

namespace skr::json {
%for enum in db.enums:
template<>
void ReadValue(simdjson::ondemand::value&& json, ${enum.name}& e)
{
    std::string_view enumStr = json.get_string().value_unsafe();
    auto hash = hash_crc32(enumStr);
    switch(hash)
    {
    %for enumerator in enum.enumerators:
        case hash_crc32<char>("${enumerator.name}"): if( enumStr == "${enumerator.name}") e = ${enumerator.name}; return;
    %endfor
    }
    SKR_UNREACHABLE_CODE();
} 
%endfor

%for record in db.records:
template<>
void ReadValue(simdjson::ondemand::value&& json, ${record.name}& record)
{
    %for field in record.allFields():
    skr::json::Read(json["${field.name}"].value_unsafe(), (${field.type}&)record.${field.name});
    %endfor
} 
%endfor
}

#ifdef __cplusplus
extern "C" {
#endif
%for enum in db.enums:
%if enum.export_to_c:
void skr_deserialize_json_${enum.name}(uint64_t* e, skr_json_reader_t* reader) { skr::json::Read<${enum.name}>(std::move(*reader->json), *(${enum.name}*)e); }
%endif
%endfor

%for record in db.records:
%if record.export_to_c:
void skr_deserialize_json_${record.name}(${record.name}* record, skr_json_reader_t* reader) { skr::json::Read<${record.name}>(std::move(*reader->json), *record); }
%endif
%endfor
#ifdef __cplusplus
}
#endif

// DO NOT MODIFY THIS FILE
// START WRITER IMPLEMENTATION
#include "json_writer.generated.h"
%for header in db.headers:
#include "${header}"
%endfor

namespace skr::json {
%for enum in db.enums:
template<>
void WriteValue(skr_json_writer_t* writer, ${enum.name} e)
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
void WriteValue(skr_json_writer_t* writer, const ${record.name}& record)
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