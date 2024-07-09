// BEGIN JSON IMPLEMENTATION
#include "SkrBase/misc/hash.h"
#include "SkrBase/misc/debug.h" 
#include "SkrCore/log.h"
#include "SkrSerde/json/reader.h"
#include "SkrSerde/json/writer.h"
#include "SkrProfile/profile.h"

[[maybe_unused]] static const char8_t* JsonArrayJsonFieldArchiveFailedFormat = u8"[SERDE/JSON] Archive %s.%s[%d] failed: %s";
[[maybe_unused]] static const char8_t* JsonArrayFieldArchiveWarnFormat = u8"[SERDE/JSON] %s.%s got too many elements (%d expected, given %d), ignoring overflowed elements";
[[maybe_unused]] static const char8_t* JsonFieldArchiveFailedFormat = u8"[SERDE/JSON] Archive %s.%s failed: %s";
[[maybe_unused]] static const char8_t* JsonFieldNotFoundErrorFormat = u8"[SERDE/JSON] %s.%s not found";
[[maybe_unused]] static const char8_t* JsonFieldNotFoundFormat = u8"[SERDE/JSON] %s.%s not found, using default value";

[[maybe_unused]] static const char8_t* JsonArrayFieldNotEnoughErrorFormat = u8"[SERDE/JSON] %s.%s has too few elements (%d expected, given %d)";
[[maybe_unused]] static const char8_t* JsonArrayFieldNotEnoughWarnFormat = u8"[SERDE/JSON] %s.%s got too few elements (%d expected, given %d), using default value";
[[maybe_unused]] static const char8_t* JsonBaseArchiveFailedFormat = u8"[SERDE/JSON] Archive %s base %s failed: %d";

#define TRUE_OR_RETURN_FALSE(x) if (!(x)) return false;

namespace skr::json {
%for enum in generator.filter_types(db.enums):
bool ReadTrait<${enum.name}>::Read(skr::archive::JsonReader* json, ${enum.name}& e)
{
    SkrZoneScopedN("json::ReadTrait<${enum.name}>::Read");
    skr::String enumStr;
    if (json->String(enumStr).has_value())
    {
        if(!skr::rttr::EnumTraits<${enum.name}>::from_string(enumStr.view(), e))
        {
            SKR_LOG_ERROR(u8"Unknown enumerator while reading enum ${enum.name}: %s", enumStr.raw().data());
            return false;
        }
        return true;
    }
    return false;
} 

bool WriteTrait<${enum.name}>::Write(skr::archive::JsonWriter* writer, ${enum.name} e)
{
    SkrZoneScopedN("json::WriteTrait<${enum.name}>::Write");
    return writer->String(skr::rttr::EnumTraits<${enum.name}>::to_string(e)).has_value();
} 
%endfor

%for record in generator.filter_types(db.records):
%if not generator.filter_debug_type(record):
bool ReadTrait<${record.name}>::ReadFields(skr::archive::JsonReader* json, ${record.name}& record)
{
    SkrZoneScopedN("json::ReadTrait<${record.name}>::Read");
    %for base in record.bases:
    {
        auto baseJson = json;
        ReadTrait<${base}>::ReadFields(baseJson, (${base}&)record);
    }
    %endfor
    %for name, field in generator.filter_fields(record.fields):
    {
        auto jSlot = json->Key(u8"${name}");
        jSlot.error_then([&](auto e){
            SKR_ASSERT(e == skr::archive::JsonReadError::KeyNotFound);
        });
        if (jSlot.has_value())
        {
            %if field.arraySize > 0:
            {
                size_t count;
                json->StartArray(count);
                size_t i = 0;
                for (i = 0; i < count; i++)
                {
                    if(i > ${field.arraySize})
                    {
                        SKR_LOG_WARN(JsonArrayFieldArchiveWarnFormat, "${record.name}", "${name}", ${field.arraySize}, i);
                        break;
                    }
                    if(!skr::json::Read(json, record.${name}[i]))
                    {
                        SKR_LOG_ERROR(JsonArrayJsonFieldArchiveFailedFormat, "${record.name}", "${name}", i, "UNKNOWN ERROR"); // TODO: ERROR MESSAGE
                        return false;
                    }
                }
                json->EndArray();    

                if(i < ${field.arraySize})
                {
                    %if hasattr(field.attrs, "no-default"):
                        SKR_LOG_ERROR(JsonArrayFieldNotEnoughErrorFormat, "${record.name}", "${name}", ${field.arraySize}, i);
                        return false;
                    %else:
                        SKR_LOG_WARN(JsonArrayFieldNotEnoughWarnFormat, "${record.name}", "${name}", ${field.arraySize}, i);
                    %endif
                }
            }
            %else:
            if(!skr::json::Read(json, (${field.type}&)record.${name}))
            {
                SKR_LOG_ERROR(JsonFieldArchiveFailedFormat, "${record.name}", "${name}", "UNKNOWN ERROR");  // TODO: ERROR MESSAGE
                return false;
            }
            %endif
        }
    }
    %endfor
    return true;
} 

bool ReadTrait<${record.name}>::Read(skr::archive::JsonReader* reader, ${record.name}& record)
{
    SkrZoneScopedN("json::WriteTrait<${record.name}>::Write");
    TRUE_OR_RETURN_FALSE(reader->StartObject().has_value());
    TRUE_OR_RETURN_FALSE(ReadFields(reader, record));
    TRUE_OR_RETURN_FALSE(reader->EndObject().has_value());
    return true;
} 
%endif

bool WriteTrait<${record.name}>::WriteFields(skr::archive::JsonWriter* writer, const ${record.name}& record)
{
    %for base in record.bases:
    TRUE_OR_RETURN_FALSE(WriteTrait<${base}>::WriteFields(writer, record));
    %endfor
    %for name, field in generator.filter_fields(record.fields):
    TRUE_OR_RETURN_FALSE(writer->Key(u8"${name}").has_value());
    %if field.arraySize > 0:
    TRUE_OR_RETURN_FALSE(writer->StartArray().has_value());
    for(int i = 0; i < ${field.arraySize}; ++i)
    {
        bool _x = skr::json::Write<${field.type}>(writer, record.${name}[i]);
        TRUE_OR_RETURN_FALSE(_x);
    }
    TRUE_OR_RETURN_FALSE(writer->EndArray().has_value());
    %else:
    {
        bool _x = skr::json::Write<${field.type}>(writer, record.${name});
        TRUE_OR_RETURN_FALSE(_x);
    }
    %endif
    %endfor
    return true;
} 

bool WriteTrait<${record.name}>::Write(skr::archive::JsonWriter* writer, const ${record.name}& record)
{
    SkrZoneScopedN("json::WriteTrait<${record.name}>::Write");
    TRUE_OR_RETURN_FALSE(writer->StartObject().has_value());
    TRUE_OR_RETURN_FALSE(WriteFields(writer, record));
    TRUE_OR_RETURN_FALSE(writer->EndObject().has_value());
    return true;
} 
%endfor
}
// END JSON IMPLEMENTATION

#undef TRUE_OR_RETURN_FALSE