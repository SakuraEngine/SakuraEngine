//DO NOT MODIFY THIS FILE
#include <type_traits>
%for header in db.headers:
#include "${header}"
%endfor

namespace bitsery {
%for enum in db.enums:
template<class S>
void serialize(S& s, ${enum.name}& e)
{
    serialize(s, (std::underlying_type_t<${enum.name}>&)(e));
} 
%endfor

%for record in db.records:
template<class S>
void serialize(S& s, ${record.name}& record)
{
    %for base in record.bases:
    serialize(s, (${base}&)record);
    %endfor
    s(${",".join( "record."+field.name for field in record.fields)});
} 
%endfor
}