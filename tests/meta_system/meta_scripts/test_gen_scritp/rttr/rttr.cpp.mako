// BEGIN RTTR GENERATED
#include "SkrRT/rttr/type_loader/type_loader.hpp"
#include "SkrRT/rttr/type/record_type.hpp"
#include "SkrRT/rttr/exec_static.hpp"
#include "SkrRT/rttr/type_loader/enum_type_from_traits_loader.hpp"
#include "SkrContainers/tuple.hpp"

namespace skr::rttr 
{
%for enum in module_db.get_enums():
span<EnumItem<${enum.name}>> EnumTraits<${enum.name}>::items()
{
    static EnumItem<${enum.name}> items[] = {
    %for name, value in enum.values.items():
        {u8"${value.short_name}", ${name}},
    %endfor
    };
    return items;
}
skr::StringView EnumTraits<${enum.name}>::to_string(const ${enum.name}& value)
{
    switch (value)
    {
    %for name, value in enum.values.items():
    case ${enum.name}::${value.short_name}: return u8"${value.short_name}";
    %endfor
    default: SKR_UNREACHABLE_CODE(); return u8"${enum.name}::INVALID_ENUMERATOR";
    }
}
bool EnumTraits<${enum.name}>::from_string(skr::StringView str, ${enum.name}& value)
{
    const auto hash = skr_hash64(str.raw().data(), str.size(), 0);
    switch(hash)
    {
    %for name, value in enum.values.items():
        case skr::consteval_hash(u8"${value.short_name}"): if(str == u8"${value.short_name}") value = ${name}; return true;
    %endfor
        default:
            return false;
    }
}
%endfor
}

SKR_RTTR_EXEC_STATIC
{
    using namespace ::skr::rttr;

%for record in module_db.get_records():
%if record.attrs.rttr.enable:
    static struct InternalTypeLoader_${record.name.replace("::", "_")} : public TypeLoader
    {
        Type* create() override {
            return SkrNew<RecordType>(
                RTTRTraits<::${record.name}>::get_name(),
                RTTRTraits<::${record.name}>::get_guid(),
                sizeof(${record.name}),
                alignof(${record.name}),
                make_record_basic_method_table<${record.name}>()
            );
        }

        void load(Type* type) override 
        {
            using namespace skr;
            using namespace skr::rttr;

            [[maybe_unused]] RecordType* result = static_cast<RecordType*>(type);

        %if record.bases:
            result->set_base_types({
            %for base in record.bases:
                {RTTRTraits<${base}>::get_guid(), {RTTRTraits<${base}>::get_type(), +[](void* p) -> void* { return static_cast<${base}*>(reinterpret_cast<${record.name}*>(p)); }}},
            %endfor
            });
        %endif

        %if record.fields:
            result->set_fields({
            %for name, field in record.fields:
            %if field.attrs.rttr.enable:
                {u8"${name}", {u8"${name}", RTTRTraits<${field.type}>::get_type(), ${field.offset}}},
            %endif
            %endfor
            });
        %endif

        %if record.methods:
            result->set_methods({
            %for method in record.methods:
            %if method.attrs.rttr.enable:
                {
                    u8"${method.short_name}",
                    {
                        u8"${method.short_name}",
                        RTTRTraits<${method.retType}>::get_type(),
                        {
                        %for name, parameter in method.parameters.items():
                            {u8"${name}", RTTRTraits<${parameter.type}>::get_type()},
                        %endfor
                        },
                        +[](void* self, void* parameters, void* return_value)
                        {
                            <%  
                                parameters_tuple = ""
                                if len(method.parameters) == 1:
                                    parameters_tuple = "tuple<%s>" % method.parameters.items()[0].type
                                elif len(method.parameters) > 1:
                                    parameters_tuple = "tuple<%s>" % ", ".join([parameter.type for name, parameter in method.parameters.items()]) 
                                invoke_expr = "{obj}->{method}({args});".format(
                                    obj = "reinterpret_cast<%s*>(self)" % record.name,
                                    method = method.short_name,
                                    args = ", ".join(["get<%d>(params)" % i for i in range(len(method.parameters))])
                                )
                            %>
                        %if len(method.parameters) >= 1:
                            ${parameters_tuple}& params = *reinterpret_cast<${parameters_tuple}*>(parameters);
                        %endif
                        %if method.retType == "void":
                            ${invoke_expr}
                        %else:
                            if (return_value)
                            {
                                *((${method.retType}*)return_value) = ${invoke_expr}
                            }
                            else
                            {
                                ${invoke_expr}
                            }
                        %endif
                        }
                    }
                },
            %endif
            %endfor
            });
        %endif
        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__${record.name.replace("::", "_")};
    register_type_loader(RTTRTraits<${record.name}>::get_guid(), &LOADER__${record.name.replace("::", "_")});
%endif
%endfor

%for enum in module_db.get_enums():
%if enum.attrs.rttr.enable:
    static EnumTypeFromTraitsLoader<${enum.name}> LOADER__${enum.name.replace("::", "_")};
    register_type_loader(RTTRTraits<${enum.name}>::get_guid(), &LOADER__${enum.replace("::", "_")});
%endif
%endfor
};
// END RTTR GENERATED
