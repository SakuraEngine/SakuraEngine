<%
records = generator.filter_rtti(db.records)
enums = generator.filter_rtti(db.enums)
%>

// BEGIN RTTI GENERATED
#include "SkrRT/type/type.hpp"
#include "SkrRT/platform/debug.h"
#include "SkrRT/misc/hash.h"
#include "SkrRT/misc/log.h"
#include "SkrRT/type/type_helper.hpp"

[[maybe_unused]] static const char8_t* ArgumentNumMisMatchFormat = u8"Argument number mismatch while calling %s, expected %d, got %d.";
[[maybe_unused]] static const char8_t* ArgumentIncompatibleFormat = u8"Argument %s is incompatible while calling %s. %s can not be converted to %s.";

%for record in records:
namespace skr::type
{
%if hasattr(record.attrs, "hashable"):
    uint64_t Hash(const ${record.name}& value, uint64_t base)
    {
    %for base in record.bases:
        base = Hash(static_cast<const ${base.name}&>(value), base);
    %endfor
    %for name, field in vars(record.fields).items():
        base = Hash(value.${name}, base);
    %endfor
        return base;
    }
%endif
    static skr_type_t* type_of_${record.id};
    void type_register<${record.name}>::instantiate_type(RecordType* type)
    {
        constexpr skr_guid_t guid = {${db.guid_constant(record)}};
        if(type_of_${record.id} == nullptr)
        {
            using namespace skr::type;
            type_of_${record.id} = type;
            size_t size = sizeof(${record.name});
            size_t align = alignof(${record.name});
            skr::string_view name = u8"${record.name}";
        %if record.bases:
            auto base = (const RecordType*)type_of<${record.bases[0]}>::get();
        %else:
            auto base = (const RecordType*)nullptr;
        %endif
            ObjectMethodTable nativeMethods {
                +[](void* self) { ((${record.name}*)self)->~${record.short_name}(); }, //dtor
                GetDefaultCtor<${record.name}>(), //ctor
                GetCopyCtor<${record.name}>(),
                GetMoveCtor<${record.name}>(),
        %if hasattr(record.attrs, "hashable"):
                +[](const void* self, uint64_t base) { return Hash(*(const ${record.name}*)self, base); }, //hash
        %else:
                nullptr, //hash
        %endif
                GetSerialize<${record.name}>(),
                GetDeserialize<${record.name}>(),
                GetJsonSerialize<${record.name}>(),
                GetJsonDeserialize<${record.name}>(),
            };
        <%
            fields = [(name, field) for name, field in vars(record.fields).items() if not hasattr(field.attrs, "no-rtti")]
            methods = [method for method in record.methods if hasattr(method.attrs, "rtti")]
        %>
        %if fields:
            static skr_field_t fields[] = {
            %for name, field in fields:
                {u8"${name}", type_of<${field.type}>::get(), ${field.offset}},
            %endfor
            };
        %else:
            static skr::span<skr_field_t> fields;
        %endif
        %for i, method in enumerate(methods):
            %if vars(method.parameters):
                static skr_field_t _params${i}[] = {
                %for name, field in vars(method.parameters).items():
                    { u8"${name}", type_of<${field.type}>::get(), ${field.offset}},
                %endfor
                };
                static skr::span<skr_field_t> params${i} = _params${i};
            %else:
                static skr::span<skr_field_t> params${i};
            %endif
        %endfor
        
        %if methods:
        static skr_method_t methods[] = {
        %for i, method in enumerate(methods):
            {
                "${db.short_name(method.name)}", 
            %if method.retType == "void":
                nullptr,
            %else:
                type_of<${method.retType}>::get(), 
            %endif
                params${i}.data(),
                +[](void* self, struct skr_value_ref_t* args, size_t nargs)
                {   
                    skr_value_t result = {};
                    if(nargs < ${len(vars(method.parameters))})
                    {
                        SKR_LOG_ERROR(ArgumentNumMisMatchFormat, "${method.name}", ${len(vars(method.parameters))}, nargs);
                        return result;
                    }
                %for j, (name, parameter) in enumerate(vars(method.parameters).items()):
                    if(!args[${j}].type->Convertible(type_of<${parameter.type}>::get()))
                    {
                        SKR_LOG_ERROR(ArgumentIncompatibleFormat, "${name}", "${method.name}", args[${j}].type->Name(), "${parameter.type}");
                        return result;
                    }
                %endfor
            %if method.retType != "void":
                    result.Emplace<${method.retType}>(((${record.name}*)self)->${db.short_name(method.name)}(${db.call_expr(method)}));
            %else:
                    ((${record.name}*)self)->${db.short_name(method.name)}(${db.call_expr(method)});
            %endif
                    return result;
                }
            },
        %endfor
            };
        %else:
            static skr::span<skr_method_t> methods;
        %endif
            type->initialize(size, align, name, skr::is_object_v<${record.name}>, base, nativeMethods, fields, methods);
        }
    }
}
static struct RegisterRTTI${record.id}Helper
{
    RegisterRTTI${record.id}Helper()
    {
        (void)skr::type::type_of<${record.name}>::get();
    }
} _RegisterRTTI${record.id}Helper;

%endfor

%for enum in enums: 

