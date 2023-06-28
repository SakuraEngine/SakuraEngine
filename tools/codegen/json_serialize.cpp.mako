// BEGIN JSON IMPLEMENTATION
#include "misc/hash.h"
#include "platform/debug.h"
#include "misc/log.h"
#include "serde/json/reader.h"
#include "serde/json/writer.h"

#include "tracy/Tracy.hpp"

namespace skr::type
{
%for enum in generator.filter_types(db.enums):
skr::string_view EnumToStringTrait<${enum.name}>::ToString(${enum.name} value)
{
    switch (value)
    {
    %for name, value in vars(enum.values).items():
    case ${enum.name}::${db.short_name(name)}: return u8"${db.short_name(name)}";
    %endfor
    default: SKR_UNREACHABLE_CODE(); return u8"${enum.name}::INVALID_ENUMERATOR";
    }
}
bool EnumToStringTrait<${enum.name}>::FromString(skr::string_view enumStr, ${enum.name}& e)
{
    const std::string_view enumStrV = {(const char*)enumStr.u8_str(), (size_t)enumStr.size()};
    const auto hash = hash_crc32(enumStrV);
    switch(hash)
    {
    %for name, value in vars(enum.values).items():
        case hash_crc32<char>("${db.short_name(name)}"): if(enumStr == u8"${db.short_name(name)}") e = ${name}; return true;
    %endfor
        default:
            return false;
    }
}
%endfor
}

namespace skr::json {
%for enum in generator.filter_types(db.enums):
error_code ReadTrait<${enum.name}>::Read(value_t&& json, ${enum.name}& e)
{
    auto value = json.get_string();
    if (value.error() != simdjson::SUCCESS)
        return (error_code)value.error();
    const auto rawView = value.value_unsafe();
    const auto enumStr = skr::string_view((const char8_t*)rawView.data(), rawView.size());
    if(!skr::type::enum_from_string(enumStr, e))
    {
        SKR_LOG_ERROR("Unknown enumerator while reading enum ${enum.name}: %s", enumStr);
        return error_code::ENUMERATOR_ERROR;
    }
    return error_code::SUCCESS;
} 

void WriteTrait<const ${enum.name}&>::Write(skr_json_writer_t* writer, ${enum.name} e)
{
    writer->String(skr::type::enum_to_string(e));
} 
%endfor

%for record in generator.filter_types(db.records):
%if not generator.filter_debug_type(record):
error_code ReadTrait<${record.name}>::Read(value_t&& json, ${record.name}& record)
{
    ZoneScopedN("ReadTrait<${record.name}>::Read");

    %for base in record.bases:
    {
        auto baseJson = json;
        skr::json::Read(std::move(baseJson), (${base}&)record);
    }
    %endfor
    %for name, field in generator.filter_fields(record.fields):
    {
        constexpr const char8_t* kName = u8"${name}";
        constexpr const char8_t* kRecordName = u8"${record.name}";

        auto field = json["${name}"];
        if (field.error() == simdjson::NO_SUCH_FIELD)
        {
        %if hasattr(field.attrs, "no-default"):
            SKR_LOG_ERROR("Field ${name} in record ${record.name} not found while reading.");
            return (error_code)simdjson::NO_SUCH_FIELD;
        %else:
            _CODEGEN_LOG_DEFAULT_FIELD(kName, kRecordName);
        %endif
        }
        else if (field.error() != simdjson::SUCCESS)
        {
            _CODEGEN_FAILED_READ_RECORD(kRecordName, (const char8_t*)error_message((error_code)field.error()));
            return (error_code)field.error();
        }
        else
        {
            %if field.arraySize > 0:
            {

                auto array = field.get_array();
                if (array.error() != simdjson::SUCCESS)
                {
                    _CODEGEN_FAILED_READ_RECORD(u8"array ${record.name}", (const char8_t*)error_message((error_code)field.error()));
                    return (error_code)array.error();
                }
                size_t i = 0;
                for (auto element : array.value_unsafe())
                {
                    if(i > ${field.arraySize})
                    {
                        SKR_LOG_WARN("Array %s in record %s has too many elements", (const char*)kName, (const char*)kRecordName);
                        break;
                    }
                    if (element.error() != simdjson::SUCCESS)
                    {
                        _CODEGEN_FAILED_READ_FIELD_ARR_ELEM(kName, kRecordName, i, (const char8_t*)error_message((error_code)element.error()));
                        return (error_code)element.error();
                    }
                    error_code result = skr::json::Read(std::move(element).value_unsafe(), record.${name}[i]);
                    if(result != error_code::SUCCESS)
                    {
                        _CODEGEN_FAILED_READ_FIELD_ARR_ELEM(kName, kRecordName, i, (const char8_t*)error_message((error_code)element.error()));
                        return result;
                    }
                    ++i;
                }
                if(i < ${field.arraySize})
                {
                    SKR_LOG_WARN("Array ${name} in record ${record.name} has too few elements");
                }
            }
            %else:
            error_code result = skr::json::Read(std::move(field).value_unsafe(), (${field.type}&)record.${name});
            if(result != error_code::SUCCESS)
            {
                _CODEGEN_FAILED_READ_FIELD(kName, kRecordName, (const char8_t*)error_message((error_code)field.error()));
                return result;
            }
            %endif
        }
    }
    %endfor
    return error_code::SUCCESS;
} 
%endif
void WriteTrait<const ${record.name}&>::WriteFields(skr_json_writer_t* writer, const ${record.name}& record)
{
    %for base in record.bases:
    WriteTrait<const ${base}&>::WriteFields(writer, record);
    %endfor
    %for name, field in generator.filter_fields(record.fields):
    writer->Key(u8"${name}", ${len(name)});
    %if field.arraySize > 0:
    writer->StartArray();
    for(int i = 0; i < ${field.arraySize}; ++i)
        skr::json::Write<const ${field.type}&>(writer, record.${name}[i]);
    writer->EndArray();
    %else:
    skr::json::Write<const ${field.type}&>(writer, record.${name});
    %endif
    %endfor
} 
void WriteTrait<const ${record.name}&>::Write(skr_json_writer_t* writer, const ${record.name}& record)
{
    writer->StartObject();
    WriteFields(writer, record);
    writer->EndObject();
} 
%endfor
}
// END JSON IMPLEMENTATION