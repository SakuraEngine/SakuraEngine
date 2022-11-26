//BEGIN BINARY GENERATED
#include "binary/reader_fwd.h"
#include "binary/writer_fwd.h"
#include "binary/blob_fwd.h"
#if defined(__cplusplus)

namespace skr::binary
{
%for record in generator.filter_types(db.records):
template<>
struct ${api} ReadHelper<${record.name}>
{
%if generator.filter_blob_type(record):
    static int Read(skr_binary_reader_t* archive, skr_blob_arena_t& arena, ${record.name}& value);
%else:
    static int Read(skr_binary_reader_t* archive, ${record.name}& value);
%endif
};
template<>
struct ${api} WriteHelper<const ${record.name}&>
{
%if generator.filter_blob_type(record):
    static int Write(skr_binary_writer_t* archive, skr_blob_arena_t& arena, const ${record.name}& value);
%else:
    static int Write(skr_binary_writer_t* archive, const ${record.name}& value);
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
struct ${record.short_name}Owned;
</%call>

namespace skr::binary
{
template<>
struct BlobOwnedType<${record.name}>
{
    using type = ${record.short_name}Owned;
};
template<>
struct ${api} BlobHelper<${record.name}>
{
    static void BuildArena(skr_blob_arena_builder_t& arena, ${record.name}& dst, const ${record.name}Owned& src);
};
}

<%call expr="namespaced(record)">
<%
    if record.bases:
        basesOwned = ": " + ", ".join(["public %sOwned"%base for base in record.bases])
    else:
        basesOwned = ""
%>
struct ${record.short_name}Owned ${basesOwned}
{
%for name, field in vars(record.fields).items():
    typename skr::binary::BlobOwnedType<${field.rawType}>::type ${name};
%endfor
};
</%call>
%endif
%endfor

#endif
//END BINARY GENERATED
