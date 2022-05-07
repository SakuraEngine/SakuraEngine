//DO NOT MODIFY THIS FILE
#include "json/reader.h"
#include "utils/hash.h"
#include "platform/debug.h"
%for header in db.headers:
#include "${header}"
%endfor

namespace skr::json {
%for enum in db.enums:
template<>
void Read(simdjson::ondemand::value&& json, ${enum.name}& e)
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
void Read(simdjson::ondemand::value&& json, ${record.name}& record)
{
    %for field in record.allFields():
    skr::json::Read<${field.type}>(json.find_field("${field.name}").value_unsafe(), record.${field.name});
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
