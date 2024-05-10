#pragma once
#include "SkrContainers/optional.hpp"
#include "SkrContainers/vector.hpp"
#include "SkrGuid/guid.hpp"
#include "SkrRTTR/rttr_traits.hpp"
#include "SkrRTTR/type_signature.hpp"
#include "SkrRTTR/enum_value.hpp"

namespace skr::rttr
{
enum class ParamModifier
{
    In,   // default
    Out,  // for reference or pointer
    Inout // for reference or pointer
};

struct ParamData {
    using MakeDefaultFunc = void (*)(void*);

    // signature
    String          name         = {};
    TypeSignature   type         = {};
    ParamModifier   modifier     = ParamModifier::In;
    MakeDefaultFunc make_default = nullptr;

    // TODO. meta data

    template <typename Arg>
    inline static ParamData Make()
    {
        return {
            {},
            type_signature_of<Arg>(),
            ParamModifier::In,
            nullptr
        };
    }
};

template <typename Data>
inline bool export_function_signature_equal(const Data& data, TypeSignature signature, ETypeSignatureCompareFlag flag)
{
    SKR_ASSERT(signature.view().is_complete());
    SKR_ASSERT(signature.view().is_function());

    // read signature data
    uint32_t param_count;
    auto     view = signature.view().read_function_signature(param_count);
    if (param_count != data.param_data.size()) { return false; }

    // compare return type
    auto ret_view = view.jump_next_type_or_data();
    if (!data.ret_type.view().equal(ret_view, flag)) { return false; }

    // compare param type
    for (uint32_t i = 0; i < param_count; ++i)
    {
        auto param_view = view.jump_next_type_or_data();
        if (!data.param_data[i].type.view().equal(param_view, flag)) { return false; }
    }

    return true;
}

struct FunctionData {
    // signature
    String            name;
    Vector<String>    name_space;
    TypeSignature     ret_type;
    Vector<ParamData> param_data;

    // [Provided by export Backend]
    void* invoke;

    // TODO. meta data

    template <typename Ret, typename... Args>
    inline void fill_signature(Ret (*)(Args...))
    {
        ret_type   = type_signature_of<Ret>();
        param_data = { ParamData::Make<Args>()... };
    }

    inline bool signature_equal(TypeSignature signature, ETypeSignatureCompareFlag flag) const
    {
        return export_function_signature_equal(*this, signature, flag);
    }
};

enum class EAccessLevel : uint8_t
{
    Public,
    Protected,
    Private
};

struct MethodData {
    // signature
    String            name;
    TypeSignature     ret_type;
    Vector<ParamData> param_data;
    bool              is_const;
    EAccessLevel      access_level;

    // [Provided by export Backend]
    void* invoke;

    // TODO. meta data

    template <class T, typename Ret, typename... Args>
    inline void fill_signature(Ret (T::*)(Args...))
    {
        ret_type   = type_signature_of<Ret>();
        param_data = { ParamData::Make<Args>()... };
        is_const   = false;
    }
    template <class T, typename Ret, typename... Args>
    inline void fill_signature(Ret (T::*)(Args...) const)
    {
        ret_type   = type_signature_of<Ret>();
        param_data = { ParamData::Make<Args>()... };
        is_const   = true;
    }

    inline bool signature_equal(TypeSignature signature, ETypeSignatureCompareFlag flag) const
    {
        return export_function_signature_equal(*this, signature, flag);
    }
};

struct FieldData {
    using GetAddressFunc = void* (*)(void*);

    // signature
    String         name;
    TypeSignature  type;
    GetAddressFunc get_address;
    EAccessLevel   access_level;

    // [Provided by export Backend]
    void* getter;
    void* setter;

    // TODO. meta data

    template <auto field, class T, typename Field>
    inline void fill_signature(Field T::*)
    {
        type        = type_signature_of<Field>();
        get_address = +[](void* p) -> void* {
            return &(reinterpret_cast<T*>(p)->*field);
        };
    }
};

struct StaticMethodData {
    // signature
    String            name;
    TypeSignature     ret_type;
    Vector<ParamData> param_data;
    EAccessLevel      access_level;

    // [Provided by export Backend]
    void* invoke;

    // TODO. meta data

