// BEGIN ECS GENERATED
#include "SkrRT/ecs/sogoi.h"
%for record in records:
template<>
struct sugoi_id_of<::${record.name}>
{
    ${api} static sugoi_type_index_t get();
};
%endfor
// END ECS GENERATED