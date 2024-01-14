<%
    def filter_fileds(fields, pred, base=0):
        result = []
        for name, field in vars(fields).items():
            if pred(name, field):
                result.append(str(field.offset + base))
            elif field.type in db.name_to_record:
                result = result + filter_fileds(db.name_to_record[field.type].fields, pred, field.offset)
        return result
    records = generator.filter_records(db.records)
%>
// BEGIN DUAL GENERATED
#include "SkrRT/ecs/sugoi.h"
#include "SkrRT/ecs/array.hpp"
#include "SkrRT/ecs/luabind.hpp"
#include "SkrRT/ecs/serde.hpp"

%for type in records:
static struct RegisterComponent${type.id}Helper
{
    RegisterComponent${type.id}Helper()
    {
        sugoi_type_description_t desc;
        desc.name = u8"${type.name}";
        
    %if hasattr(type.attrs.component, "buffer"):
        desc.size = sizeof(sugoi::array_comp_T<${type.name}, ${type.attrs.component.buffer}>);
    %else:
        desc.size = std::is_empty_v<${type.name}> ? 0 : sizeof(${type.name});
    %endif
    <%
        entityFields = filter_fileds(type.fields, lambda name, field: field.rawType == "sugoi_entity_t")
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
        resourceFields = filter_fileds(type.fields, lambda name, field: field.type == "skr_resource_handle_t" or field.type.startswith("skr::resource::TResourceHandle"))
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
        desc.guidStr = u8"${type.attrs.guid}";
        desc.flags = 0;
    %if hasattr(type.attrs.component, "pin"):
        desc.flags |= SUGOI_TYPE_FLAG_PIN;
    %endif 
    %if hasattr(type.attrs.component, "chunk"):
        desc.flags |= SUGOI_TYPE_FLAG_CHUNK;
    %endif
    %if hasattr(type.attrs.component, "buffer"):
        desc.elementSize = sizeof(${type.name});
    %else:
        desc.elementSize = 0;
    %endif
    %if hasattr(type.attrs.component, "buffer"):
        desc.alignment = alignof(sugoi::array_comp_T<${type.name}, ${type.attrs.component.buffer}>);
    %else:
        desc.alignment = alignof(${type.name});
    %endif

        sugoi::SetLuaBindCallback<${type.name}>(desc);
        sugoi::SetSerdeCallback<${type.name}>(desc);
    
    %if hasattr(type.attrs.component, "custom"):
        ${type.attrs.component.custom}(desc, skr::type_t<${type.name}>{});
    %endif
    
    %if not hasattr(type.attrs.component, "unsafe"):
        ::sugoi::check_managed(desc, skr::type_t<${type.name}>{});
    %endif
        type = sugoiT_register_type(&desc);
    }
    sugoi_type_index_t type = SUGOI_NULL_TYPE;
} _RegisterComponent${type.id}Helper;

sugoi_type_index_t sugoi_id_of<::${type.name}>::get()
{
    auto result = _RegisterComponent${type.id}Helper.type;
    SKR_ASSERT(result != SUGOI_NULL_TYPE);
    return result;
}
%endfor

skr::span<sugoi_type_index_t> sugoi_get_all_component_types_${module}()
{
    static sugoi_type_index_t result[${len(records)}] {
    %for type in records:
        sugoi_id_of<::${type.name}>::get(),
    %endfor
    };
    return {result};
}

//END DUAL GENERATED