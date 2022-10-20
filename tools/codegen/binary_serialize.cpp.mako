// BEGIN BINARY GENERATED
#include "utils/hash.h"
#include "platform/debug.h"
#include "utils/log.h"

namespace skr::binary {
%for record in generator.filter_types(db.records):
template<class S>
int __Archive(S* archive, ${record.name}& record)
{
    int ret = 0;
    %for name, field in generator.filter_fields(record.fields):
    ret = Archive(archive, record.${name});
    if(ret != 0)
        return ret;
    %endfor
    return ret;
}

template<>
int ReadValue(skr_binary_reader_t* archive, ${record.name}& record)
{
%for base in record.bases:
    int ret = ReadValue(archive, (${base}&)record);
    if(ret != 0)
        return ret;
%endfor
    return __Archive(archive, record);
} 
template<>
int WriteValue(skr_binary_writer_t* archive, const ${record.name}& record)
{
%for base in record.bases:
    int ret = WriteValue<const ${base}&>(archive, record);
    if(ret != 0)
        return ret;
%endfor
    return __Archive(archive, (${record.name}&)record);
} 
%endfor
}
//END BINARY GENERATED