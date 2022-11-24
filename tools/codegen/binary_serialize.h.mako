//BEGIN BINARY GENERATED
#include "binary/reader_fwd.h"
#include "binary/writer_fwd.h"
#if defined(__cplusplus)
namespace skr::binary
{
%for record in generator.filter_types(db.records):
    template<>
    struct ${api} ReadHelper<${record.name}>
    {
        static int Read(skr_binary_reader_t* archive, ${record.name}& value);
    };
    template<>
    struct ${api} WriteHelper<const ${record.name}&>
    {
        static int Write(skr_binary_writer_t* archive, const ${record.name}& value);
    };
%endfor
}
#endif
//END BINARY GENERATED
