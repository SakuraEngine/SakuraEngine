// DO NOT MODIFY THIS FILE
// START READER IMPLEMENTATION
#include "utils/hash.h"
#include "platform/debug.h"
#include "utils/log.h"
#include "json_serialize.generated.h"
%for header in db.headers:
#include "${header}"
%endfor

namespace skr::json {
%for enum in db.enums:
template<>
error_code ReadValue(simdjson::ondemand::value&& json, ${enum.name}& e)
{
    auto value = json.get_string();
    if (value.error() != simdjson::SUCCESS)
        return (error_code)value.error();
    std::string_view enumStr = value.value_unsafe();
    auto hash = hash_crc32(enumStr);
    switch(hash)
    {
    %for enumerator in enum.enumerators:
        case hash_crc32<char>("${enumerator.name}"): if( enumStr == "${enumerator.name}") e = ${enumerator.name}; return error_code::SUCCESS;
    %endfor
        default:
            SKR_LOG_ERROR("Unknown enumerator while reading enum ${enum.name}: %s", enumStr);
            return error_code::ENUMERATOR_ERROR;
    }
    SKR_UNREACHABLE_CODE();
} 

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
error_code ReadValue(simdjson::ondemand::value&& json, ${record.name}& record)
{
    %for field in record.allFields():
    {
        auto field = json["${field.name}"];
        if (field.error() == simdjson::NO_SUCH_FIELD)
        {
            SKR_LOG_ERROR("Field ${field.name} in record ${record.name} not found while reading");
        }
        else if (field.error() != simdjson::SUCCESS)
        {
            SKR_LOG_ERROR("Failed to read record ${record.name} %s", error_message((error_code)field.error()));
            return (error_code)field.error();
        }
        else
        {
            error_code result = skr::json::Read(std::move(field).value_unsafe(), (${field.type}&)record.${field.name});
            if(result != error_code::SUCCESS)
            {
                SKR_LOG_ERROR("Failed to read field ${field.name} of record ${record.name} %s", error_message(result));
                return result;
            }
        }
    }
    %endfor
    return error_code::SUCCESS;
} 
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
void skr_deserialize_json_${enum.name}(uint64_t* e, skr_json_reader_t* reader) { skr::json::Read<${enum.name}>(std::move(*reader->json), *(${enum.name}*)e); }
void skr_serialize_json_${enum.name}(uint64_t e, skr_json_writer_t* writer) { skr::json::Write(writer, (${enum.name})e); }
%endif
%endfor

%for record in db.records:
%if record.export_to_c:
void skr_deserialize_json_${record.name}(${record.name}* record, skr_json_reader_t* reader) { skr::json::Read<${record.name}>(std::move(*reader->json), *record); }
void skr_serialize_json_${record.name}(${record.name}* record, skr_json_writer_t* writer) { skr::json::Write<const ${record.name}&>(writer, *record); }
%endif
%endfor
#ifdef __cplusplus
}
#endif
