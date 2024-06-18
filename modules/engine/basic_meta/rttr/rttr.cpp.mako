// BEGIN RTTR GENERATED
#include "SkrRTTR/type.hpp"
#include "SkrCore/exec_static.hpp"
#include "SkrContainers/tuple.hpp"
#include "SkrRTTR/export/export_builder.hpp"

namespace skr::rttr 
{
//============================> Begin Enum Traits <============================
%for enum in enums:
span<EnumItem<${enum.name}>> EnumTraits<${enum.name}>::items()
{
    static EnumItem<${enum.name}> items[] = {
    %for enum_value in enum.values.values():
        {u8"${enum_value.short_name}", ${enum_value.name}},
    %endfor
    };
    return items;
}
skr::StringView EnumTraits<${enum.name}>::to_string(const ${enum.name}& value)
{
    switch (value)
    {
    %for enum_value in enum.values.values():
    case ${enum.name}::${enum_value.short_name}: return u8"${enum_value.short_name}";
    %endfor
    default: SKR_UNREACHABLE_CODE(); return u8"${enum.name}::INVALID_ENUMERATOR";
    }
}
bool EnumTraits<${enum.name}>::from_string(skr::StringView str, ${enum.name}& value)
{
    const auto hash = skr_hash64(str.raw().data(), str.size(), 0);
    switch(hash)
    {
    %for enum_value in enum.values.values():
        case skr::consteval_hash(u8"${enum_value.short_name}"): if(str == u8"${enum_value.short_name}") value = ${enum_value.name}; return true;
    %endfor
        default:
            return false;
    }
}
%endfor
//============================> End Enum Traits <============================
}

SKR_EXEC_STATIC_CTOR
{
    using namespace ::skr::rttr;

    //============================> Begin Record Export <============================
%for record in records:
    register_type_loader(type_id_of<${record.name}>(), +[](Type* type){
<%
    record_rttr_data=record.generator_data["rttr"]
%>\
        // init type
        type->init(ETypeCategory::Record);
        auto& record_data = type->record_data();

        // reserve
        record_data.bases_data.reserve(${len(record.bases)});
        record_data.methods.reserve(${len(record.methods)});
        record_data.fields.reserve(${len(record.fields)});
        
        // basic
        RecordBuilder<${record.name}> builder(&record_data);
        builder.basic_info();
        %if record_rttr_data.reflect_bases:
        builder.bases<${", ".join(record_rttr_data.reflect_bases)}>();
        %endif
        
        // methods
        %if record_rttr_data.reflect_methods:
        %for method in record_rttr_data.reflect_methods:
        { // ${record.name}::${method.short_name}
            %if method.is_static:
            auto method_builder = builder.static_method<${tools.function_signature_of(method)}, <&${record.name}::${method.short_name}>(u8"${method.short_name}");
            %else:
            auto method_builder = builder.method<${tools.function_signature_of(method)}, <&${record.name}::${method.short_name}>(u8"${method.short_name}");
            %endif
            %for (i, param) in enumerate(method.parameters.values()):
            method_builder.param_at(${i})
                .name(u8"${param.name}");
            %endfor
        }
        %endfor
        %endif

        // fields
        %if record_rttr_data.reflect_fields:
        %for field in record_rttr_data.reflect_fields:
        { // ${record.name}::${field.name}
            %if field.is_static:
            auto field_builder = builder.static_field<<&${record.name}::${field.name}>(u8"${field.name}");
            %else:
            auto field_builder = builder.field<<&${record.name}::${field.name}>(u8"${field.name}");
            %endif
        }
        %endfor

        // flags & attributes
        %if record_rttr_data.flags:
        builder.flags(${tools.flags_expr(record, record_rttr_data.flags)});
        %endif

        %endif
    });
%endfor
    //============================> End Record Export <============================

    //============================> Begin Enum Export <============================
%for enum in enums:
    register_type_loader(type_id_of<${enum.name}>(), &enum_type_loader_from_traits<${enum.name}>);
%endfor
    //============================> End Enum Export <============================
};
// END RTTR GENERATED
