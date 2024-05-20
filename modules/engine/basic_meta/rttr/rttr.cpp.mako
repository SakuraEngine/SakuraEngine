// BEGIN RTTR GENERATED
#include "SkrRTTR/type.hpp"
#include "SkrCore/exec_static.hpp"
#include "SkrContainers/tuple.hpp"
#include "SkrRTTR/export/enum_builder.hpp"
#include "SkrRTTR/export/record_builder.hpp"

namespace skr::rttr 
{
//============================> Begin Enum Traits <============================
%for enum in generator.enums:
span<EnumItem<${enum.name}>> EnumTraits<${enum.name}>::items()
{
    static EnumItem<${enum.name}> items[] = {
    %for name, value in vars(enum.values).items():
        {u8"${db.short_name(name)}", ${name}},
    %endfor
    };
    return items;
}
skr::StringView EnumTraits<${enum.name}>::to_string(const ${enum.name}& value)
{
    switch (value)
    {
    %for name, value in vars(enum.values).items():
    case ${enum.name}::${db.short_name(name)}: return u8"${db.short_name(name)}";
    %endfor
    default: SKR_UNREACHABLE_CODE(); return u8"${enum.name}::INVALID_ENUMERATOR";
    }
}
bool EnumTraits<${enum.name}>::from_string(skr::StringView str, ${enum.name}& value)
{
    const auto hash = skr_hash64(str.raw().data(), str.size(), 0);
    switch(hash)
    {
    %for name, value in vars(enum.values).items():
        case skr::consteval_hash(u8"${db.short_name(name)}"): if(str == u8"${db.short_name(name)}") value = ${name}; return true;
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
%for record in generator.records:
    register_type_loader(type_id_of<${record.name}>(), +[](Type* type){
        // init type
        type->init(ETypeCategory::Record);
        auto& record_data = type->record_data();

        // reserve
        record_data.bases_data.reserve(${len(record.bases)});
        record_data.methods.reserve(${len(record.methods)});
        record_data.fields.reserve(${len(record.fields)});
        
        // build
        RecordBuilder<${record.name}> builder(&record_data);
        builder
            .basic_info()
            // bases
        %if record.bases:
            .bases<${", ".join(record.bases)}>()
        %endif
            // methods
        %if record.fields:
            %for name, field in record.fields:
            .field<&${record.name}::${name}>(${name})
            %endfor
        %endif
            // fields
        %if record.methods:
            %for method in record.methods:
            .method<&${record.name}::${db.short_name(method.name)}>(${db.short_name(method.name)})
            %endfor
        %endif
            ;
    });
%endfor
    //============================> End Record Export <============================

    //============================> Begin Enum Export <============================
%for enum in generator.enums:
    register_type_loader(type_id_of<${enum.name}>(), &enum_type_loader_from_traits<${enum.name}>);
%endfor
    //============================> End Enum Export <============================
};
// END RTTR GENERATED
