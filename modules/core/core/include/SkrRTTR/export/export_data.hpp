#pragma once
#include "SkrContainers/vector.hpp"
#include "SkrGuid/guid.hpp"
#include "SkrRTTR/rttr_traits.hpp"
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

// TODO. 使用 TypeDesc 代替
struct TypeIdentifier {
    GUID type_id;
    bool is_const;
    bool is_ref;
    bool is_rvalue_ref;
    bool is_pointer;

    template <typename T>
    inline static TypeIdentifier Make()
    {
        // we not process array extent here
        using RemoveExtentType = std::remove_all_extents_t<T>;

        constexpr bool is_const      = std::is_const_v<std::remove_reference_t<RemoveExtentType>>;
        constexpr bool is_ref        = std::is_lvalue_reference_v<RemoveExtentType>;
        constexpr bool is_rvalue_ref = std::is_rvalue_reference_v<RemoveExtentType>;
        constexpr bool is_pointer    = std::is_pointer_v<RemoveExtentType>;
        using DecayType              = std::remove_pointer_t<std::decay_t<RemoveExtentType>>;

        return {
            RTTRTraits<DecayType>::get_guid(),
            is_const,
            is_ref,
            is_rvalue_ref,
            is_pointer
        };
    }
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
    TypeIdentifier  type         = {};
    ParamModifier   modifier     = ParamModifier::In;
    MakeDefaultFunc make_default = nullptr;

    // TODO. meta data

    template <typename Arg>
    inline static ParamData Make()
    {
        return {
            {},
            TypeIdentifier::Make<Arg>(),
            ParamModifier::In,
            nullptr
        };
    }
};

struct FunctionData {
    // signature
    String            name;
    Vector<String>    name_space;
    TypeIdentifier    ret_type;
    Vector<ParamData> param_data;
    // TODO. meta data

    // [Provided by export platform]
    void* invoke;

    template <typename Ret, typename... Args>
    inline void fill_signature(Ret (*)(Args...))
    {
        ret_type   = TypeIdentifier::Make<Ret>();
        param_data = { ParamData::Make<Args>()... };
    }
};

struct MethodData {
    // signature
    String            name;
    TypeIdentifier    ret_type;
    Vector<ParamData> param_data;
    bool              is_const;
    // TODO. meta data

    // [Provided by export platform]
    void* invoke;

    template <class T, typename Ret, typename... Args>
    inline void fill_signature(Ret (T::*)(Args...))
    {
        ret_type   = TypeIdentifier::Make<Ret>();
        param_data = { ParamData::Make<Args>()... };
        is_const   = false;
    }
    template <class T, typename Ret, typename... Args>
    inline void fill_signature(Ret (T::*)(Args...) const)
    {
        ret_type   = TypeIdentifier::Make<Ret>();
        param_data = { ParamData::Make<Args>()... };
        is_const   = true;
    }
};

struct FieldData {
    // signature
    String         name;
    TypeIdentifier type;
    size_t         offset;
    // TODO. meta data

    // [Provided by export platform]
    void* getter;
    void* setter;

    template <class T, typename Field>
    inline void fill_signature(Field T::*p_field)
    {
        type   = TypeIdentifier::Make<Field>();
        offset = reinterpret_cast<size_t>(&(static_cast<T*>(nullptr)->*p_field));
    }
};

struct StaticMethodData {
    // signature
    String            name;
    TypeIdentifier    ret_type;
    Vector<ParamData> param_data;
    // TODO. meta data

    // [Provided by export platform]
    void* invoke;

    template <typename Ret, typename... Args>
    inline void fill_signature(Ret (*)(Args...))
    {
        ret_type   = TypeIdentifier::Make<Ret>();
        param_data = { ParamData::Make<Args>()... };
    }
};

struct StaticFieldData {
    // signature
    String         name;
    TypeIdentifier type;

    // [Provided by export platform]
    void* getter;
    void* setter;

    template <typename T>
    inline void fill_signature(T* p_field)
    {
        type = TypeIdentifier::Make<T>();
    }
};

// TODO. copy/move ctor & operator, 增加限制，且实现固定，或者以 lambda 形式自定义
enum class RecordOperator
{
    // cpp basic
    CopyCtor,
    MoveCtor,
    CopyAssign,
    MoveAssign,

    // arithmetic
    Add, // +
    Sub, // -
    Mul, // *
    Div, // /
    Unm, // 一元 -
    Mod, // %

    // compare
    Eq, // ==
    Lt, // <
    Le, // <=
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

    // [Provided by export platform]
    void* invoke;

    template <typename... Args>
    inline void fill_signature()
    {
        param_data = { ParamData::Make<Args>()... };
    }
};

struct DtorData {
    // [Provided by export platform]
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

    // TODO. meta data
};

} // namespace skr::rttr