namespace skr::type
{
    static skr_type_t* type_of_${enum.id};
    void type_register<${enum.name}>::instantiate_type(EnumType* type)
    {
        constexpr skr_guid_t guid = {${db.guid_constant(enum)}};
        if(type_of_${enum.id} == nullptr)
        {
            using namespace skr::type;
            type_of_${enum.id} = type;
            static EnumType::Enumerator enumerators[] = 
            {
            %for name, enumerator in vars(enum.values).items():
                {u8"${db.short_name(name)}", ${enumerator.value}},
            %endfor
            };
            type->initialize(
                type_of<std::underlying_type_t<${enum.name}>>::get(),
                u8"${enum.name}",
                +[](void* self, skr::string_view enumStr)
                {
                    auto& This = *((${enum.name}*)self);
                    auto hash = hash_crc32<char8_t>({enumStr.raw().data(), (size_t)enumStr.size()});
                    switch(hash)
                    {
                    %for name, enumerator in vars(enum.values).items():
                        case hash_crc32<char>("${db.short_name(name)}"): if(enumStr == u8"${db.short_name(name)}") This = ${name}; return;
                    %endfor
                    }
                    SKR_UNREACHABLE_CODE();
                },
                +[](const void* self)
                {
                    auto& This = *((const ${enum.name}*)self);
                    switch(This)
                    {
                    %for name, enumerator in vars(enum.values).items():
                        case ${name}: return skr::string(u8"${db.short_name(name)}");
                    %endfor
                    }
                    SKR_UNREACHABLE_CODE();
                    return skr::string(u8"${enum.name}::INVALID_ENUMERATOR");
                },
                enumerators
            );
        }
    }
}
static struct RegisterRTTI${enum.id}Helper
{
    RegisterRTTI${enum.id}Helper()
    {
        (void)skr::type::type_of<${enum.name}>::get();
    }
} _RegisterRTTI${enum.id}Helper;
%endfor

skr::span<const skr_type_t*> skr_get_all_records_${module}()
{
%if records:
    const skr_type_t* types[${len(records)}] = {
    %for record in records:
        skr::type::type_of<${record.name}>::get(),
    %endfor
    };
    return {types};
%else:
    return {};
%endif
}

skr::span<const skr_type_t*> skr_get_all_enums_${module}()
{
%if enums:
    const skr_type_t* types[${len(enums)}] = {
    %for enum in enums:
        skr::type::type_of<${enum.name}>::get(),
    %endfor
    };
    return {types};
%else:
    return {};
%endif
}

//END RTTI GENERATED


// BEGIN RTTR GENERATED
#include "SkrRT/rttr/type_loader/type_loader.hpp"
#include "SkrRT/rttr/type/record_type.hpp"
#include "SkrRT/rttr/exec_static.hpp"
#include "SkrRT/rttr/type_loader/enum_type_from_traits_loader.hpp"
#include "SkrRT/containers_new/tuple.hpp"

namespace skr::rttr 
{
%for enum in generator.filter_rtti(db.enums):
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
    const std::string_view enumStrV = {(const char*)str.raw().data(), (size_t)str.size()};
    const auto hash = hash_crc32(enumStrV);
    switch(hash)
    {
    %for name, value in vars(enum.values).items():
        case hash_crc32<char>("${db.short_name(name)}"): if(str == u8"${db.short_name(name)}") value = ${name}; return true;
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

%for record in records:
        <%  
            bases = record.bases
            fields = [(name, field) for name, field in vars(record.fields).items() if not hasattr(field.attrs, "no-rtti")]
            methods = [method for method in record.methods if hasattr(method.attrs, "rtti")]
        %>
    static struct InternalTypeLoader_${record.id} : public TypeLoader
    {
        Type* load() override 
        {
            using namespace skr;
            using namespace skr::rttr;

            RecordType* result = SkrNew<RecordType>(
                RTTRTraits<::${record.name}>::get_name(),
                RTTRTraits<::${record.name}>::get_guid(),
                sizeof(${record.name}),
                alignof(${record.name}),
                make_record_basic_method_table<${record.name}>()
            );

        %if bases:
            UMap<GUID, BaseInfo> base_types = {
            %for base in bases:
                {RTTRTraits<${base}>::get_guid(), {RTTRTraits<${base}>::get_type(), get_cast_offset<${record.name}, ${base}>()}},
            %endfor
            };
            result->set_base_types(base_types);
        %endif

        %if fields:
            MultiUMap<string, Field> fields = {
            %for name, field in fields:
                {u8"${name}", {u8"${name}", RTTRTraits<${field.type}>::get_type(), ${field.offset}}},
            %endfor
            };
            result->set_fields(fields);
        %endif

        %if methods:
            MultiUMap<string, Method> methods = {
            %for method in methods:
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
                            ${parameters_tuple}& params = *reinterpret_cast<${parameters_tuple}*>(parameters);
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
            };
            result->set_methods(methods);
        %endif

            return result;
        }
        void destroy(Type* type) override
        {
            SkrDelete(type);
        }
    } LOADER__${record.id};
    register_type_loader(RTTRTraits<${record.name}>::get_guid(), &LOADER__${record.id});
%endfor

%for enum in generator.filter_rtti(db.enums):
    static EnumTypeFromTraitsLoader<${enum.name}> LOADER__${enum.id};
    register_type_loader(RTTRTraits<${enum.name}>::get_guid(), &LOADER__${enum.id});
%endfor
};
// END RTTR GENERATED
