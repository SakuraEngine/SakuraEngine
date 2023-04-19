//BEGIN BINARY GENERATED
#include "binary/reader_fwd.h"
#include "binary/writer_fwd.h"
#include "binary/blob_fwd.h"
#if defined(__cplusplus)
namespace skr::binary
{
%for record in generator.filter_types(db.records):
template<>
struct ${api} ReadTrait<${record.name}>
{<% configParam = ", " + record.attrs.serialize_config if hasattr(record.attrs, "serialize_config") else ""%>
%if generator.filter_blob_type(record):
    static int Read(skr_binary_reader_t* archive, skr_blob_arena_t& arena, ${record.name}& value${configParam});
%else:
    static int Read(skr_binary_reader_t* archive, ${record.name}& value ${configParam});
%endif
};
template<>
struct ${api} WriteTrait<const ${record.name}&>
{<% configParam = ", " + record.attrs.serialize_config if hasattr(record.attrs, "serialize_config") else ""%>
%if generator.filter_blob_type(record):
    static int Write(skr_binary_writer_t* archive, skr_blob_arena_t& arena, const ${record.name}& value${configParam});
%else:
    static int Write(skr_binary_writer_t* archive, const ${record.name}& value ${configParam});
%endif
};
%endfor
}

<%def name="namespaced(record)">
%if hasattr(record, "namespace"):
namespace ${record.namespace} {
%endif
${caller.body()}
%if hasattr(record, "namespace"):
}
%endif
</%def>

%for record in generator.filter_types(db.records):
%if generator.filter_blob_type(record):

<%call expr="namespaced(record)">
struct ${record.short_name}Builder;
</%call>

namespace skr::binary
{
template<>
struct BlobBuilderType<${record.name}>
{
    using type = ${record.name}Builder;
};
template<>
struct ${api} BlobTrait<${record.name}>
{
    static void BuildArena(skr_blob_arena_builder_t& arena, ${record.name}& dst, const ${record.name}Builder& src);
    static void Remap(skr_blob_arena_t& arena, ${record.name}& dst);
};
}
<%
    if record.bases:
        basesBuilder = ": " + ", ".join(["public %sBuilder"%base for base in record.bases])
    else:
        basesBuilder = ""
%>
#define GENERATED_BLOB_BUILDER_${db.file_id}_${record.short_name} ${"\\"}
struct ${record.short_name}Builder ${basesBuilder} ${"\\"}
{ ${"\\"}
%for name, field in vars(record.fields).items():
    typename skr::binary::BlobBuilderType<${field.rawType}>::type ${name}; ${"\\"}
%endfor
};
%endif
%endfor

#endif
//END BINARY GENERATED
