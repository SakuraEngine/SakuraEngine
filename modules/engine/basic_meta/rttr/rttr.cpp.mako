// BEGIN RTTR GENERATED
#include "SkrBase/misc/hash.h"
#include "SkrRTTR/type.hpp"
#include "SkrCore/exec_static.hpp"
#include "SkrContainers/tuple.hpp"
#include "SkrRTTR/export/export_builder.hpp"

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
<% method_rttr_data = method.generator_data["rttr"] %>\
            %if method.is_static:
            [[maybe_unused]] auto method_builder = builder.static_method<${tools.function_signature_of(method)}, <&${record.name}::${method.short_name}>(u8"${method.short_name}");
            %else:
            [[maybe_unused]] auto method_builder = builder.method<${tools.function_signature_of(method)}, <&${record.name}::${method.short_name}>(u8"${method.short_name}");
            %endif

            // params
            %for (i, param) in enumerate(method.parameters.values()):
            method_builder.param_at(${i})
                .name(u8"${param.name}");
            %endfor

            // flags
            %if method_rttr_data.flags:
            method_builder.flag(${tools.flags_expr(method, method_rttr_data.flags)});
            %endif

            // attributes
            %if method_rttr_data.attrs:
            method_builder.
                %for attr in method_rttr_data.attrs:
                attribute(::skr::attr::${attr})
                %endfor
                ;
            %endif
        }
        %endfor
        %endif

        // fields
        %if record_rttr_data.reflect_fields:
        %for field in record_rttr_data.reflect_fields:
        { // ${record.name}::${field.name}
<% field_rttr_data = field.generator_data["rttr"] %>\
            %if field.is_static:
            [[maybe_unused]] auto field_builder = builder.static_field<<&${record.name}::${field.name}>(u8"${field.name}");
            %else:
            [[maybe_unused]] auto field_builder = builder.field<<&${record.name}::${field.name}>(u8"${field.name}");
            %endif

            // flags
            %if field_rttr_data.flags:
            field_builder.flag(${tools.flags_expr(field, field_rttr_data.flags)});
            %endif

            // attributes
            %if field_rttr_data.attrs:
            field_builder.
                %for attr in field_rttr_data.attrs:
                attribute(::skr::attr::${attr})
                %endfor
                ;
            %endif
        }
        %endfor

        // flags
        %if record_rttr_data.flags:
        builder.flag(${tools.flags_expr(record, record_rttr_data.flags)});
        %endif

        // attributes
        %if record_rttr_data.attrs:
        builder.
            %for attr in record_rttr_data.attrs:
            attribute(::skr::attr::${attr})
            %endfor
            ;
        %endif

        %endif
    });
%endfor
    //============================> End Record Export <============================

    //============================> Begin Enum Export <============================
%for enum in enums:
    register_type_loader(type_id_of<${enum.name}>(), +[](Type* type){
<%
    enum_rttr_data=enum.generator_data["rttr"]
%>\
        // init type
        type->init(ETypeCategory::Enum);
        auto& enum_data = type->enum_data();

        // reserve
        enum_data.items.reserve(${len(enum.values)});

        // basic
        EnumBuilder<${enum.name}> builder(&enum_data);
        builder.basic_info();

        // items
        %for enum_value in enum.values.values():
        { // ${enum.name}::${enum_value.short_name}
<% enum_value_rttr_data=enum_value.generator_data["rttr"] %>\
            [[maybe_unused]] auto item_builder = builder.item(u8"${enum_value.short_name}", ${enum_value.name});

            // flags
            %if enum_value_rttr_data.flags:
            item_builder.flag(${tools.flags_expr(enum_value, enum_value_rttr_data.flags)});
            %endif

            // attributes
            %if enum_value_rttr_data.attrs:
            item_builder.
                %for attr in enum_value_rttr_data.attrs:
                attribute(::skr::attr::${attr})
                %endfor
                ;
            %endif
        }

        // flags
        %if enum_rttr_data.flags:
        builder.flag(${tools.flags_expr(enum, enum_rttr_data.flags)});
        %endif

        // attributes
        %if enum_rttr_data.attrs:
        builder.
            %for attr in enum_rttr_data.attrs:
            attribute(::skr::attr::${attr})
            %endfor
            ;
        %endif

        %endfor

    });
%endfor
    //============================> End Enum Export <============================
};
// END RTTR GENERATED
