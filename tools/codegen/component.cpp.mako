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
#include "SkrRT/ecs/dual.h"
#include "SkrRT/ecs/array.hpp"
#include "SkrRT/ecs/luabind.hpp"
#include "SkrRT/ecs/serde.hpp"

%for type in records:
static struct RegisterComponent${type.id}Helper
{
    RegisterComponent${type.id}Helper()
    {
        dual_type_description_t desc;
        desc.name = u8"${type.name}";
        
    %if hasattr(type.attrs.component, "buffer"):
        desc.size = sizeof(dual::array_comp_T<${type.name}, ${type.attrs.component.buffer}>);
    %else:
        desc.size = std::is_empty_v<${type.name}> ? 0 : sizeof(${type.name});
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
    %if hasattr(type.attrs.component, "buffer"):
        desc.alignment = alignof(dual::array_comp_T<${type.name}, ${type.attrs.component.buffer}>);
    %else:
        desc.alignment = alignof(${type.name});
    %endif

        dual::SetLuaBindCallback<${type.name}>(desc);
        dual::SetSerdeCallback<${type.name}>(desc);
    
    %if hasattr(type.attrs.component, "custom"):
        ${type.attrs.component.custom}(desc, skr::type_t<${type.name}>{});
    %endif
    
    %if not hasattr(type.attrs.component, "unsafe"):
        ::dual::check_managed(desc, skr::type_t<${type.name}>{});
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

skr::span<dual_type_index_t> dual_get_all_component_types_${module}()
{
    static dual_type_index_t result[${len(records)}] {
    %for type in records:
        dual_id_of<::${type.name}>::get(),
    %endfor
    };
    return {result};
}

//END DUAL GENERATED