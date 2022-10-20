// BEGIN DUAL GENERATED
#include "ecs/dual.h"

#ifdef __cplusplus
%for type in db.records:
%if generator.filter_record(type):
template<>
struct dual_id_of<::${type.name}>
{
    ${api} static dual_type_index_t get();
};
%endif
%endfor
#endif

//END DUAL GENERATED