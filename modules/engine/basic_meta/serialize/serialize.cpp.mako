// BEGIN SERIALIZE GENERATED
#include "SkrProfile/profile.h"

// debug
[[maybe_unused]] static const char8_t* JsonArrayJsonFieldArchiveFailedFormat = u8"[SERDE/JSON] Archive %s.%s[%d] failed: %s";
[[maybe_unused]] static const char8_t* JsonArrayFieldArchiveWarnFormat = u8"[SERDE/JSON] %s.%s got too many elements (%d expected, given %d), ignoring overflowed elements";
[[maybe_unused]] static const char8_t* JsonFieldArchiveFailedFormat = u8"[SERDE/JSON] Archive %s.%s failed: %s";
[[maybe_unused]] static const char8_t* JsonFieldNotFoundErrorFormat = u8"[SERDE/JSON] %s.%s not found";
[[maybe_unused]] static const char8_t* JsonFieldNotFoundFormat = u8"[SERDE/JSON] %s.%s not found, using default value";

[[maybe_unused]] static const char8_t* JsonArrayFieldNotEnoughErrorFormat = u8"[SERDE/JSON] %s.%s has too few elements (%d expected, given %d)";
[[maybe_unused]] static const char8_t* JsonArrayFieldNotEnoughWarnFormat = u8"[SERDE/JSON] %s.%s got too few elements (%d expected, given %d), using default value";
[[maybe_unused]] static const char8_t* JsonBaseArchiveFailedFormat = u8"[SERDE/JSON] Archive %s base %s failed: %d";

[[maybe_unused]] static const char8_t* BinaryArrayBinaryFieldArchiveFailedFormat = u8"[SERDE/BIN] Failed to %s %s.%s[%d]: %d";
[[maybe_unused]] static const char8_t* BinaryFieldArchiveFailedFormat = u8"[SERDE/BIN] Failed to %s %s.%s: %d";
[[maybe_unused]] static const char8_t* BinaryBaseArchiveFailedFormat = u8"[SERDE/BIN] Failed to %s %s's base %s: %d";

// json serde
namespace skr
{
// enum serde traits
%for enum in json_enums:
skr::StringView EnumSerdeTraits<${enum.name}>::to_string(const ${enum.name}& value)
{
    switch (value)
    {
%for enum_value in enum.values.values():
    case ${enum.name}::${enum_value.short_name}: return u8"${enum_value.name}";
%endfor
    default: SKR_UNREACHABLE_CODE(); return u8"${enum.name}::__INVALID_ENUMERATOR__";
    }
}
bool EnumSerdeTraits<${enum.name}>::from_string(skr::StringView str, ${enum.name}& value)
{
    const auto hash = skr_hash64(str.raw().data(), str.size(), 0);
    switch(hash)
    {
%for enum_value in enum.values.values():
    case skr::consteval_hash(u8"${enum_value.short_name}"): if(str == u8"${enum_value.short_name}") value = ${enum_value.name}; return true;
%endfor
    default: return false;
    }
}
%endfor

// record serde
%for record in json_records:
<% record_serde_data = record.generator_data["serde"] %>\
bool JsonSerde<${record.name}>::read_fields(skr::archive::JsonReader* r, ${record.name}& v)
{
    SkrZoneScopedN("JsonSerde<${record.name}>::read_fields");
    
    // read bases
%for base in record.bases:
    JsonSerde<${base}>::read_fields(r, v);
%endfor

    // read self fields
%for field in record_serde_data.json_fields:
<% field_type = f"{field.type}[{field.array_size}]" if field.array_size else field.type %>\
    {
        auto jSlot = r->Key(u8"${field.name}");
        jSlot.error_then([&](auto e){
            SKR_ASSERT(e == skr::archive::JsonReadError::KeyNotFound);
        });
        if (jSlot.has_value())
        {
            if(!json_read<${field_type}>(r, v.${field.name}))
            {
                SKR_LOG_ERROR(JsonFieldArchiveFailedFormat, "${record.name}", "${field.name}", "UNKNOWN ERROR");  // TODO: ERROR MESSAGE
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
%for field in record_serde_data.json_fields:
<% field_type = f"{field.type}[{field.array_size}]" if field.array_size else field.type %>\
    
    SKR_EXPECTED_CHECK(w->Key(u8"${field.name}"), false);
    if (!json_write<${field_type}>(w, v.${field.name})) return false;
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

// bin serde
namespace skr
{
%for record in bin_records:
<% record_serde_data = record.generator_data["serde"] %>\
bool BinSerde<${record.name}>::read(SBinaryReader* r, ${record.name}& v)
{
    SkrZoneScopedN("BinSerde<${record.name}>::read");

    // serde bases
%for base in record.bases:
    if(!bin_read<${base}>(r, v))
    {
        SKR_LOG_ERROR(BinaryBaseArchiveFailedFormat, "Read", "${record.name}", "${base}", -1);
        return false;
    }
%endfor

    // serde self
%for field in record_serde_data.bin_fields:
<% field_type = f"{field.type}[{field.array_size}]" if field.array_size else field.type %>\
    if(!bin_read<${field_type}>(r, v.${field.name}))
    {
        SKR_LOG_ERROR(BinaryFieldArchiveFailedFormat, "Read", "${record.name}", "${field.name}", -1);
        return false;
    }
%endfor

    return true;
}
bool BinSerde<${record.name}>::write(SBinaryWriter* w, const ${record.name}& v)
{
    SkrZoneScopedN("BinSerde<${record.name}>::write");

    // serde bases
%for base in record.bases:
    if(!bin_write<${base}>(w, v))
    {
        SKR_LOG_ERROR(BinaryBaseArchiveFailedFormat, "Write", "${record.name}", "${base}", -1);
        return false;
    }
%endfor

    // serde self
%for field in record_serde_data.bin_fields:
<% field_type = f"{field.type}[{field.array_size}]" if field.array_size else field.type %>\
    if(!bin_write<${field_type}>(w, v.${field.name}))
    {
        SKR_LOG_ERROR(BinaryFieldArchiveFailedFormat, "Write", "${record.name}", "${field.name}", -1);
        return false;
    }
%endfor

    return true;
}
%endfor
}
// END SERIALIZE GENERATED