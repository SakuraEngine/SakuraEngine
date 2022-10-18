// DO NOT MODIFY THIS FILE
// START SERIALIZE IMPLEMENTATION
#include "utils/hash.h"
#include "platform/debug.h"
#include "utils/log.h"
#include "binary_serialize.generated.h"
%for header in db.headers:
#include "${header}"
%endfor

namespace skr::binary {
%for record in db.records:
template<class S>
int __Archive(S* archive, ${record.name}& record)
{
    int ret = 0;
    %for field in record.allFields():
    ret = Archive(archive, record.${field.name});
    if(ret != 0)
        return ret;
    %endfor
    return ret;
}

template<>
int ReadValue(skr_binary_reader_t* archive, ${record.name}& record)
{
    return __Archive(archive, record);
} 
template<>
int WriteValue(skr_binary_writer_t* archive, const ${record.name}& record)
{
    return __Archive(archive, (${record.name}&)record);
} 
%endfor
}