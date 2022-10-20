// BEGIN DUAL GENERATED
#include "ecs/dual.h"

#ifdef __cplusplus
%for type in generator.filter_records(db.records):
template<>
struct dual_id_of<::${type.name}>
{
    ${api} static dual_type_index_t get();
};
%endfor
#endif

//END DUAL GENERATED