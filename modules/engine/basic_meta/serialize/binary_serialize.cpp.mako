// BEGIN BINARY GENERATED
#include "SkrBase/misc/hash.h"
#include "SkrBase/misc/debug.h" 
#include "SkrCore/log.h"
#include "SkrProfile/profile.h"

[[maybe_unused]] static const char8_t* BinaryArrayBinaryFieldArchiveFailedFormat = u8"[SERDE/BIN] Failed to %s %s.%s[%d]: %d";
[[maybe_unused]] static const char8_t* BinaryFieldArchiveFailedFormat = u8"[SERDE/BIN] Failed to %s %s.%s: %d";
[[maybe_unused]] static const char8_t* BinaryBaseArchiveFailedFormat = u8"[SERDE/BIN] Failed to %s %s's base %s: %d";

namespace skr {
%for record in generator.filter_types(db.records):
// binary serialize for ${record.name}
bool BinSerde<${record.name}>::read(SBinaryReader* r, ${record.name}& v)
{
    SkrZoneScopedN("binary::BinSerde<${record.name}>::read");

    // serde bases
%for base in record.bases:
    if(!bin_read<${base}>(r, v))
    {
        SKR_LOG_ERROR(BinaryBaseArchiveFailedFormat, "Read", "${record.name}", "${base}", -1);
        return false;
    }
%endfor

    // serde self
%for name, field in generator.filter_fields(record.fields):
<% field_type = f"{field.type}[{field.arraySize}]" if field.arraySize else field.type %>\
    if(!bin_read<${field_type}>(r, v.${name}))
    {
        SKR_LOG_ERROR(BinaryFieldArchiveFailedFormat, "Read", "${record.name}", "${name}", -1);
        return false;
    }
%endfor

    return true;
}
bool BinSerde<${record.name}>::write(SBinaryWriter* w, const ${record.name}& v)
{
    SkrZoneScopedN("binary::WriteTrait<${record.name}>::Write");

    // serde bases
%for base in record.bases:
    if(!bin_write<${base}>(w, v))
    {
        SKR_LOG_ERROR(BinaryBaseArchiveFailedFormat, "Write", "${record.name}", "${base}", -1);
        return false;
    }
%endfor

    // serde self
%for name, field in generator.filter_fields(record.fields):
<% field_type = f"{field.type}[{field.arraySize}]" if field.arraySize else field.type %>\
    if(!bin_write<${field_type}>(w, v.${name}))
    {
        SKR_LOG_ERROR(BinaryFieldArchiveFailedFormat, "Write", "${record.name}", "${name}", -1);
        return false;
    }
%endfor

    return true;
}
%endfor
}
//END BINARY GENERATED