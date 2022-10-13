//DO NOT MODIFY THIS FILE
//generated from rtti.cpp.mako
#include "type/type_registry.h"
#include "platform/debug.h"
#include "utils/hash.h"
#include "utils/log.h"
%for header in db.headers:
#include "${header}"
%endfor

%for record in db.records:
namespace skr::type
{
%if record.hashable:
    size_t Hash(const ${record.name}& value, size_t base)
    {
    %for field in record.allFields():
        base = Hash(value.${field.name}, base);
    %endfor
        return base;
    }
%endif
    static skr_type_t* type_of_${record.id};
    const skr_type_t* type_of<${record.name}>::get()
    {
        static struct DoOnce
        {
            DoOnce()
            {
                size_t size = sizeof(${record.name});
                size_t align = alignof(${record.name});
                eastl::string_view name = "${record.name}";
            %if record.bases:
                auto base = (const RecordType*)type_of<${record.bases[0].name}>::get();
            %else:
                auto base = (const RecordType*)nullptr;
            %endif
                ObjectMethodTable nativeMethods {
                    +[](void* self) { ((${record.name}*)self)->~${record.short_name}(); }, //dtor
                    +[](void* self, struct skr_value_t* param, size_t nparam) { /*TODO*/ }, //ctor
                    GetCopyCtor<${record.name}>(),
                    GetMoveCtor<${record.name}>(),
        %if record.hashable:
                    +[](const void* self, size_t base) { return Hash(*(const ${record.name}*)self, base); }, //hash
        %else:
                    nullptr, //hash
        %endif
                };
            %if record.fields:
                static skr_field_t fields[] = {
                %for field in record.fields:
                    {"${field.name}", type_of<${field.type}>::get(), ${field.offset}},
                %endfor
                };
            %else:
                static gsl::span<skr_field_t> fields;
            %endif
            %for method in record.methods.values():
            %if record.methods:
                static skr_field_t ${method.short_name}Params[] = {
                %for field in method.descs[0].fields:
                    { "${field.name}", type_of<${field.type}>::get(), ${field.offset}},
                %endfor
                };
            %else:
                static gsl::span<skr_field_t> ${method.short_name}Params;
            %endif
            %endfor
            %if record.methods:
                static skr_method_t methods[] = {
                %for method in record.methods.values():
                    {
                        "${method.short_name}", 
                    %if method.descs[0].retType == "void":
                        nullptr,
                    %else:
                        type_of<${method.descs[0].retType}>::get(), 
                    %endif
                        ${method.short_name}Params,
                        +[](void* self, struct skr_value_ref_t* args, size_t nargs)
                        {   
                            skr_value_t result = {};
                            if(nargs < ${len(method.descs[0].fields)})
                            {
                                SKR_LOG_ERROR("[${method.name}] not enough arguments provided.");
                                return result;
                            }
                        %for i, field in enumerate(method.descs[0].fields):
                            if(!args[${i}].type->Convertible(type_of<${field.type}>::get()))
                            {
                                SKR_LOG_ERROR("[${method.name}] argument ${field.name} is not compatible.");
                                return result;
                            }
                        %endfor
                    %if method.descs[0].retType != "void":
                            result.Emplace<${method.descs[0].retType}>(((${record.name}*)self)->${method.short_name}(${method.descs[0].getCall()}));
                    %else:
                            ((${record.name}*)self)->${method.short_name}(${method.descs[0].getCall()});
                    %endif
                            return result;
                        }
                    },
                %endfor
                };
            %else:
                static gsl::span<skr_method_t> methods;
            %endif
                constexpr skr_guid_t guid = {${record.guidConstant}};
                static RecordType type(size, align, name, guid, base, nativeMethods, fields, methods);
                type_of_${record.id} = &type;
            }
        } once;
        return type_of_${record.id};
    }
}
    static struct RegisterRTTI${record.id}Helper
    {
        RegisterRTTI${record.id}Helper()
        {
            using namespace skr::type;
            auto registry = GetTypeRegistry();
            constexpr skr_guid_t guid = {${record.guidConstant}};
            registry->register_type(guid, type_of<${record.name}>::get());
        }
    } _RegisterRTTI${record.id}Helper;
%endfor

%for enum in db.enums: 
namespace skr::type
{
    const skr_type_t* type_of<${enum.name}>::get()
    {
        static EnumType::Enumerator enumerators[] = 
        {
        %for enumerator in enum.enumerators:
            {"${enumerator.short_name}", ${enumerator.value}},
        %endfor
        };
        constexpr skr_guid_t guid = {${enum.guidConstant}};
        static EnumType type{
            type_of<std::underlying_type_t<${enum.name}>>::get(),
            "${enum.name}", guid,
            +[](void* self, eastl::string_view enumStr)
            {
                auto& This = *((${enum.name}*)self);

                auto hash = hash_crc32<char>({enumStr.data(), enumStr.size()});
                switch(hash)
                {
                %for enumerator in enum.enumerators:
                    case hash_crc32<char>("${enumerator.short_name}"): if( enumStr.compare("${enumerator.short_name}") == 0) This = ${enumerator.name}; return;
                %endfor
                }
                SKR_UNREACHABLE_CODE();
            },
            +[](const void* self)
            {
                auto& This = *((const ${enum.name}*)self);
                switch(This)
                {
                %for enumerator in enum.enumerators:
                    case ${enumerator.name}: return eastl::string("${enumerator.short_name}");
                %endfor
                }
                SKR_UNREACHABLE_CODE();
                return eastl::string("${enum.name}::Unknown");
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
            constexpr skr_guid_t guid = {${enum.guidConstant}};
            registry->register_type(guid, type_of<${enum.name}>::get());
        }
    } _RegisterRTTI${enum.id}Helper;
%endfor