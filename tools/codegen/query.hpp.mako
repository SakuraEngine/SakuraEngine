//BEGIN QUERY GENERATED
#include "ecs/dual.h"
%for record in generator.filter_records(db.records):
<% query = generator.parse(record) %>
#define GENERATED_QUERY_BODY_${db.file_id}_${record.short_name} ${"\\"}
dual_query_t* query; ${"\\"}
void Initialize(dual_storage_t* storage); ${"\\"}
void Release() { if(query) dualQ_release(query); } ${"\\"}
struct TaskContext : private dual::task_context_t ${"\\"}
{ ${"\\"}
    using dual::task_context_t::task_context_t; ${"\\"}
    using dual::task_context_t::set_dirty; ${"\\"}
    using dual::task_context_t::count; ${"\\"}
    struct View ${"\\"}
    { ${"\\"}
    %for i, component in query.sequence():
        ${"const " if query.accesses[i].readonly else ""}${component}* _${db.short_name(component)}; ${"\\"}
    %endfor
    }; ${"\\"}
    View unpack(); ${"\\"}
    template<class T> ${"\\"}
    auto get() { return get(skr::type_t<T>{}); } ${"\\"}
    %for i, component in query.sequence():
    auto get(skr::type_t<${component}>) { return dual::task_context_t::get_owned_${"ro" if query.accesses[i].readonly else "rw"}<${component}, true>(${i}); } ${"\\"}
    %endfor
    template<class T> void set_dirty(dual::dirty_comp_t& dirty) { set_dirty(dirty, skr::type_t<T>{}); } ${"\\"}
    %for i, component in query.sequence():
    void set_dirty(dual::dirty_comp_t& dirty, skr::type_t<${component}>) { dual::task_context_t::set_dirty(dirty, ${i}); } ${"\\"}
    %endfor
    template<class T> ${"\\"}
    auto get(dual_chunk_view_t* view) { return get(view, skr::type_t<T>{}); } ${"\\"}
    %for i, component in query.unsequence():
    auto get(dual_chunk_view_t* view, skr::type_t<${component}>) { return dual::task_context_t::get_owned_${"ro" if query.accesses[i].readonly else "rw"}<${component}, true>(view, ${i}); } ${"\\"}
    %endfor
};
%endfor
//END QUERY GENERATED