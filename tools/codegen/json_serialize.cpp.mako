// BEGIN JSON IMPLEMENTATION
#include "utils/hash.h"
#include "platform/debug.h"
#include "utils/log.h"

namespace skr::json {
%for enum in generator.filter_types(db.enums):
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
    %for name, value in vars(enum.values).items():
        case hash_crc32<char>("${db.short_name(name)}"): if( enumStr == "${db.short_name(name)}") e = ${name}; return error_code::SUCCESS;
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
    %for name, value in vars(enum.values).items():
        case ${name}: writer->String("${db.short_name(name)}", ${len(db.short_name(name))}); return;
    %endfor
    }
} 
%endfor

%for record in generator.filter_types(db.records):
template<>
error_code ReadValue(simdjson::ondemand::value&& json, ${record.name}& record)
{
    %for base in record.bases:
    {
        auto baseJson = json;
        ReadValue(std::move(baseJson), (${base}&)record);
    }
    %endfor
    %for name, field in generator.filter_fields(record.fields):
    {
        auto field = json["${name}"];
        if (field.error() == simdjson::NO_SUCH_FIELD)
        {
            SKR_LOG_ERROR("Field ${name} in record ${record.name} not found while reading");
        }
        else if (field.error() != simdjson::SUCCESS)
        {
            SKR_LOG_ERROR("Failed to read record ${record.name} %s", error_message((error_code)field.error()));
            return (error_code)field.error();
        }
        else
        {
            %if field.arraySize > 0:
            {
                auto array = field.get_array();
                if (array.error() != simdjson::SUCCESS)
                {
                    SKR_LOG_ERROR("Failed to read array ${name} in record ${record.name} %s", error_message((error_code)array.error()));
                    return (error_code)array.error();
                }
                size_t i = 0;
                for (auto element : array.value_unsafe())
                {
                    if(i > ${field.arraySize})
                    {
                        SKR_LOG_WARN("Array ${name} in record ${record.name} has too many elements");
                        break;
                    }
                    if (element.error() != simdjson::SUCCESS)
                    {
                        SKR_LOG_ERROR("Failed to read field ${name} array element %lld in record ${record.name}", i);
                        return (error_code)element.error();
                    }
                    error_code result = skr::json::Read(std::move(element).value_unsafe(), record.${name}[i]);
                    if(result != error_code::SUCCESS)
                    {
                        SKR_LOG_ERROR("Failed to read field ${name} array element %lld in record ${record.name}", i);
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
                SKR_LOG_ERROR("Failed to read field ${name} of record ${record.name} %s", error_message(result));
                return result;
            }
            %endif
        }
    }
    %endfor
    return error_code::SUCCESS;
} 
template<>
void WriteFields(skr_json_writer_t* writer, const ${record.name}& record)
{
    %for base in record.bases:
    WriteFields<const ${base}&>(writer, record);
    %endfor
    %for name, field in vars(record.fields).items():
    writer->Key("${name}", ${len(name)});
    %if field.arraySize > 0:
    for(int i = 0; i < ${field.arraySize}; ++i)
        skr::json::Write<skr::json::TParamType<${field.type}>>(writer, record.${name}[i]);
    %else:
    skr::json::Write<skr::json::TParamType<${field.type}>>(writer, record.${name});
    %endif
    %endfor
} 
template<>
void WriteValue(skr_json_writer_t* writer, const ${record.name}& record)
{
    writer->StartObject();
    WriteFields<const ${record.name}&>(writer, record);
    writer->EndObject();
} 
%endfor
}
// END JSON IMPLEMENTATION