    template <typename Ret, typename... Args>
    inline void fill_signature(Ret (*)(Args...))
    {
        ret_type   = type_signature_of<Ret>();
        param_data = { ParamData::Make<Args>()... };
    }

    inline bool signature_equal(TypeSignature signature, ETypeSignatureCompareFlag flag) const
    {
        return export_function_signature_equal(*this, signature, flag);
    }
};

struct StaticFieldData {
    // signature
    String        name;
    TypeSignature type;
    void*         address;
    EAccessLevel  access_level;

    // [Provided by export Backend]
    void* getter;
    void* setter;

    // TODO. meta data

    template <typename T>
    inline void fill_signature(T* p_field)
    {
        type = type_signature_of<T>();
    }
};

struct ExternMethodData {
    // signature
    String            name;
    TypeSignature     ret_type;
    Vector<ParamData> param_data;

    // [Provided by export Backend]
    void* invoke;

    // TODO. meta data

    template <typename Ret, typename... Args>
    inline void fill_signature(Ret (*)(Args...))
    {
        ret_type   = type_signature_of<Ret>();
        param_data = { ParamData::Make<Args>()... };
    }

    inline bool signature_equal(TypeSignature signature, ETypeSignatureCompareFlag flag) const
    {
        return export_function_signature_equal(*this, signature, flag);
    }
};

struct BaseData {
    using CastFunc = void* (*)(void*);

    GUID         type_id;
    CastFunc     cast;
    EAccessLevel access_level;

    template <typename T, typename Base>
    inline static BaseData Make()
    {
        return {
            RTTRTraits<Base>::get_guid(),
            +[](void* p) -> void* {
                return static_cast<Base*>(reinterpret_cast<T*>(p));
            }
        };
    }
};

struct CtorData {
    // signature
    Vector<ParamData> param_data;
    EAccessLevel      access_level;

    // [Provided by export Backend]
    void* invoke;

    // TODO. meta data

    template <typename... Args>
    inline void fill_signature()
    {
        param_data = { ParamData::Make<Args>()... };
    }

    inline bool signature_equal(TypeSignature signature, ETypeSignatureCompareFlag flag) const
    {
        SKR_ASSERT(signature.view().is_complete());
        SKR_ASSERT(signature.view().is_function());

        // read signature data
        uint32_t param_count;
        auto     view = signature.view().read_function_signature(param_count);
        if (param_count != param_data.size()) { return false; }

        // compare return type
        auto                     ret_view = view.jump_next_type_or_data();
        TypeSignatureTyped<void> ret_type;
        if (!ret_type.view().equal(ret_view, flag)) { return false; }

        // compare param type
        for (uint32_t i = 0; i < param_count; ++i)
        {
            auto param_view = view.jump_next_type_or_data();
            if (!param_data[i].type.view().equal(param_view, flag)) { return false; }
        }

        return true;
    }
};

struct DtorData {
    EAccessLevel access_level;

    // [Provided by export Backend]
    void* invoke;
};

struct RecordData {
    // basic
    String         name;
    Vector<String> name_space;
    GUID           type_id;
    size_t         size;
    size_t         alignment;

    // bases
    Vector<BaseData> bases_data;

    // ctor & dtor
    Vector<CtorData> ctor_data;
    DtorData         dtor_data;

    // method & fields
    Vector<MethodData> methods;
    Vector<FieldData>  fields;

    // static method & static fields
    Vector<StaticMethodData> static_methods;
    Vector<StaticFieldData>  static_fields;

    // extern method
    Vector<ExternMethodData> extern_methods;

    // TODO. meta data

