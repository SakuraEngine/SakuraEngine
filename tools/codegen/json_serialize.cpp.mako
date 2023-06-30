// BEGIN JSON IMPLEMENTATION
#include "misc/hash.h"
#include "platform/debug.h"
#include "misc/log.h"
#include "serde/json/reader.h"
#include "serde/json/writer.h"
#include "tracy/Tracy.hpp"
static const char* JsonArrayJsonFieldArchiveFailedFormat = "[SERDE/JSON] Archive %s.%s[%d] failed: %s";
static const char* JsonArrayFieldArchiveWarnFormat = "[SERDE/JSON] %s.%s got too many elements (%d expected, given %d), ignoring overflowed elements";
static const char* JsonFieldArchiveFailedFormat = "[SERDE/JSON] Archive %s.%s failed: %s";
static const char* JsonFieldNotFoundErrorFormat = "[SERDE/JSON] %s.%s not found";
static const char* JsonFieldNotFoundFormat = "[SERDE/JSON] %s.%s not found, using default value";

static const char* JsonArrayFieldNotEnoughErrorFormat = "[SERDE/JSON] %s.%s has too few elements (%d expected, given %d)";
static const char* JsonArrayFieldNotEnoughWarnFormat = "[SERDE/JSON] %s.%s got too few elements (%d expected, given %d), using default value";
static const char* JsonBaseArchiveFailedFormat = "[SERDE/JSON] Archive %s base %s failed: %d";

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
    ZoneScopedN("json::ReadTrait<${enum.name}>::Read");
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
    ZoneScopedN("json::WriteTrait<${enum.name}>::Write");
    writer->String(skr::type::enum_to_string(e));
} 
%endfor

%for record in generator.filter_types(db.records):
%if not generator.filter_debug_type(record):
error_code ReadTrait<${record.name}>::Read(value_t&& json, ${record.name}& record)
{
    ZoneScopedN("json::ReadTrait<${record.name}>::Read");
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
            SKR_LOG_ERROR(JsonFieldNotFoundErrorFormat, "${record.name}", "${name}");
            return (error_code)simdjson::NO_SUCH_FIELD;
        %else:
            SKR_LOG_WARN(JsonFieldNotFoundFormat, "${record.name}", "${name}");
        %endif
        }
        else if (field.error() != simdjson::SUCCESS)
        {
            SKR_LOG_ERROR(JsonFieldArchiveFailedFormat, "${record.name}", "${name}", error_message((error_code)field.error()));
            return (error_code)field.error();
        }
        else
        {
            %if field.arraySize > 0:
            {
                auto array = field.get_array();
                if (array.error() != simdjson::SUCCESS)
                {
                    SKR_LOG_ERROR(JsonFieldArchiveFailedFormat, "${record.name}", "${name}", error_message((error_code)field.error()));
                    return (error_code)array.error();
                }
                size_t i = 0;
                for (auto element : array.value_unsafe())
                {
                    if(i > ${field.arraySize})
                    {
                        SKR_LOG_WARN(JsonArrayFieldArchiveWarnFormat, "${record.name}", "${name}", ${field.arraySize}, i);
                        break;
                    }
                    if (element.error() != simdjson::SUCCESS)
                    {
                        SKR_LOG_ERROR(JsonArrayJsonFieldArchiveFailedFormat, "${record.name}", "${name}", i, error_message((error_code)element.error()));
                        return (error_code)element.error();
                    }
                    error_code result = skr::json::Read(std::move(element).value_unsafe(), record.${name}[i]);
                    if(result != error_code::SUCCESS)
                    {
                        SKR_LOG_ERROR(JsonArrayJsonFieldArchiveFailedFormat, "${record.name}", "${name}", i, error_message(result));
                        return result;
                    }
                    ++i;
                }
                if(i < ${field.arraySize})
                {
                    %if hasattr(field.attrs, "no-default"):
                        SKR_LOG_ERROR(JsonArrayFieldNotEnoughErrorFormat, "${record.name}", "${name}", ${field.arraySize}, i);
                        return (error_code)simdjson::NO_SUCH_FIELD;
                    %else:
                        SKR_LOG_WARN(JsonArrayFieldNotEnoughWarnFormat, "${record.name}", "${name}", ${field.arraySize}, i);
                    %endif
                }
            }
            %else:
            error_code result = skr::json::Read(std::move(field).value_unsafe(), (${field.type}&)record.${name});
            if(result != error_code::SUCCESS)
            {
                SKR_LOG_ERROR(JsonFieldArchiveFailedFormat, "${record.name}", "${name}", error_message((error_code)field.error()));
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
    ZoneScopedN("json::WriteTrait<${record.name}>::Write");
    writer->StartObject();
    WriteFields(writer, record);
    writer->EndObject();
} 
%endfor
}
// END JSON IMPLEMENTATION