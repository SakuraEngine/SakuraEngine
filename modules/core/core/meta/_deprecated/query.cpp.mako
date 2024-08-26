
// BEGIN QUERY GENERATED
%for record in generator.filter_records(db.records):
<% query = generator.parse(record) %>
void ${record.name}::Initialize(sugoi_storage_t* storage)
{
    query = sugoiQ_from_literal(storage, u8"${record.attrs.query}");
}

${record.name}::TaskContext::View ${record.name}::TaskContext::unpack()
{
    View result
    {
%for i, component in query.sequence():
        (${"const" if query.accesses[i].readonly else ""} ${component}*)paramPtrs[${i}],
%endfor
    };
    return result;
}

${record.name}::TaskContext::Ref ${record.name}::TaskContext::unpack(int i)
{
    Ref result
    {
%for i, component in query.sequence():
    %if query.accesses[i].optional:
        paramPtrs[${i}] ? (${"const" if query.accesses[i].readonly else ""} ${component}*)paramPtrs[${i}] + i : nullptr,
    %else:
        ((${"const" if query.accesses[i].readonly else ""} ${component}*)paramPtrs[${i}])[i],
    %endif
%endfor
    };
    return result;
}
%endfor
// END QUERY GENERATED
