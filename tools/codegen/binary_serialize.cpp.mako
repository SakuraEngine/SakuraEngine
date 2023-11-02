// BEGIN BINARY GENERATED
#include "SkrRT/misc/hash.h"
#include "SkrRT/platform/debug.h"
#include "SkrRT/misc/log.h"
#include "SkrRT/serde/binary/reader.h"
#include "SkrRT/serde/binary/writer.h"
#include "SkrRT/serde/binary/blob.h"
#include "SkrProfile/profile.h"

[[maybe_unused]] static const char8_t* BinaryArrayBinaryFieldArchiveFailedFormat = u8"[SERDE/BIN] Failed to %s %s.%s[%d]: %d";
[[maybe_unused]] static const char8_t* BinaryFieldArchiveFailedFormat = u8"[SERDE/BIN] Failed to %s %s.%s: %d";
[[maybe_unused]] static const char8_t* BinaryBaseArchiveFailedFormat = u8"[SERDE/BIN] Failed to %s %s's base %s: %d";

<%def name="archive_field(name, field, array, cfg)">
%if hasattr(field.attrs, "arena"):
    ret = ArchiveBlob(archive, arena_${field.attrs.arena}, record.${name}${array}${cfg});
%else:
    ret = Archive(archive, record.${name}${array}${cfg});
%endif
</%def>

namespace skr::binary {

%for record in generator.filter_types(db.records):
<% configParam = ", " + record.attrs.serialize_config if hasattr(record.attrs, "serialize_config") else ""%>
<% configArg = ", " + record.attrs.serialize_config.split(" ")[1] if hasattr(record.attrs, "serialize_config") else ""%>
%if generator.filter_blob_type(record):
template<class S>
int __Archive(S* archive, skr_blob_arena_t& arena, ${record.name}& record${configParam})
{
    constexpr bool isWriter = std::is_same_v<S, skr_binary_writer_t>;
    const char* action = isWriter ? "Write" : "Read";
    int ret = 0;
    %for name, field in generator.filter_fields(record.fields):
    <% fieldConfigArg = ", " + field.attrs.serialize_config if hasattr(field.attrs, "serialize_config") else ""%>
    %if field.arraySize > 0:
    for(int i = 0; i < ${field.arraySize}; ++i)
    {
        ret = ArchiveBlob(archive, arena, record.${name}[i]${fieldConfigArg});
        if(ret != 0)
        {
            SKR_LOG_ERROR(BinaryArrayBinaryFieldArchiveFailedFormat, action, "${record.name}", "${name}", i, ret);
            return ret;
        }
    }
    %else:
    ret = ArchiveBlob(archive, arena, record.${name}${fieldConfigArg});
    if(ret != 0)
    {
        SKR_LOG_ERROR(BinaryFieldArchiveFailedFormat, action, "${record.name}", "${name}", ret);
        return ret;
    }
    %endif
    %endfor
    return ret;
}
%else:
template<class S>
int __Archive(S* archive, ${record.name}& record${configParam})
{
    constexpr bool isWriter = std::is_same_v<S, skr_binary_writer_t>;
    const char* action = isWriter ? "Write" : "Read";
    int ret = 0;
    %for name, field in generator.filter_fields(record.fields):
    <% fieldConfigArg = ", " + field.attrs.serialize_config if hasattr(field.attrs, "serialize_config") else ""%>
    %if field.type == "skr_blob_arena_t":
    auto& arena_${name} = record.${name};
    ret = Archive(archive, arena_${name});
    if(ret != 0)
    {
        SKR_LOG_ERROR(BinaryFieldArchiveFailedFormat, action, "${record.name}", "${name}", ret);
        return ret;
    }
    %elif field.arraySize > 0:
    for(int i = 0; i < ${field.arraySize}; ++i)
    {
        ${archive_field(name, field, "[i]", fieldConfigArg)}
        if(ret != 0)
        {
            SKR_LOG_ERROR(BinaryArrayBinaryFieldArchiveFailedFormat, action, "${record.name}", "${name}", i, ret);
            return ret;
        }
    }
    %else:
    ${archive_field(name, field, "", fieldConfigArg)}
    if(ret != 0)
    {
        SKR_LOG_ERROR(BinaryFieldArchiveFailedFormat, action, "${record.name}", "${name}", ret);
        return ret;
    }
    %endif
    %endfor
    return ret;
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
int ReadTrait<${record.name}>::Read(skr_binary_reader_t* archive, skr_blob_arena_t& arena, ${record.name}& record${configParam})
{
    SkrZoneScopedN("binary::ReadTrait<${record.name}>::Read");
%for base in record.bases:
    int ret = ReadTrait<const ${base}&>::Read(archive, arena, (${base}&)record);
    if(ret != 0)
    {
        SKR_LOG_ERROR(BinaryBaseArchiveFailedFormat, "Read", "${record.name}", "${base}", ret);
        return ret;
    }
%endfor
    return __Archive(archive, arena, record${configArg});
}
int WriteTrait<${record.name}>::Write(skr_binary_writer_t* archive, skr_blob_arena_t& arena, const ${record.name}& record${configParam})
{
    SkrZoneScopedN("binary::WriteTrait<${record.name}>::Write");
%for base in record.bases:
    int ret = WriteTrait<${base}>::Write(archive, arena, (${base}&)record);
    if(ret != 0)
    {
        SKR_LOG_ERROR(BinaryBaseArchiveFailedFormat, "Write", "${record.name}", "${base}", ret);
        return ret;
    }
%endfor
    return __Archive(archive, arena, (${record.name}&)record${configArg});
} 
%else:
int ReadTrait<${record.name}>::Read(skr_binary_reader_t* archive, ${record.name}& record${configParam})
{
    SkrZoneScopedN("binary::ReadTrait<${record.name}>::Read");
%for base in record.bases:
    int ret = skr::binary::Read(archive, (${base}&)record);
    if(ret != 0)
    {
        SKR_LOG_ERROR(BinaryBaseArchiveFailedFormat, "Read", "${record.name}", "${base}", ret);
        return ret;
    }
%endfor
    return __Archive(archive, record${configArg});
}
int WriteTrait<${record.name}>::Write(skr_binary_writer_t* archive, const ${record.name}& record${configParam})
{
    SkrZoneScopedN("binary::WriteTrait<${record.name}>::Write");
%for base in record.bases:
    int ret = skr::binary::Write<${base}>(archive, record);
    if(ret != 0)
    {
        SKR_LOG_ERROR(BinaryBaseArchiveFailedFormat, "Write", "${record.name}", "${base}", ret);
        return ret;
    }
%endfor
    return __Archive(archive, (${record.name}&)record${configArg});
} 
%endif
%endfor
}
//END BINARY GENERATED