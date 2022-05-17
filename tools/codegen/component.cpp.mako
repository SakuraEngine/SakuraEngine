
#include "ecs/dual.h"
%for header in db.headers:
#include "${header}"
%endfor

%for type in db.types:
static struct RegisterComponent${type.id}Helper
{
    RegisterComponent${type.id}Helper()
    {
        dual_type_description_t desc;
        desc.name = "${type.short_name}";
        
    %if type.buffer:
        desc.size = sizeof(${type.name}) * ${type.buffer} + 16u;
    %else:
        desc.size = sizeof(${type.name});
    %endif
        desc.entityFieldsCount = ${len(type.entityFields)};
    %if type.entityFields:
        static intptr_t entityFields[] = {${", ".join(type.entityFields)}};
        desc.entityFields = (intptr_t)entityFields;
    %else:
        desc.entityFields = 0;
    %endif
        desc.guid = {${type.guidConstant}};
    %if type.managed:
        desc.callback = GetComponentCallback<${type.name}>();
    %else:
        desc.callback = {};
    %endif
        desc.flags = 0;
    %if type.pin:
        desc.flags |= DTF_PIN;
    %endif
    %if type.buffer:
        desc.elementSize = sizeof(${type.name});
    %else:
        desc.elementSize = 0;
    %endif
        desc.alignment = alignof(${type.name});
        type = dualT_register_type(&desc);
    }
    dual_type_index_t type = NULL_TYPE;
} _RegisterComponent${type.id}Helper;

dual_type_index_t dual_id_of<::${type.name}>::get()
{
    auto result = _RegisterComponent${type.id}Helper.type;
    SKR_ASSERT(result != NULL_TYPE);
    return result;
}
%endfor