// BEGIN RTTR GENERATED
#include "SkrRT/rttr/type_loader/type_loader.hpp"
#include "SkrRT/rttr/type/record_type.hpp"
#include "SkrRT/rttr/exec_static.hpp"
#include "SkrRT/rttr/type_loader/enum_type_from_traits_loader.hpp"
#include "SkrRT/containers_new/tuple.hpp"

namespace skr::rttr 
{
%for enum in generator.enums:
Span<EnumItem<${enum.name}>> EnumTraits<${enum.name}>::items()
{
    static EnumItem<${enum.name}> items[] = {
    %for name, value in vars(enum.values).items():
        {u8"${db.short_name(name)}", ${name}},
    %endfor
    };
    return items;
}
skr::string_view EnumTraits<${enum.name}>::to_string(const ${enum.name}& value)
{
    switch (value)
    {
    %for name, value in vars(enum.values).items():
    case ${enum.name}::${db.short_name(name)}: return u8"${db.short_name(name)}";
    %endfor
    default: SKR_UNREACHABLE_CODE(); return u8"${enum.name}::INVALID_ENUMERATOR";
    }
}
bool EnumTraits<${enum.name}>::from_string(string_view str, ${enum.name}& value)
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
}

SKR_RTTR_EXEC_STATIC
{
    using namespace ::skr::rttr;

%for record in generator.records:
    static struct InternalTypeLoader_${record.id} : public TypeLoader
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

            RecordType* result = static_cast<RecordType*>(type);

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
                {u8"${name}", {u8"${name}", RTTRTraits<${field.type}>::get_type(), ${field.offset}}},
            %endfor
            });
        %endif

        %if record.methods:
            result->set_methods({
            %for method in record.methods:
                {
                    u8"${db.short_name(method.name)}",
                    {
                        u8"${db.short_name(method.name)}",
                        RTTRTraits<${method.retType}>::get_type(),
                        {
                        %for name, parameter in vars(method.parameters).items():
                            {u8"${name}", RTTRTraits<${parameter.type}>::get_type()},
                        %endfor
                        },
                        +[](void* self, void* parameters, void* return_value)
                        {
                            <%  
                                parameters_tuple = ""
                                if len(vars(method.parameters)) == 1:
                                    parameters_tuple = "tuple<%s>" % vars(method.parameters).items()[0].type
                                elif len(vars(method.parameters)) > 1:
                                    parameters_tuple = "tuple<%s>" % ", ".join([parameter.type for name, parameter in vars(method.parameters).items()]) 
                                invoke_expr = "{obj}->{method}({args});".format(
                                    obj = "reinterpret_cast<%s*>(self)" % record.name,
                                    method = db.short_name(method.name),
                                    args = ", ".join(["get<%d>(params)" % i for i in range(len(vars(method.parameters)))])
                                )
                            %>
                        %if len(vars(method.parameters)) >= 1:
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
            %endfor
            });
        %endif
        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__${record.id};
    register_type_loader(RTTRTraits<${record.name}>::get_guid(), &LOADER__${record.id});
%endfor

%for enum in generator.enums:
    static EnumTypeFromTraitsLoader<${enum.name}> LOADER__${enum.id};
    register_type_loader(RTTRTraits<${enum.name}>::get_guid(), &LOADER__${enum.id});
%endfor
};
// END RTTR GENERATED
