// BEGIN BINARY GENERATED
#include "utils/hash.h"
#include "platform/debug.h"
#include "utils/log.h"
#include "binary/reader.h"
#include "binary/writer.h"

namespace skr::binary {
%for record in generator.filter_types(db.records):
template<class S>
int __Archive(S* archive, ${record.name}& record)
{
    int ret = 0;
    %for name, field in generator.filter_fields(record.fields):
    %if field.arraySize > 0:
    for(int i = 0; i < ${field.arraySize}; ++i)
    {
        ret = Archive(archive, record.${name}[i]);
        if(ret != 0)
            return ret;
    }
    %else:
    ret = Archive(archive, record.${name});
    if(ret != 0)
        return ret;
    %endif
    %endfor
    return ret;
}


int ReadHelper<${record.name}>::Read(skr_binary_reader_t* archive, ${record.name}& record)
{
%for base in record.bases:
    int ret = skr::binary::Read(archive, (${base}&)record);
    if(ret != 0)
        return ret;
%endfor
    return __Archive(archive, record);
} 
int WriteHelper<const ${record.name}&>::Write(skr_binary_writer_t* archive, const ${record.name}& record)
{
%for base in record.bases:
    int ret = skr::binary::Write<const ${base}&>(archive, record);
    if(ret != 0)
        return ret;
%endfor
    return __Archive(archive, (${record.name}&)record);
} 
%endfor
}
//END BINARY GENERATED