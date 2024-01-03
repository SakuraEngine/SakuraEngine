// BEGIN DUAL GENERATED
#include "SkrRT/ecs/sugoi.h"

#ifdef __cplusplus
%for type in generator.filter_records(db.records):
template<>
struct sugoi_id_of<::${type.name}>
{
    ${api} static sugoi_type_index_t get();
};
%endfor
//${api} skr::span<sugoi_type_index_t> sugoi_get_all_component_types_${module}();
#endif

//END DUAL GENERATED