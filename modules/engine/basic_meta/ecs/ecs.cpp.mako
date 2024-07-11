// BEGIN ECS GENERATED
#include "SkrRT/ecs/sugoi.h"
#include "SkrRT/ecs/array.hpp"
#include "SkrRT/ecs/serde.hpp"
#include "SkrCore/exec_static.hpp"

// impl sugoi_id_of
%for record in records:
static sugoi_type_index_t& _sugoi_id_${str.replace(record.name, "::", "_")}() { static sugoi_type_index_t val = SUGOI_NULL_TYPE; return val;  }
sugoi_type_index_t sugoi_id_of<::${record.name}>::get()
{
    SKR_ASSERT(_sugoi_id_${str.replace(record.name, "::", "_")}() != SUGOI_NULL_TYPE);
    return _sugoi_id_${str.replace(record.name, "::", "_")}();
}
%endfor

// register component types
%if records:
SKR_EXEC_STATIC_CTOR
{
    using namespace skr::literals;

%for record in records:
    // register ecs component type ${record.name}
    {
<%
    record_ecs_comp_data = record.generator_data["ecs_component"]
    entity_fields_offset = tools.make_field_offset_list(record, tools.filter_entity, codegen_db)
    resource_fields_offset = tools.make_field_offset_list(record, tools.filter_resource_handle, codegen_db)
%>\
        sugoi_type_description_t desc;
        desc.name = u8"${record.name}";

        // guid
        desc.guid = u8"${record.generator_data["guid"]}"_guid;
        desc.guidStr = u8"${record.generator_data["guid"]}";

        // size & alignment
    %if record_ecs_comp_data.array:
        desc.alignment = alignof(sugoi::ArrayComponent<${record.name}, ${record_ecs_comp_data.array}>);
        desc.size = sizeof(sugoi::ArrayComponent<${record.name}, ${record_ecs_comp_data.array}>);
        desc.elementSize = sizeof(${record.name});
    %else:
        desc.alignment = alignof(${record.name});
        desc.size = std::is_empty_v<${record.name}> ? 0 : sizeof(${record.name});
        desc.elementSize = 0;
    %endif

        // flags
    %if record_ecs_comp_data.flags:
        desc.flags = ${tools.flag_expr(record_ecs_comp_data.flags)}
    %else:
        desc.flags = 0;
    %endif

        // entity fields
    %if entity_fields_offset:
        desc.entityFieldsCount = ${len(entity_fields_offset)};
        static intptr_t entityFields[] = {${", ".join(entity_fields_offset)}};
        desc.entityFields = (intptr_t)entityFields;
    %else:
        desc.entityFieldsCount = 0;
        desc.entityFields = 0;
    %endif

        // resource fields
    %if resource_fields_offset:
        desc.resourceFieldsCount = ${len(resource_fields_offset)};
        static intptr_t resourceFields[] = {${", ".join(resource_fields_offset)}};
        desc.resourceFields = (intptr_t)resourceFields;
    %else:
        desc.resourceFieldsCount = 0;
        desc.resourceFields = 0;
    %endif

        // custom logic
    %if record_ecs_comp_data.custom:
        ${record_ecs_comp_data.custom}(desc, skr::type_t<${record.name}>{});
    %endif

        // check managed
        ::sugoi::check_managed(desc, skr::type_t<${record.name}>{});

        _sugoi_id_${str.replace(record.name, "::", "_")}() = sugoiT_register_type(&desc);
    }
%endfor
};
%endif
// END ECS GENERATED