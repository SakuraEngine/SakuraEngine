// BEGIN BINARY GENERATED
#include "SkrBase/misc/hash.h"
#include "SkrBase/misc/debug.h" 
#include "SkrCore/log.h"
#include "SkrSerde/binary/reader.h"
#include "SkrSerde/binary/writer.h"
#include "SkrProfile/profile.h"
#include "SkrSerde/blob.h"

[[maybe_unused]] static const char8_t* BinaryArrayBinaryFieldArchiveFailedFormat = u8"[SERDE/BIN] Failed to %s %s.%s[%d]: %d";
[[maybe_unused]] static const char8_t* BinaryFieldArchiveFailedFormat = u8"[SERDE/BIN] Failed to %s %s.%s: %d";
[[maybe_unused]] static const char8_t* BinaryBaseArchiveFailedFormat = u8"[SERDE/BIN] Failed to %s %s's base %s: %d";

namespace skr::binary {
%for record in generator.filter_types(db.records):
// binary serialize for ${record.name}
bool ReadTrait<${record.name}>::Read(SBinaryReader* archive, ${record.name}& record)
{
    SkrZoneScopedN("binary::ReadTrait<${record.name}>::Read");

    // serde bases
%for base in record.bases:
    if(!skr::binary::Read(archive, (${base}&)record))
    {
        SKR_LOG_ERROR(BinaryBaseArchiveFailedFormat, "Read", "${record.name}", "${base}", -1);
        return false;
    }
%endfor

    // serde self
%for name, field in generator.filter_fields(record.fields):
%if field.arraySize > 0:
    for(int i = 0; i < ${field.arraySize}; ++i)
    {
        if(!ReadTrait<${field.type}>::Read(archive, record.${name}[i]))
        {
            SKR_LOG_ERROR(BinaryArrayBinaryFieldArchiveFailedFormat, "Read", "${record.name}", "${name}", i, -1);
            return false;
        }
    }
%else:
    if(!ReadTrait<${field.type}>::Read(archive, record.${name}))
    {
        SKR_LOG_ERROR(BinaryFieldArchiveFailedFormat, "Read", "${record.name}", "${name}", -1);
        return false;
    }
%endif
%endfor

    return true;
}
bool WriteTrait<${record.name}>::Write(SBinaryWriter* archive, const ${record.name}& record)
{
    SkrZoneScopedN("binary::WriteTrait<${record.name}>::Write");

    // serde bases
%for base in record.bases:
    if(!skr::binary::Write<${base}>(archive, record))
    {
        SKR_LOG_ERROR(BinaryBaseArchiveFailedFormat, "Write", "${record.name}", "${base}", -1);
        return false;
    }
%endfor

    // serde self
%for name, field in generator.filter_fields(record.fields):
%if field.arraySize > 0:
    for(int i = 0; i < ${field.arraySize}; ++i)
    {
        if(!WriteTrait<${field.type}>::Write(archive, record.${name}[i]))
        {
            SKR_LOG_ERROR(BinaryArrayBinaryFieldArchiveFailedFormat, "Write", "${record.name}", "${name}", i, -1);
            return false;
        }
    }
%else:
    if(!WriteTrait<${field.type}>::Write(archive, record.${name}))
    {
        SKR_LOG_ERROR(BinaryFieldArchiveFailedFormat, "Write", "${record.name}", "${name}", -1);
        return false;
    }
%endif
%endfor

    return true;
}
%endfor
}
//END BINARY GENERATED