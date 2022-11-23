//BEGIN BINARY GENERATED
#include "binary/reader_fwd.h"
#include "binary/writer_fwd.h"
#if defined(__cplusplus)
namespace skr::binary
{
%for record in generator.filter_types(db.records):
    template <>
    ${api} int ReadValue(skr_binary_reader_t* archive, ${record.name}& v);
    template <>
    ${api} int WriteValue(skr_binary_writer_t* archive, const ${record.name}& v);
%endfor
}
#endif
//END BINARY GENERATED
