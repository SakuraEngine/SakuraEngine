//BEGIN BINARY GENERATED
#include "SkrBase/types.h"
#include "SkrSerde/blob.h"

namespace skr::binary
{
%for record in generator.filter_types(db.records):
template<>
struct ${api} ReadTrait<${record.name}>
{
    static bool Read(SBinaryReader* archive, ${record.name}& value);
};
template<>
struct ${api} WriteTrait<${record.name}>
{
    static bool Write(SBinaryWriter* archive, const ${record.name}& value);
};
%endfor
}
//END BINARY GENERATED
