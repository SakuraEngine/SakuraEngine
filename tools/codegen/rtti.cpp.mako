// BEGIN RTTI GENERATED
#include "type/type.hpp"
#include "platform/debug.h"
#include "utils/hash.h"
#include "utils/log.h"
#include "type/type_helper.hpp"

%for record in generator.filter_rtti(db.records):
namespace skr::type
{
%if hasattr(record.attrs, "hashable"):
    size_t Hash(const ${record.name}& value, size_t base)
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
    const skr_type_t* type_of<${record.name}>::get()
    {
        static bool initialized = false;
        if(!initialized)
        {
            initialized = true;
            size_t size = sizeof(${record.name});
            size_t align = alignof(${record.name});
            skr::string_view name = "${record.name}";
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
                +[](const void* self, size_t base) { return Hash(*(const ${record.name}*)self, base); }, //hash
        %else:
                nullptr, //hash
        %endif
                GetSerialize<${record.name}>(),
                GetDeserialize<${record.name}>(),
                GetDeleter<${record.name}>(),
                GetTextSerialize<${record.name}>(),
                GetTextDeserialize<${record.name}>(),
            };
        <%
            fields = [(name, field) for name, field in vars(record.fields).items() if not hasattr(field.attrs, "no-rtti")]
            methods = [method for method in record.methods if not hasattr(method.attrs, "no-rtti")]
        %>
        %if fields:
            static skr_field_t fields[] = {
            %for name, field in fields:
                {"${name}", type_of<${field.type}>::get(), ${field.offset}},
            %endfor
            };
        %else:
            static skr::span<skr_field_t> fields;
        %endif
        
        %for i, method in enumerate(generator.filter_rtti(methods)):
            %if vars(method.parameters):
                static skr_field_t _params${i}[] = {
                %for name, field in vars(method.parameters).items():
                    { "${name}", type_of<${field.type}>::get(), ${field.offset}},
                %endfor
                };
                static skr::span<skr_field_t> params${i} = _params${i};
            %else:
                static skr::span<skr_field_t> params${i};
            %endif
        %endfor
        
        %if methods:
        static skr_method_t methods[] = {
        %for i, method in enumerate(generator.filter_rtti(methods)):
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
                        SKR_LOG_ERROR("[${method.name}] not enough arguments provided.");
                        return result;
                    }
                %for j, (name, parameter) in enumerate(vars(method.parameters).items()):
                    if(!args[${j}].type->Convertible(type_of<${parameter.type}>::get()))
                    {
                        SKR_LOG_ERROR("[${method.name}] argument ${name} is not compatible.");
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
            constexpr skr_guid_t guid = {${db.guid_constant(record)}};
            static RecordType type(size, align, name, guid, skr::is_object_v<${record.name}>, base, nativeMethods, fields, methods);
            type_of_${record.id} = &type;
        }
        return type_of_${record.id};
    }
}
static struct RegisterRTTI${record.id}Helper
{
    RegisterRTTI${record.id}Helper()
    {
        using namespace skr::type;
        auto registry = GetTypeRegistry();
        constexpr skr_guid_t guid = {${db.guid_constant(record)}};
        registry->register_type(guid, type_of<${record.name}>::get());
    }
} _RegisterRTTI${record.id}Helper;

%endfor

%for enum in generator.filter_rtti(db.enums): 

namespace skr::type
{
    const skr_type_t* type_of<${enum.name}>::get()
    {
        static EnumType::Enumerator enumerators[] = 
        {
        %for name, enumerator in vars(enum.values).items():
            {"${db.short_name(name)}", ${enumerator.value}},
        %endfor
        };
        constexpr skr_guid_t guid = {${db.guid_constant(enum)}};
        static EnumType type{
            type_of<std::underlying_type_t<${enum.name}>>::get(),
            "${enum.name}", guid,
            +[](void* self, skr::string_view enumStr)
            {
                auto& This = *((${enum.name}*)self);

                auto hash = hash_crc32<char>({enumStr.data(), enumStr.size()});
                switch(hash)
                {
                %for name, enumerator in vars(enum.values).items():
                    case hash_crc32<char>("${db.short_name(name)}"): if( enumStr.compare("${db.short_name(name)}") == 0) This = ${name}; return;
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
                    case ${name}: return skr::string("${db.short_name(name)}");
                %endfor
                }
                SKR_UNREACHABLE_CODE();
                return skr::string("${enum.name}::INVALID_ENUMERATOR");
            },
            enumerators
        };
        return &type;
    }
}
    static struct RegisterRTTI${enum.id}Helper
    {
        RegisterRTTI${enum.id}Helper()
        {
            using namespace skr::type;
            auto registry = GetTypeRegistry();
            constexpr skr_guid_t guid = {${db.guid_constant(enum)}};
            registry->register_type(guid, type_of<${enum.name}>::get());
        }
    } _RegisterRTTI${enum.id}Helper;
    
%endfor
//END RTTI GENERATED