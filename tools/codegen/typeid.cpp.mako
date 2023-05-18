//BEGIN TYPEID GENERATED
#include "type/type.hpp"

<%
types = generator.filter_types(db.records) + generator.filter_types(db.enums)
%>
%for type in types:
static struct RegisterName${type.id}Helper
{
    RegisterName${type.id}Helper()
    {
        skr_guid_t id{${db.guid_constant(type)}};
        skr_register_type_name(&id, u8"${type.name}");
    }
} _RegisterName${type.id}Helper;
%endfor

//END TYPEID GENERATED