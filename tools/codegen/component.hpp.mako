// BEGIN DUAL GENERATED
#include "SkrRT/ecs/dual.h"

#ifdef __cplusplus
%for type in generator.filter_records(db.records):
template<>
struct dual_id_of<::${type.name}>
{
    ${api} static dual_type_index_t get();
};
%endfor
//${api} skr::span<dual_type_index_t> dual_get_all_component_types_${module}();
#endif

//END DUAL GENERATED