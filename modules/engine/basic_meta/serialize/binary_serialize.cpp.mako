// BEGIN BINARY GENERATED
#include "SkrBase/misc/hash.h"
#include "SkrBase/misc/debug.h" 
#include "SkrCore/log.h"
#include "SkrSerde/binary/reader.h"
#include "SkrSerde/binary/writer.h"
#include "SkrSerde/binary/blob.h"
#include "SkrProfile/profile.h"
#include "SkrSerde/blob.h"

[[maybe_unused]] static const char8_t* BinaryArrayBinaryFieldArchiveFailedFormat = u8"[SERDE/BIN] Failed to %s %s.%s[%d]: %d";
[[maybe_unused]] static const char8_t* BinaryFieldArchiveFailedFormat = u8"[SERDE/BIN] Failed to %s %s.%s: %d";
[[maybe_unused]] static const char8_t* BinaryBaseArchiveFailedFormat = u8"[SERDE/BIN] Failed to %s %s's base %s: %d";

namespace skr::binary {

%for record in generator.filter_types(db.records):
// archive for ${record.name}
template<class S>
bool __Archive(S* archive, ${record.name}& record)
{
    constexpr bool isWriter = std::is_same_v<S, SBinaryWriter>;
    const char* action = isWriter ? "Write" : "Read";
    
%for name, field in generator.filter_fields(record.fields):
%if field.arraySize > 0:
    for(int i = 0; i < ${field.arraySize}; ++i)
    {
        if(!Archive(archive, record.${name}[i]))
        {
            SKR_LOG_ERROR(BinaryArrayBinaryFieldArchiveFailedFormat, action, "${record.name}", "${name}", i, -1);
            return false;
        }
    }
%else:
    if(!Archive(archive, record.${name}))
    {
        SKR_LOG_ERROR(BinaryFieldArchiveFailedFormat, action, "${record.name}", "${name}", -1);
        return false;
    }
%endif
%endfor
    return true;
}

// reader for ${record.name}
bool ReadTrait<${record.name}>::Read(SBinaryReader* archive, ${record.name}& record)
{
    SkrZoneScopedN("binary::ReadTrait<${record.name}>::Read");
%for base in record.bases:
    if(!skr::binary::Read(archive, (${base}&)record))
    {
        SKR_LOG_ERROR(BinaryBaseArchiveFailedFormat, "Read", "${record.name}", "${base}", -1);
        return false;
    }
%endfor
    return __Archive(archive, record);
}

// writter for ${record.name}
bool WriteTrait<${record.name}>::Write(SBinaryWriter* archive, const ${record.name}& record)
{
    SkrZoneScopedN("binary::WriteTrait<${record.name}>::Write");
%for base in record.bases:
    if(!skr::binary::Write<${base}>(archive, record))
    {
        SKR_LOG_ERROR(BinaryBaseArchiveFailedFormat, "Write", "${record.name}", "${base}", -1);
        return false;
    }
%endfor
    return __Archive(archive, (${record.name}&)record);
}
%endfor
}
//END BINARY GENERATED