//BEGIN BINARY GENERATED
#include "binary/reader.h"
#include "binary/writer.h"
#if defined(__cplusplus)
namespace skr::binary
{
%for record in generator.filter_types(db.records):
    template <>
    ${api} int ReadValue(skr_binary_reader_t* archive, ${record.name}& v);
    template <>
    ${api} int WriteValue(skr_binary_writer_t* archive, const ${record.name}& v);
%endfor
%for enum in generator.filter_types(db.enums):
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
//END BINARY GENERATED
