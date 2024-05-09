#pragma once
#include "SkrContainers/vector.hpp"
#include "SkrGuid/guid.hpp"
#include "SkrRTTR/rttr_traits.hpp"
#include "SkrRTTR/type_signature.hpp"
#include "SkrRTTR/enum_value.hpp"

namespace skr::rttr
{
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

    // TODO. meta data
};

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
};

} // namespace skr::rttr