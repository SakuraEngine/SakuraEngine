// BEGIN JSON IMPLEMENTATION
#include "utils/hash.h"
#include "platform/debug.h"
#include "utils/log.h"
#include "json/reader.h"
#include "json/writer.h"

namespace skr::json {
%for enum in generator.filter_types(db.enums):
error_code ReadHelper<${enum.name}>::Read(value_t&& json, ${enum.name}& e)
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

void WriteHelper<const ${enum.name}&>::Write(skr_json_writer_t* writer, ${enum.name} e)
{
    switch(e)
    {
    %for name, value in vars(enum.values).items():
        case ${name}: writer->String("${db.short_name(name)}", ${len(db.short_name(name))}); return;
    %endfor
        default: writer->String("INVALID_ENUMERATOR", ${len("INVALID_ENUMERATOR")}); 
        SKR_UNREACHABLE_CODE(); return;
    }
    SKR_UNREACHABLE_CODE();
} 
%endfor

%for record in generator.filter_types(db.records):
error_code ReadHelper<${record.name}>::Read(value_t&& json, ${record.name}& record)
{
    %for base in record.bases:
    {
        auto baseJson = json;
        skr::json::Read(std::move(baseJson), (${base}&)record);
    }
    %endfor
    %for name, field in generator.filter_fields(record.fields):
    {
        auto field = json["${name}"];
        if (field.error() == simdjson::NO_SUCH_FIELD)
        {
        %if hasattr(field.attrs, "no-default"):
            SKR_LOG_ERROR("Field ${name} in record ${record.name} not found while reading.");
            return (error_code)simdjson::NO_SUCH_FIELD;
        %else:
            SKR_LOG_TRACE("Field ${name} in record ${record.name} not found while reading, using default value.");
        %endif
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
void WriteHelper<const ${record.name}&>::WriteFields(skr_json_writer_t* writer, const ${record.name}& record)
{
    %for base in record.bases:
    WriteHelper<const ${base}&>::WriteFields(writer, record);
    %endfor
    %for name, field in generator.filter_fields(record.fields):
    writer->Key("${name}", ${len(name)});
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
void WriteHelper<const ${record.name}&>::Write(skr_json_writer_t* writer, const ${record.name}& record)
{
    writer->StartObject();
    WriteFields(writer, record);
    writer->EndObject();
} 
%endfor
}
// END JSON IMPLEMENTATION