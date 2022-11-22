//BEGIN TYPEID GENERATED
#include "type/type_registry.h"

<%
types = generator.filter_types(db.records)
types = types + generator.filter_types(db.enums)
%>
%for type in types:
static struct RegisterName${type.id}Helper
{
    RegisterName${type.id}Helper()
    {
        skr_guid_t id{${db.guid_constant(type)}};
        skr_register_type_name(&id, "${type.name}");
    }
} _RegisterName${type.id}Helper;
%endfor

//END TYPEID GENERATED