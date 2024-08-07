// BEGIN JSON IMPLEMENTATION
#include "SkrBase/misc/hash.h"
#include "SkrBase/misc/debug.h" 
#include "SkrCore/log.h"
#include "SkrProfile/profile.h"

[[maybe_unused]] static const char8_t* JsonArrayJsonFieldArchiveFailedFormat = u8"[SERDE/JSON] Archive %s.%s[%d] failed: %s";
[[maybe_unused]] static const char8_t* JsonArrayFieldArchiveWarnFormat = u8"[SERDE/JSON] %s.%s got too many elements (%d expected, given %d), ignoring overflowed elements";
[[maybe_unused]] static const char8_t* JsonFieldArchiveFailedFormat = u8"[SERDE/JSON] Archive %s.%s failed: %s";
[[maybe_unused]] static const char8_t* JsonFieldNotFoundErrorFormat = u8"[SERDE/JSON] %s.%s not found";
[[maybe_unused]] static const char8_t* JsonFieldNotFoundFormat = u8"[SERDE/JSON] %s.%s not found, using default value";

[[maybe_unused]] static const char8_t* JsonArrayFieldNotEnoughErrorFormat = u8"[SERDE/JSON] %s.%s has too few elements (%d expected, given %d)";
[[maybe_unused]] static const char8_t* JsonArrayFieldNotEnoughWarnFormat = u8"[SERDE/JSON] %s.%s got too few elements (%d expected, given %d), using default value";
[[maybe_unused]] static const char8_t* JsonBaseArchiveFailedFormat = u8"[SERDE/JSON] Archive %s base %s failed: %d";

namespace skr 
{
%for enum in generator.filter_types(db.enums):
// enum serde traits
skr::StringView EnumSerdeTraits<${enum.name}>::to_string(const ${enum.name}& value)
{
    switch (value)
    {
%for enum_item_name, enum_value in vars(enum.values).items():
    case ${enum.name}::${db.short_name(enum_item_name)}: return u8"${enum_item_name}";
%endfor
    default: SKR_UNREACHABLE_CODE(); return u8"${enum.name}::INVALID_ENUMERATOR";
    }
}
bool EnumSerdeTraits<${enum.name}>::from_string(skr::StringView str, ${enum.name}& value)
{
    const auto hash = skr_hash64(str.raw().data(), str.size(), 0);
    switch(hash)
    {
%for enum_item_name, enum_value in vars(enum.values).items():
        case skr::consteval_hash(u8"${db.short_name(enum_item_name)}"): if(str == u8"${db.short_name(enum_item_name)}") value = ${enum_item_name}; return true;
%endfor
        default:
            return false;
    }
}
%endfor

%for record in generator.filter_types(db.records):
bool JsonSerde<${record.name}>::read_fields(skr::archive::JsonReader* r, ${record.name}& v)
{
    SkrZoneScopedN("json::JsonSerde<${record.name}>::read_fields");
    
    // read bases
%for base in record.bases:
    {
        auto baseJson = r;
        JsonSerde<${base}>::read_fields(baseJson, (${base}&)v);
    }
%endfor

    // read self fields
%for name, field in generator.filter_fields(record.fields):
<% field_type = f"{field.type}[{field.arraySize}]" if field.arraySize else field.type %>\
    {
        auto jSlot = r->Key(u8"${name}");
        jSlot.error_then([&](auto e){
            SKR_ASSERT(e == skr::archive::JsonReadError::KeyNotFound);
        });
        if (jSlot.has_value())
        {
            if(!json_read<${field_type}>(r, v.${name}))
            {
                SKR_LOG_ERROR(JsonFieldArchiveFailedFormat, "${record.name}", "${name}", "UNKNOWN ERROR");  // TODO: ERROR MESSAGE
                return false;
            }
        }
    }
%endfor
    return true;
} 
bool JsonSerde<${record.name}>::write_fields(skr::archive::JsonWriter* w, const ${record.name}& v)
{
    // write bases
%for base in record.bases:
    if (!JsonSerde<${base}>::write_fields(w, v)) return false;
%endfor

    // write self fields
%for name, field in generator.filter_fields(record.fields):
<% field_type = f"{field.type}[{field.arraySize}]" if field.arraySize else field.type %>\
    
    SKR_EXPECTED_CHECK(w->Key(u8"${name}"), false);
    if (!json_write<${field_type}>(w, v.${name})) return false;
%endfor
    return true;
} 
bool JsonSerde<${record.name}>::read(skr::archive::JsonReader* r, ${record.name}& v)
{
    SkrZoneScopedN("JsonSerde<${record.name}>::write");
    SKR_EXPECTED_CHECK(r->StartObject(), false);
    if (!read_fields(r, v)) return false;
    SKR_EXPECTED_CHECK(r->EndObject(), false);
    return true;
}
bool JsonSerde<${record.name}>::write(skr::archive::JsonWriter* w, const ${record.name}& v)
{
    SkrZoneScopedN("JsonSerde<${record.name}>::write");
    SKR_EXPECTED_CHECK(w->StartObject(), false);
    if (!write_fields(w, v)) return false;
    SKR_EXPECTED_CHECK(w->EndObject(), false);
    return true;
} 
%endfor
}
// END JSON IMPLEMENTATION