//BEGIN QUERY GENERATED
#include "SkrRT/ecs/sugoi.h"
%for record in generator.filter_records(db.records):
<% query = generator.parse(record) %>
#define GENERATED_QUERY_BODY_${db.file_id}_${record.short_name} ${"\\"}
sugoi_query_t* query; ${"\\"}
void Initialize(sugoi_storage_t* storage); ${"\\"}
void Release() { if(query) sugoiQ_release(query); } ${"\\"}
struct TaskContext : private sugoi::task_context_t ${"\\"}
{ ${"\\"}
    using sugoi::task_context_t::task_context_t; ${"\\"}
    using sugoi::task_context_t::count; ${"\\"}
    struct View ${"\\"}
    { ${"\\"}
    %for i, component in query.sequence():
        ${"const " if query.accesses[i].readonly else ""}${component}* _${db.short_name(component)}; ${"\\"}
    %endfor
    }; ${"\\"}
    struct Ref ${"\\"}
    { ${"\\"}
    %for i, component in query.sequence():
    %if query.accesses[i].optional:
        ${"const " if query.accesses[i].readonly else ""}${component}* _${db.short_name(component)}; ${"\\"}
    %else:
        ${"const " if query.accesses[i].readonly else ""}${component}& _${db.short_name(component)}; ${"\\"}
    %endif
    %endfor
    }; ${"\\"}
    View unpack(); ${"\\"}
    Ref unpack(int i); ${"\\"}
    template<class T> ${"\\"}
    auto get() { return get(skr::type_t<T>{}); } ${"\\"}
    %for i, component in query.sequence():
    auto get(skr::type_t<${component}>) { return (${"const" if query.accesses[i].readonly else ""} ${component}*)paramPtrs[${i}]; } ${"\\"}
    %endfor
    template<class T> void set_dirty(sugoi::dirty_comp_t& dirty) { set_dirty(dirty, skr::type_t<T>{}); } ${"\\"}
    %for i, component in query.sequence():
    void set_dirty(sugoi::dirty_comp_t& dirty, skr::type_t<${component}>) { sugoi::task_context_t::set_dirty(dirty, ${i}); } ${"\\"}
    %endfor
    template<class T> ${"\\"}
    auto get(sugoi_chunk_view_t* view) { return get(view, skr::type_t<T>{}); } ${"\\"}
    %for i, component in query.unsequence():
    auto get(sugoi_chunk_view_t* view, skr::type_t<${component}>) { return sugoi::task_context_t::get_owned_${"ro" if query.accesses[i].readonly else "rw"}<${component}, true>(view, sugoi_id_of<${component}>::get()); } ${"\\"}
    %endfor
};
%endfor
//END QUERY GENERATED