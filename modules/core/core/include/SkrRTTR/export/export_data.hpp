#pragma once
#include "SkrContainers/vector.hpp"
#include "SkrGuid/guid.hpp"
#include "SkrRTTR/rttr_traits.hpp"
#include "SkrRTTR/type_desc.hpp"
#include "SkrRTTR/enum_value.hpp"

namespace skr::rttr
{
struct EnumItemExport {
    String    name;
    EnumValue value;
    // TODO. meta data
};
struct EnumData {
    String                 name;
    Vector<String>         name_space;
    GUID                   type_id;
    GUID                   underlying_type_id;
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
    TypeDesc        type         = {};
    ParamModifier   modifier     = ParamModifier::In;
    MakeDefaultFunc make_default = nullptr;

    // TODO. meta data

    template <typename Arg>
    inline static ParamData Make()
    {
        return {
            {},
            type_desc_of<Arg>(),
            ParamModifier::In,
            nullptr
        };
    }
};

struct FunctionData {
    // signature
    String            name;
    Vector<String>    name_space;
    TypeDesc          ret_type;
    Vector<ParamData> param_data;
    // TODO. meta data

    // [Provided by export Backend]
    void* invoke;

    template <typename Ret, typename... Args>
    inline void fill_signature(Ret (*)(Args...))
    {
        ret_type   = type_desc_of<Ret>();
        param_data = { ParamData::Make<Args>()... };
    }
};

struct MethodData {
    // signature
    String            name;
    TypeDesc          ret_type;
    Vector<ParamData> param_data;
    bool              is_const;
    // TODO. meta data

    // [Provided by export Backend]
    void* invoke;

    template <class T, typename Ret, typename... Args>
    inline void fill_signature(Ret (T::*)(Args...))
    {
        ret_type   = type_desc_of<Ret>();
        param_data = { ParamData::Make<Args>()... };
        is_const   = false;
    }
    template <class T, typename Ret, typename... Args>
    inline void fill_signature(Ret (T::*)(Args...) const)
    {
        ret_type   = type_desc_of<Ret>();
        param_data = { ParamData::Make<Args>()... };
        is_const   = true;
    }
};

struct FieldData {
    using GetAddressFunc = void* (*)(void*);

    // signature
    String         name;
    TypeDesc       type;
    GetAddressFunc get_address;
    // TODO. meta data

    // [Provided by export Backend]
    void* getter;
    void* setter;

    template <auto field, class T, typename Field>
    inline void fill_signature(Field T::*)
    {
        type        = type_desc_of<Field>();
        get_address = +[](void* p) -> void* {
            return &(reinterpret_cast<T*>(p)->*field);
        };
    }
};

struct StaticMethodData {
    // signature
    String            name;
    TypeDesc          ret_type;
    Vector<ParamData> param_data;
    // TODO. meta data

    // [Provided by export Backend]
    void* invoke;

    template <typename Ret, typename... Args>
    inline void fill_signature(Ret (*)(Args...))
    {
        ret_type   = type_desc_of<Ret>();
        param_data = { ParamData::Make<Args>()... };
    }
};

struct StaticFieldData {
    // signature
    String   name;
    TypeDesc type;
    void*    address;

    // [Provided by export Backend]
    void* getter;
    void* setter;

    template <typename T>
    inline void fill_signature(T* p_field)
    {
        type = type_desc_of<T>();
    }
};

struct ExternMethodData {
    // signature
    String            name;
    TypeDesc          ret_type;
    Vector<ParamData> param_data;
    // TODO. meta data

    // [Provided by export Backend]
    void* invoke;

    template <typename Ret, typename... Args>
    inline void fill_signature(Ret (*)(Args...))
    {
        ret_type   = type_desc_of<Ret>();
        param_data = { ParamData::Make<Args>()... };
    }
};

struct BaseData {
    using CastFunc = void* (*)(void*);

    GUID     type_id;
    CastFunc cast;

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
    // TODO. meta data

    // [Provided by export Backend]
    void* invoke;

    template <typename... Args>
    inline void fill_signature()
    {
        param_data = { ParamData::Make<Args>()... };
    }
};

struct DtorData {
    // [Provided by export Backend]
    void* invoke;
};

struct RecordData {
    // basic
    String           name;
    Vector<String>   name_space;
    GUID             type_id;
    Vector<BaseData> bases_data;
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

struct FunctionSignature {
};

inline bool match_parameters(span<ParamData> param_data, span<TypeDescView> signature, ETypeDescNormalizeFlag match_flag)
{
    // match length
    if (param_data.size() != signature.size())
        return false;

    // match per parameter
    for (size_t i = 0; i < param_data.size(); ++i)
    {
        if (!param_data[i].type.equal(signature[i], match_flag))
        {
            return false;
        }
    }

    return true;
}

template <typename... Args>
struct ParameterSignatureExporter {
    inline ParameterSignatureExporter()
    {
        fill_signature(std::make_index_sequence<sizeof...(Args)>());
    }

    template <size_t... idx>
    inline void fill_signature(std::index_sequence<idx...>)
    {
        int dummy[] = { (args_type_desc_view[idx] = std::get<idx>(args_tuple), 0)... };
        (void)dummy;
    }

    std::tuple<TypedTypeDesc<Args>...> args_tuple = {};
    TypeDescView                       args_type_desc_view[sizeof...(Args)];
};

} // namespace skr::rttr