//DO NOT MODIFY THIS FILE
#pragma once
#include "${config}"
#include "binary/reader.h"
#include "binary/writer.h"

#ifdef __cplusplus
%for record in db.records:
%if hasattr(record, "namespace"):
namespace ${record.namespace} { struct ${record.short_name}; }
%else:
struct ${record.short_name};
%endif
%endfor
%for enum in db.enums:
%if hasattr(enum, "namespace"):
namespace ${enum.namespace} { enum ${enum.short_name} ${enum.postfix}; }
%else:
enum ${enum.short_name} ${enum.postfix};
%endif
%endfor
namespace skr::binary
{
%for record in db.records:
    template <>
    ${api} int ReadValue(skr_binary_reader_t* archive, ${record.name}& v);
    template <>
    ${api} int WriteValue(skr_binary_writer_t* archive, const ${record.name}& v);
%endfor
%for enum in db.enums:
    template <>
    inline int ReadValue(skr_binary_reader_t* archive, ${enum.name}& e)
    {
        return ReadValue(archive, (std::underlying_type_t<${enum.name}>&)(e));
    }
    template <>
    inline int WriteValue(skr_binary_writer_t* archive, ${enum.name} e)
    {
        return WriteValue(archive, (std::underlying_type_t<${enum.name}>)(e));
    }
%endfor
}
#endif