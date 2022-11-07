// BEGIN DUAL GENERATED
#include "ecs/dual.h"
#include "ecs/array.hpp"

%for type in generator.filter_records(db.records):
static struct RegisterComponent${type.id}Helper
{
    RegisterComponent${type.id}Helper()
    {
        dual_type_description_t desc;
        desc.name = "${type.short_name}";
        
    %if hasattr(type.attrs.component, "buffer"):
        desc.size = sizeof(dual::array_component_T<${type.name}, ${type.attrs.component.buffer}>);
    %else:
        desc.size = sizeof(${type.name});
    %endif
    <%
        entityFields = ["(intptr_t)&{}::{}".format(type.name, name) for name, field in vars(type.fields).items() if field.rawType == "dual_entity_t"]
    %>
    %if entityFields:
        desc.entityFieldsCount = ${len(entityFields)};
        static intptr_t entityFields[] = {${", ".join(entityFields)}};
        desc.entityFields = (intptr_t)entityFields;
    %else:
        desc.entityFieldsCount = 0;
        desc.entityFields = 0;
    %endif
    <%
        resourceFields = ["(intptr_t)&{}::{}".format(type.name, name) for name, field in vars(type.fields).items() if field.rawType == "skr_resource_handle_t" or field.rawType.startswith("TResourceHandle")]
    %>
    %if resourceFields:
        desc.resourceFieldsCount = ${len(resourceFields)};
        static intptr_t resourceFields[] = {${", ".join(resourceFields)}};
        desc.resourceFields = (intptr_t)resourceFields;
    %else:
        desc.resourceFieldsCount = 0;
        desc.resourceFields = 0;
    %endif
        desc.guid = {${db.guid_constant(type)}};
    %if hasattr(type.attrs.component, "managed"):
        desc.callback = GetComponentCallback<${type.name}>();
    %else:
        desc.callback = {};
    %endif
        desc.flags = 0;
    %if hasattr(type.attrs.component, "pin"):
        desc.flags |= DTF_PIN;
    %endif 
    %if hasattr(type.attrs.component, "chunk"):
        desc.flags |= DTF_CHUNK;
    %endif
    %if hasattr(type.attrs.component, "buffer"):
        desc.elementSize = sizeof(${type.name});
    %else:
        desc.elementSize = 0;
    %endif
        desc.alignment = alignof(${type.name});
        type = dualT_register_type(&desc);
    }
    dual_type_index_t type = DUAL_NULL_TYPE;
} _RegisterComponent${type.id}Helper;

dual_type_index_t dual_id_of<::${type.name}>::get()
{
    auto result = _RegisterComponent${type.id}Helper.type;
    SKR_ASSERT(result != DUAL_NULL_TYPE);
    return result;
}
%endfor

//END DUAL GENERATED