    // signature find
    inline const CtorData* find_ctor(TypeSignatureView signature, ETypeSignatureCompareFlag flag) const
    {
        return ctor_data.find_if([&](const CtorData& ctor) {
                            return ctor.signature_equal(signature, flag);
                        })
            .ptr();
    }
    inline const MethodData* find_method(TypeSignatureView signature, StringView name, ETypeSignatureCompareFlag flag) const
    {
        return methods.find_if([&](const MethodData& method) {
                          return method.name == name && method.signature_equal(signature, flag);
                      })
            .ptr();
    }
    inline const FieldData* find_field(TypeSignatureView signature, StringView name, ETypeSignatureCompareFlag flag) const
    {
        return fields.find_if([&](const FieldData& field) {
                         return field.name == name && field.type.view().equal(signature, flag);
                     })
            .ptr();
    }
    inline const StaticMethodData* find_static_method(TypeSignatureView signature, StringView name, ETypeSignatureCompareFlag flag) const
    {
        return static_methods.find_if([&](const StaticMethodData& method) {
                                 return method.name == name && method.signature_equal(signature, flag);
                             })
            .ptr();
    }
    inline const StaticFieldData* find_static_field(TypeSignatureView signature, StringView name, ETypeSignatureCompareFlag flag) const
    {
        return static_fields.find_if([&](const StaticFieldData& field) {
                                return field.name == name && field.type.view().equal(signature, flag);
                            })
            .ptr();
    }
    inline const ExternMethodData* find_extern_method(TypeSignatureView signature, StringView name, ETypeSignatureCompareFlag flag) const
    {
        return extern_methods.find_if([&](const ExternMethodData& method) {
                                 return method.name == name && method.signature_equal(signature, flag);
                             })
            .ptr();
    }

    // template find
    template <typename Func>
    inline Optional<const CtorData*> find_ctor(ETypeSignatureCompareFlag flag) const
    {
        TypeSignatureTyped<Func> signature;
        return find_ctor(signature.view(), flag);
    }
    template <typename Func>
    inline Optional<const MethodData*> find_method(StringView name, ETypeSignatureCompareFlag flag) const
    {
        TypeSignatureTyped<Func> signature;
        return find_method(signature.view(), name, flag);
    }
    template <typename Func>
    inline Optional<const FieldData*> find_field(StringView name, ETypeSignatureCompareFlag flag) const
    {
        TypeSignatureTyped<Func> signature;
        return find_field(signature.view(), name, flag);
    }
    template <typename Func>
    inline Optional<const StaticMethodData*> find_static_method(StringView name, ETypeSignatureCompareFlag flag) const
    {
        TypeSignatureTyped<Func> signature;
        return find_static_method(signature.view(), name, flag);
    }
    template <typename Func>
    inline Optional<const StaticFieldData*> find_static_field(StringView name, ETypeSignatureCompareFlag flag) const
    {
        TypeSignatureTyped<Func> signature;
        return find_static_field(signature.view(), name, flag);
    }
    template <typename Func>
    inline Optional<const ExternMethodData*> find_extern_method(StringView name, ETypeSignatureCompareFlag flag) const
    {
        TypeSignatureTyped<Func> signature;
        return find_extern_method(signature.view(), name, flag);
    }
};

struct EnumItemExport {
    String    name;
    EnumValue value;

    // TODO. meta data
};
struct EnumData {
    // basic
    String         name;
    Vector<String> name_space;
    GUID           type_id;
    size_t         size;
    size_t         alignment;

    // underlying type
    GUID underlying_type_id;

    // items
    Vector<EnumItemExport> items;

    // extern method
    Vector<ExternMethodData> extern_methods;

    // TODO. meta data

    // signature find
    inline const ExternMethodData* find_extern_method(TypeSignatureView signature, StringView name, ETypeSignatureCompareFlag flag) const
    {
        return extern_methods.find_if([&](const ExternMethodData& method) {
                                 return method.name == name && method.signature_equal(signature, flag);
                             })
            .ptr();
    }

    // template find
    template <typename Func>
    inline Optional<const ExternMethodData*> find_extern_method(StringView name, ETypeSignatureCompareFlag flag) const
    {
        TypeSignatureTyped<Func> signature;
        return find_extern_method(signature.view(), name, flag);
    }
};

struct PrimitiveData {
    // basic
    String name;
    GUID   type_id;
    size_t size;
    size_t alignment;

    // extern method
    Vector<ExternMethodData> extern_methods;

    // TODO. meta data

    // signature find
    inline const ExternMethodData* find_extern_method(TypeSignatureView signature, StringView name, ETypeSignatureCompareFlag flag) const
    {
        return extern_methods.find_if([&](const ExternMethodData& method) {
                                 return method.name == name && method.signature_equal(signature, flag);
                             })
            .ptr();
    }

    // template find
    template <typename Func>
    inline const ExternMethodData* find_extern_method(StringView name, ETypeSignatureCompareFlag flag) const
    {
        TypeSignatureTyped<Func> signature;
        return find_extern_method(signature.view(), name, flag);
    }
};

} // namespace skr::rttr