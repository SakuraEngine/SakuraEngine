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
    rttr_fields=record.generator_data["rttr"]["rttr_fields"]
    rttr_methods=record.generator_data["rttr"]["rttr_methods"]
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
        %if record.bases:
        builder.bases<${", ".join(record.bases)}>();
        %endif
        
        // methods
        %if len(rttr_methods) > 0:
        %for method in rttr_methods:
        builder..method<<&${record.name}::${method.short_name}>(u8"${method.short_name}");
        %endfor
        %endif

        // fields
        %if len(rttr_fields) > 0:
        %for field in rttr_fields:
        builder.field<<&${record.name}::${field.name}>(u8"${field.name}");
        %endfor
        %endif
            ;
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
