//BEGIN QUERY GENERATED
#include "ecs/dual.h"
%for record in generator.filter_records(db.records):
<% query = generator.parse(record) %>
#define GENERATED_QUERY_BODY_${db.file_id}_${record.short_name} ${"\\"}
dual_query_t* query; ${"\\"}
void Initialize(dual_storage_t* storage); ${"\\"}
void Release() { if(query) dualQ_release(query); } ${"\\"}
struct TaskContext : dual::task_context_t ${"\\"}
{ ${"\\"}
    struct View ${"\\"}
    { ${"\\"}
    %for i, component in enumerate(query.sequence):
        ${"const " if query.accesses[i].readonly else ""}${component}* _${db.short_name(component)}; ${"\\"}
    %endfor
    }; ${"\\"}
    View Unpack(); ${"\\"}
};
%endfor
//END QUERY GENERATED