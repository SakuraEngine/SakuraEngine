// BEGIN BINARY GENERATED
#include "misc/hash.h"
#include "platform/debug.h"
#include "misc/log.h"
#include "serde/binary/reader.h"
#include "serde/binary/writer.h"
#include "serde/binary/blob.h"

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
    int ret = 0;
    %for name, field in generator.filter_fields(record.fields):
    <% fieldConfigArg = ", " + field.attrs.serialize_config if hasattr(field.attrs, "serialize_config") else ""%>
    %if field.arraySize > 0:
    for(int i = 0; i < ${field.arraySize}; ++i)
    {
        ret = ArchiveBlob(archive, arena, record.${name}[i]${fieldConfigArg});
        if(ret != 0)
        {
            SKR_LOG_ERROR("Archive ${record.name}.${name}[%d] with arena failed: %d", i, ret);
            return ret;
        }
    }
    %else:
    ret = ArchiveBlob(archive, arena, record.${name}${fieldConfigArg});
    if(ret != 0)
    {
        SKR_LOG_ERROR("Archive ${record.name}.${name} with arena failed: %d", ret);
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
    int ret = 0;
    %for name, field in generator.filter_fields(record.fields):
    <% fieldConfigArg = ", " + field.attrs.serialize_config if hasattr(field.attrs, "serialize_config") else ""%>
    %if field.type == "skr_blob_arena_t":
    auto& arena_${name} = record.${name};
    ret = Archive(archive, arena_${name});
    if(ret != 0)
    {
        SKR_LOG_ERROR("Archive ${record.name}.${name} failed: %d", ret);
        return ret;
    }
    %elif field.arraySize > 0:
    for(int i = 0; i < ${field.arraySize}; ++i)
    {
        ${archive_field(name, field, "[i]", fieldConfigArg)}
        if(ret != 0)
        {
            SKR_LOG_ERROR("Archive ${record.name}.${name}[%d] failed: %d", i, ret);
            return ret;
        }
    }
    %else:
    ${archive_field(name, field, "", fieldConfigArg)}
    if(ret != 0)
    {
        SKR_LOG_ERROR("Archive ${record.name}.${name} failed: %d", ret);
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
%for base in record.bases:
    int ret = ReadTrait<const ${base}&>::Read(archive, arena, (${base}&)record);
    if(ret != 0)
    {
        SKR_LOG_ERROR("Read ${record.name} base ${base} failed: %d", ret);
        return ret;
    }
%endfor
    return __Archive(archive, arena, record${configArg});
}
int WriteTrait<const ${record.name}&>::Write(skr_binary_writer_t* archive, skr_blob_arena_t& arena, const ${record.name}& record${configParam})
{
%for base in record.bases:
    int ret = WriteTrait<const ${base}&>::Write(archive, arena, (${base}&)record);
    if(ret != 0)
    {
        SKR_LOG_ERROR("Write ${record.name} base ${base} with arena failed: %d", ret);
        return ret;
    }
%endfor
    return __Archive(archive, arena, (${record.name}&)record${configArg});
} 
%else:
int ReadTrait<${record.name}>::Read(skr_binary_reader_t* archive, ${record.name}& record${configParam})
{
%for base in record.bases:
    int ret = skr::binary::Read(archive, (${base}&)record);
    if(ret != 0)
    {
        SKR_LOG_ERROR("Read ${record.name} base ${base} failed: %d", ret);
        return ret;
    }
%endfor
    return __Archive(archive, record${configArg});
}
int WriteTrait<const ${record.name}&>::Write(skr_binary_writer_t* archive, const ${record.name}& record${configParam})
{
%for base in record.bases:
    int ret = skr::binary::Write<const ${base}&>(archive, record);
    if(ret != 0)
    {
        SKR_LOG_ERROR("Write ${record.name} base ${base} failed: %d", ret);
        return ret;
    }
%endfor
    return __Archive(archive, (${record.name}&)record${configArg});
} 
%endif
%endfor
}
//END BINARY GENERATED