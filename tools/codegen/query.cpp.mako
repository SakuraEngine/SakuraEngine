
// BEGIN QUERY GENERATED
%for record in generator.filter_records(db.records):
<% query = generator.parse(record) %>
void ${record.name}::Initialize(dual_storage_t* storage)
{
    query = dualQ_from_literal(storage, "${record.attrs.query}");
}

${record.name}::TaskContext::View ${record.name}::TaskContext::Unpack()
{
    View result;
%for i, component in enumerate(query.components):
%if query.accesses[i].readonly:
    result._${db.short_name(component)} = get_owned_ro<${component}, true>(${i});
%else:
    result._${db.short_name(component)} = get_owned_rw<${component}, true>(${i});
%endif
%endfor
    return result;
}
%endfor
// END QUERY GENERATED
