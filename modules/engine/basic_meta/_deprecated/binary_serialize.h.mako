//BEGIN BINARY GENERATED
#include "SkrSerde/bin_serde.hpp"

namespace skr
{
%for record in generator.filter_types(db.records):
template<>
struct ${api} BinSerde<${record.name}>
{
    static bool read(SBinaryReader* r, ${record.name}& v);
    static bool write(SBinaryWriter* w, const ${record.name}& v);
};
%endfor
}
//END BINARY GENERATED
