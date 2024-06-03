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

<%def name="archive_field(name, field, array, cfg)">
%if hasattr(field.attrs, "arena"):
    ArchiveBlob(archive, arena_${field.attrs.arena}, record.${name}${array}${cfg})
%else:
    Archive(archive, record.${name}${array}${cfg})
%endif
</%def>

namespace skr::binary {

%for record in generator.filter_types(db.records):
<% configParam = ", " + record.attrs.serialize_config if hasattr(record.attrs, "serialize_config") else ""%>
<% configArg = ", " + record.attrs.serialize_config.split(" ")[1] if hasattr(record.attrs, "serialize_config") else ""%>
%if generator.filter_blob_type(record):
template<class S>
bool __Archive(S* archive, skr_blob_arena_t& arena, ${record.name}& record${configParam})
{
    constexpr bool isWriter = std::is_same_v<S, SBinaryWriter>;
    const char* action = isWriter ? "Write" : "Read";
    %for name, field in generator.filter_fields(record.fields):
    <% fieldConfigArg = ", " + field.attrs.serialize_config if hasattr(field.attrs, "serialize_config") else ""%>
    %if field.arraySize > 0:
    for(int i = 0; i < ${field.arraySize}; ++i)
    {
        if(!ArchiveBlob(archive, arena, record.${name}[i]${fieldConfigArg}))
        {
            SKR_LOG_ERROR(BinaryArrayBinaryFieldArchiveFailedFormat, action, "${record.name}", "${name}", i, -1);
            return true;
        }
    }
    %else:
    if(!ArchiveBlob(archive, arena, record.${name}${fieldConfigArg}))
    {
        SKR_LOG_ERROR(BinaryFieldArchiveFailedFormat, action, "${record.name}", "${name}", -1);
        return false;
    }
    %endif
    %endfor
    return true;
}
%else:
template<class S>
bool __Archive(S* archive, ${record.name}& record${configParam})
{
    constexpr bool isWriter = std::is_same_v<S, SBinaryWriter>;
    const char* action = isWriter ? "Write" : "Read";
    %for name, field in generator.filter_fields(record.fields):
    <% fieldConfigArg = ", " + field.attrs.serialize_config if hasattr(field.attrs, "serialize_config") else ""%>
    %if field.type == "skr_blob_arena_t":
    auto& arena_${name} = record.${name};
    if(!Archive(archive, arena_${name}))
    {
        SKR_LOG_ERROR(BinaryFieldArchiveFailedFormat, action, "${record.name}", "${name}", -1);
        return false;
    }
    %elif field.arraySize > 0:
    for(int i = 0; i < ${field.arraySize}; ++i)
    {
        if(!${archive_field(name, field, "[i]", fieldConfigArg)})
        {
            SKR_LOG_ERROR(BinaryArrayBinaryFieldArchiveFailedFormat, action, "${record.name}", "${name}", i, -1);
            return false;
        }
    }
    %else:
    if(!${archive_field(name, field, "", fieldConfigArg)})
    {
        SKR_LOG_ERROR(BinaryFieldArchiveFailedFormat, action, "${record.name}", "${name}", -1);
        return false;
    }
    %endif
    %endfor
    return true;
}
%endif

%if generator.filter_blob_type(record):
void BlobTrait<${record.name}>::BuildArena(skr_blob_arena_builder_t& arena, ${record.name}& dst, const ${record.name}Builder& src)
{
    SkrZoneScopedN("binary::BlobTrait<${record.name}>::BuildArena");
%for base in record.bases:
    BlobTrait<${base}>::BuildArena(arena, (${base}&)dst, (${base}Builder&) src);
%endfor
%for name, field in generator.filter_fields(record.fields):
%if field.arraySize > 0:
    for(int i = 0; i < ${field.arraySize}; ++i)
    {
        ::skr::binary::BuildArena<${field.type}>(arena, dst.${name}[i], src.${name}[i]);
    }
%else:
    ::skr::binary::BuildArena<${field.type}>(arena, dst.${name}, src.${name});
%endif
%endfor
}
void BlobTrait<${record.name}>::Remap(skr_blob_arena_t& arena, ${record.name}& dst)
{
    SkrZoneScopedN("binary::BlobTrait<${record.name}>::Remap");
%for base in record.bases:
    BlobTrait<${base}>::Remap(arena, (${base}&)dst);
%endfor
%for name, field in generator.filter_fields(record.fields):
%if field.arraySize > 0:
    for(int i = 0; i < ${field.arraySize}; ++i)
    {
        ::skr::binary::Remap<${field.type}>(arena, dst.${name}[i]);
    }
%else:
    ::skr::binary::Remap<${field.type}>(arena, dst.${name});
%endif
%endfor
}
bool ReadTrait<${record.name}>::Read(SBinaryReader* archive, skr_blob_arena_t& arena, ${record.name}& record${configParam})
{
    SkrZoneScopedN("binary::ReadTrait<${record.name}>::Read");
%for base in record.bases:
    if(!ReadTrait<const ${base}&>::Read(archive, arena, (${base}&)record))
    {
        SKR_LOG_ERROR(BinaryBaseArchiveFailedFormat, "Read", "${record.name}", "${base}", -1);
        return false;
    }
%endfor
    return __Archive(archive, arena, record${configArg});
}
bool WriteTrait<${record.name}>::Write(SBinaryWriter* archive, skr_blob_arena_t& arena, const ${record.name}& record${configParam})
{
    SkrZoneScopedN("binary::WriteTrait<${record.name}>::Write");
%for base in record.bases:
    if(!WriteTrait<${base}>::Write(archive, arena, (${base}&)record))
    {
        SKR_LOG_ERROR(BinaryBaseArchiveFailedFormat, "Write", "${record.name}", "${base}", -1);
        return false;
    }
%endfor
    return __Archive(archive, arena, (${record.name}&)record${configArg});
} 
%else:
bool ReadTrait<${record.name}>::Read(SBinaryReader* archive, ${record.name}& record${configParam})
{
    SkrZoneScopedN("binary::ReadTrait<${record.name}>::Read");
%for base in record.bases:
    if(!skr::binary::Read(archive, (${base}&)record))
    {
        SKR_LOG_ERROR(BinaryBaseArchiveFailedFormat, "Read", "${record.name}", "${base}", -1);
        return false;
    }
%endfor
    return __Archive(archive, record${configArg});
}
bool WriteTrait<${record.name}>::Write(SBinaryWriter* archive, const ${record.name}& record${configParam})
{
    SkrZoneScopedN("binary::WriteTrait<${record.name}>::Write");
%for base in record.bases:
    if(!skr::binary::Write<${base}>(archive, record))
    {
        SKR_LOG_ERROR(BinaryBaseArchiveFailedFormat, "Write", "${record.name}", "${base}", -1);
        return false;
    }
%endfor
    return __Archive(archive, (${record.name}&)record${configArg});
} 
%endif
%endfor
}
//END BINARY GENERATED