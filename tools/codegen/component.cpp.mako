<%
    def filter_fileds(fields, pred, base=0):
        result = []
        for name, field in vars(fields).items():
            if pred(name, field):
                result.append(str(field.offset + base))
            elif field.type in db.name_to_record:
                result = result + filter_fileds(db.name_to_record[field.type].fields, pred, field.offset)
        return result
%>
// BEGIN DUAL GENERATED
#include "ecs/dual.h"
#include "ecs/array.hpp"

%for type in generator.filter_records(db.records):
static struct RegisterComponent${type.id}Helper
{
    RegisterComponent${type.id}Helper()
    {
        dual_type_description_t desc;
        desc.name = "${type.name}";
        
    %if hasattr(type.attrs.component, "buffer"):
        desc.size = sizeof(dual::array_component_T<${type.name}, ${type.attrs.component.buffer}>);
    %else:
        desc.size = sizeof(${type.name});
    %endif
    <%
        entityFields = filter_fileds(type.fields, lambda name, field: field.rawType == "dual_entity_t")
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
        resourceFields = filter_fileds(type.fields, lambda name, field: field.rawType == "skr_resource_handle_t" or field.rawType.startswith("TResourceHandle"))
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
    
    %if hasattr(type.attrs.component, "custom"):
        ${type.attrs.component.custom}(desc, skr::type_t<${type.name}>{});
    %endif
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