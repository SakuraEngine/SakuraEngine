#pragma once
#include "SkrContainersDef/map.hpp"
#include "SkrContainersDef/optional.hpp"
#include "SkrContainersDef/vector.hpp"
#include "SkrRTTR/rttr_traits.hpp"
#include "SkrRTTR/type_signature.hpp"
#include "SkrRTTR/enum_tools.hpp"
#include "SkrRTTR/export/dynamic_stack.hpp"
#include <SkrRTTR/any.hpp>
#ifndef __meta__
    #include "SkrRTTR/export/export_data.generated.h"
#endif

// utils
namespace skr
{
// basic enums
// clang-format off
sreflect_enum_class(guid = "26ff0860-5d65-4ece-896e-225b3c083ecb")
ERTTRAccessLevel : uint8_t
// clang-format on
{
    Public,
    Protected,
    Private
};

// flags
// clang-format off
sreflect_enum_class(guid = "3874222c-79a4-4868-b146-d90c925914e0")
ERTTRParamFlag : uint32_t
// clang-format on
{
    None = 0,      // default
    In   = 1 << 0, // is input native pointer/reference
    Out  = 1 << 1, // is output native pointer/reference

    InOut = In | Out, // is input/output native pointer/reference
};
// clang-format off
sreflect_enum_class(guid = "cf941db5-afa5-476e-9890-af836a373e73")
ERTTRFunctionFlag : uint32_t
// clang-format on
{
    None          = 0,      // default
    ScriptVisible = 1 << 0, // can script visit this function
};
// clang-format off
sreflect_enum_class(guid = "42d77f46-f3e0-492d-b67f-f8445eff2885")
ERTTRMethodFlag : uint32_t
// clang-format on
{
    None          = 0,      // default
    ScriptVisible = 1 << 0, // can script visit this method
    ScriptMixin   = 1 << 1, // script can mixin logic into this method
};
// clang-format off
sreflect_enum_class(guid = "52c78c63-9973-49f4-9118-a8b59b9ceb9e")
ERTTRStaticMethodFlag : uint32_t
// clang-format on
{
    None          = 0,      // default
    ScriptVisible = 1 << 0, // can script visit this static method
};
// clang-format off
sreflect_enum_class(guid = "f7199493-29f5-4235-86c1-13ec7541917b")
ERTTRExternMethodFlag : uint32_t
// clang-format on
{
    None          = 0,      // default
    ScriptVisible = 1 << 0, // can script visit this static method
};
// clang-format off
sreflect_enum_class(guid = "2f8c20d1-34b2-4ebe-bfd6-258a9e4c0c9e")
ERTTRFieldFlag : uint32_t
// clang-format on
{
    None          = 0,      // default
    ScriptVisible = 1 << 0, // can script visit this field
};
// clang-format off
sreflect_enum_class(guid = "396e9de1-ce51-4a65-8e47-09525e91207f")
ERTTRStaticFieldFlag : uint32_t
// clang-format on
{
    None          = 0,      // default
    ScriptVisible = 1 << 0, // can script visit this static field
};
// clang-format off
sreflect_enum_class(guid = "1f2aa88d-4d2f-47c0-8c97-b03cf574d673")
ERTTRCtorFlag : uint32_t
// clang-format on
{
    None          = 0,
    ScriptVisible = 1 << 0, // can script visit this ctor
};
// clang-format off
sreflect_enum_class(guid = "a1497c67-9865-44cb-b81d-08c4e9548ae9")
ERTTRRecordFlag : uint32_t
// clang-format on
{
    None          = 0,      // default
    ScriptVisible = 1 << 0, // can script visit this record
    ScriptNewable = 1 << 1, // can script new this record
    ScriptMapping = 1 << 2, // export to script use mapping mode
};
// clang-format off
sreflect_enum_class(guid = "a2d04427-aa0a-43b4-9975-bc0e6b92120e")
ERTTREnumItemFlag : uint32_t
// clang-format on
{
    None          = 0,      // default
    ScriptVisible = 1 << 0, // can script visit this enum item
};
// clang-format off
sreflect_enum_class(guid = "e76f7ad6-303d-4704-9065-827a7f6f270d")
ERTTREnumFlag : uint32_t
// clang-format on
{
    None          = 0,      // default
    ScriptVisible = 1 << 0, // can script visit this enum
    Flag          = 1 << 1, // is flag enum
};

// util data
struct RTTRParamData {
    using MakeDefaultFunc = void (*)(void*);

    // signature
    uint64_t      index = 0;
    String        name  = {};
    TypeSignature type  = {};
    // TODO. make default 需要处理 xvalue 的 case，以及判断是否是 InitListExpr
    //       同时，需要通过是否是 ExprWithCleanups 来判断是否是 xvalue 类型的构造
    MakeDefaultFunc make_default = nullptr;

    // TODO. flag & Attribute
    ERTTRParamFlag flag = ERTTRParamFlag::None;
    Vector<Any>    attrs;

    template <typename Arg>
    inline static RTTRParamData* New()
    {
        RTTRParamData* result = SkrNew<RTTRParamData>();
        result->type          = type_signature_of<Arg>();
        return result;
    }
    template <typename... Args>
    inline static void Fill(Vector<RTTRParamData*>& out)
    {
        out = { RTTRParamData::New<Args>()... };
        for (uint32_t i = 0; i < out.size(); ++i)
        {
            out[i]->index = i;
        }
    }
};
struct MemoryTraitsData {
    bool use_ctor : 1;
    bool use_dtor : 1;
    bool use_copy : 1;
    bool use_move : 1;
    bool use_assign : 1;
    bool use_move_assign : 1;
    bool need_dtor_after_move : 1;
    bool use_realloc : 1;
    bool use_compare : 1;

    template <typename T>
    inline void Fill()
    {
        use_ctor             = memory::MemoryTraits<T>::use_ctor;
        use_dtor             = memory::MemoryTraits<T>::use_dtor;
        use_copy             = memory::MemoryTraits<T>::use_copy;
        use_move             = memory::MemoryTraits<T>::use_move;
        use_assign           = memory::MemoryTraits<T>::use_assign;
        use_move_assign      = memory::MemoryTraits<T>::use_move_assign;
        need_dtor_after_move = memory::MemoryTraits<T>::need_dtor_after_move;
        use_realloc          = memory::MemoryTraits<T>::use_realloc;
        use_compare          = memory::MemoryTraits<T>::use_compare;
    }
    template <typename T>
    inline static MemoryTraitsData Make()
    {
        MemoryTraitsData data;
        data.Fill<T>();
        return data;
    }
};

// help functions
template <typename Data>
inline bool export_function_signature_equal(const Data& data, TypeSignatureView signature, ETypeSignatureCompareFlag flag)
{
    SKR_ASSERT(signature.is_complete());
    SKR_ASSERT(signature.is_function());

    // read signature data
    uint32_t param_count;
    auto     view = signature.read_function_signature(param_count);
    if (param_count != data.param_data.size()) { return false; }

    // compare return type
    auto ret_view = view.jump_next_type_or_data();
    if (!data.ret_type.view().equal(ret_view, flag)) { return false; }

    // compare param type
    for (uint32_t i = 0; i < param_count; ++i)
    {
        auto param_view = view.jump_next_type_or_data();
        if (!data.param_data[i]->type.view().equal(param_view, flag)) { return false; }
    }

    return true;
}
template <typename DataA, typename DataB>
inline bool cross_data_signature_equal(const DataA& data_a, const DataB& data_b, ETypeSignatureCompareFlag flag)
{
    // compare param count
    if (data_a.param_data.size() != data_b.param_data.size()) { return false; }

    // compare return type
    if (!data_a.ret_type.view().equal(data_b.ret_type.view(), flag)) { return false; }

    // compare param type
    for (uint32_t i = 0; i < data_a.param_data.size(); ++i)
    {
        if (!data_a.param_data[i]->type.view().equal(data_b.param_data[i]->type.view(), flag)) { return false; }
    }

    return true;
}
} // namespace skr

// attributes
namespace skr::attr
{
// specify property getter if script has property functional
// use on:
//   method
//   static_method
// clang-format off
sreflect_struct(guid = "982d9f4d-1c0f-4ee2-914a-b211f7539cff")
ScriptGetter {
    // clang-format on
    inline ScriptGetter(String name)
        : prop_name(std::move(name))
    {
    }
    String prop_name = {};
};

// specify property setter if script has property functionalScriptGetter
// use on:
//   method
//   static_method
// clang-format off
sreflect_struct(guid = "ee933720-6b10-4b03-a6e7-20227e4829b2")
ScriptSetter {
    // clang-format on
    inline ScriptSetter(String name)
        : prop_name(std::move(name))
    {
    }
    String prop_name = {};
};
} // namespace skr::attr

// functions and methods
namespace skr
{
struct RTTRFunctionData {
    // signature
    String                 name       = {};
    Vector<String>         name_space = {};
    TypeSignature          ret_type   = {};
    Vector<RTTRParamData*> param_data = {};

    // [Provided by export Backend]
    void*                   native_invoke        = nullptr;
    FuncInvokerDynamicStack dynamic_stack_invoke = nullptr;

    // flag & attributes
    ERTTRFunctionFlag flag = ERTTRFunctionFlag::None;
    Vector<Any>       attrs;

    inline ~RTTRFunctionData()
    {
        // delete param data
        for (auto param : param_data)
        {
            SkrDelete(param);
        }
    }

    template <typename Ret, typename... Args>
    inline void fill_signature(Ret (*)(Args...))
    {
        ret_type = type_signature_of<Ret>();
        RTTRParamData::Fill<Args...>(param_data);
    }

    inline bool signature_equal(TypeSignatureView signature, ETypeSignatureCompareFlag flag) const
    {
        return export_function_signature_equal(*this, signature, flag);
    }
    template <typename Data>
    inline bool signature_equal(const Data& data, ETypeSignatureCompareFlag flag) const
    {
        return cross_data_signature_equal(*this, data, flag);
    }
};
struct RTTRMethodData {
    // signature
    String                 name         = {};
    TypeSignature          ret_type     = {};
    Vector<RTTRParamData*> param_data   = {};
    bool                   is_const     = false;
    ERTTRAccessLevel       access_level = ERTTRAccessLevel::Public;

    // [Provided by export Backend]
    void*                     native_invoke        = nullptr;
    MethodInvokerDynamicStack dynamic_stack_invoke = nullptr;

    // flag & attributes
    ERTTRMethodFlag flag = ERTTRMethodFlag::None;
    Vector<Any>     attrs;

    inline ~RTTRMethodData()
    {
        // delete param data
        for (auto param : param_data)
        {
            SkrDelete(param);
        }
    }

    template <class T, typename Ret, typename... Args>
    inline void fill_signature(Ret (T::*)(Args...))
    {
        ret_type = type_signature_of<Ret>();
        RTTRParamData::Fill<Args...>(param_data);
        is_const = false;
    }
    template <class T, typename Ret, typename... Args>
    inline void fill_signature(Ret (T::*)(Args...) const)
    {
        ret_type = type_signature_of<Ret>();
        RTTRParamData::Fill<Args...>(param_data);
        is_const = true;
    }

    inline bool signature_equal(TypeSignatureView signature, ETypeSignatureCompareFlag flag) const
    {
        return export_function_signature_equal(*this, signature, flag);
    }
    template <typename Data>
    inline bool signature_equal(const Data& data, ETypeSignatureCompareFlag flag) const
    {
        return cross_data_signature_equal(*this, data, flag);
    }
};
struct RTTRStaticMethodData {
    // signature
    String                 name         = {};
    TypeSignature          ret_type     = {};
    Vector<RTTRParamData*> param_data   = {};
    ERTTRAccessLevel       access_level = ERTTRAccessLevel::Public;

    // [Provided by export Backend]
    void*                   native_invoke        = nullptr;
    FuncInvokerDynamicStack dynamic_stack_invoke = nullptr;

    // flag & attributes
    ERTTRStaticMethodFlag flag = ERTTRStaticMethodFlag::None;
    Vector<Any>           attrs;

    inline ~RTTRStaticMethodData()
    {
        // delete param data
        for (auto param : param_data)
        {
            SkrDelete(param);
        }
    }

    template <typename Ret, typename... Args>
    inline void fill_signature(Ret (*)(Args...))
    {
        ret_type = type_signature_of<Ret>();
        RTTRParamData::Fill<Args...>(param_data);
    }

    inline bool signature_equal(TypeSignatureView signature, ETypeSignatureCompareFlag flag) const
    {
        return export_function_signature_equal(*this, signature, flag);
    }
    template <typename Data>
    inline bool signature_equal(const Data& data, ETypeSignatureCompareFlag flag) const
    {
        return cross_data_signature_equal(*this, data, flag);
    }
};
struct RTTRExternMethodData {
    // signature
    String                 name         = {};
    TypeSignature          ret_type     = {};
    Vector<RTTRParamData*> param_data   = {};
    ERTTRAccessLevel       access_level = ERTTRAccessLevel::Public;

    // [Provided by export Backend]
    void*                   native_invoke        = nullptr;
    FuncInvokerDynamicStack dynamic_stack_invoke = nullptr;

    // flag & attributes
    ERTTRExternMethodFlag flag = ERTTRExternMethodFlag::None;
    Vector<Any>           attrs;

    inline ~RTTRExternMethodData()
    {
        // delete param data
        for (auto param : param_data)
        {
            SkrDelete(param);
        }
    }

    template <typename Ret, typename... Args>
    inline void fill_signature(Ret (*)(Args...))
    {
        ret_type = type_signature_of<Ret>();
        RTTRParamData::Fill<Args...>(param_data);
    }

    inline bool signature_equal(TypeSignatureView signature, ETypeSignatureCompareFlag flag) const
    {
        return export_function_signature_equal(*this, signature, flag);
    }
    template <typename Data>
    inline bool signature_equal(const Data& data, ETypeSignatureCompareFlag flag) const
    {
        return cross_data_signature_equal(*this, data, flag);
    }
};
struct RTTRCtorData {
    // signature
    Vector<RTTRParamData*> param_data   = {};
    ERTTRAccessLevel       access_level = ERTTRAccessLevel::Public;

    // [Provided by export Backend]
    void*                     native_invoke        = nullptr;
    MethodInvokerDynamicStack dynamic_stack_invoke = nullptr;

    // flag & attributes
    ERTTRCtorFlag flag = ERTTRCtorFlag::None;
    Vector<Any>   attrs;

    inline ~RTTRCtorData()
    {
        // delete param data
        for (auto param : param_data)
        {
            SkrDelete(param);
        }
    }

    template <typename... Args>
    inline void fill_signature()
    {
        RTTRParamData::Fill<Args...>(param_data);
    }

    inline bool signature_equal(TypeSignatureView signature, ETypeSignatureCompareFlag flag) const
    {
        SKR_ASSERT(signature.is_complete());
        SKR_ASSERT(signature.is_function());

        // read signature data
        uint32_t param_count;
        auto     view = signature.read_function_signature(param_count);
        if (param_count != param_data.size()) { return false; }

        // compare return type
        auto                     ret_view = view.jump_next_type_or_data();
        TypeSignatureTyped<void> ret_type;
        if (!ret_type.view().equal(ret_view, flag)) { return false; }

        // compare param type
        for (uint32_t i = 0; i < param_count; ++i)
        {
            auto param_view = view.jump_next_type_or_data();
            if (!param_data[i]->type.view().equal(param_view, flag)) { return false; }
        }

        return true;
    }
};
} // namespace skr

// fields
namespace skr
{
struct RTTRFieldData {
    using GetAddressFunc = void* (*)(void*);

    // signature
    String           name         = {};
    TypeSignature    type         = {};
    GetAddressFunc   get_address  = nullptr;
    ERTTRAccessLevel access_level = ERTTRAccessLevel::Public;

    // flag & attributes
    ERTTRFieldFlag flag = ERTTRFieldFlag::None;
    Vector<Any>    attrs;

    inline ~RTTRFieldData()
    {
    }

    template <auto field, class T, typename Field>
    inline void fill_signature(Field T::*)
    {
        type        = type_signature_of<Field>();
        get_address = +[](void* p) -> void* {
            return &(reinterpret_cast<T*>(p)->*field);
        };
    }
};
struct RTTRStaticFieldData {
    // signature
    String           name         = {};
    TypeSignature    type         = {};
    void*            address      = nullptr;
    ERTTRAccessLevel access_level = ERTTRAccessLevel::Public;

    // flag & attributes
    ERTTRStaticFieldFlag flag = ERTTRStaticFieldFlag::None;
    Vector<Any>          attrs;

    inline ~RTTRStaticFieldData()
    {
    }

    template <typename T>
    inline void fill_signature(T* p_field)
    {
        type = type_signature_of<T>();
    }
};
} // namespace skr

// record
namespace skr
{
struct RTTRBaseData {
    using CastFunc = void* (*)(void*);

    GUID     type_id;
    CastFunc cast_to_base;

    template <typename T, typename Base>
    inline static RTTRBaseData* New()
    {
        return SkrNew<RTTRBaseData>(
            RTTRTraits<Base>::get_guid(),
            +[](void* p) -> void* {
                return static_cast<Base*>(reinterpret_cast<T*>(p));
            }
        );
    }
};
using DtorInvoker = void (*)(void*);
struct RTTRDtorData {
    ERTTRAccessLevel access_level;

    // [Provided by export Backend]
    DtorInvoker native_invoke;
};
struct RTTRRecordData {
    // basic
    String           name               = {};
    Vector<String>   name_space         = {};
    GUID             type_id            = {};
    size_t           size               = 0;
    size_t           alignment          = 0;
    MemoryTraitsData memory_traits_data = {};

    // bases
    Vector<RTTRBaseData*> bases_data = {};

    // ctor & dtor
    Vector<RTTRCtorData*> ctor_data = {};
    RTTRDtorData          dtor_data = {};

    // method & fields
    Vector<RTTRMethodData*> methods = {};
    Vector<RTTRFieldData*>  fields  = {};

    // static method & static fields
    Vector<RTTRStaticMethodData*> static_methods = {};
    Vector<RTTRStaticFieldData*>  static_fields  = {};

    // extern method
    Vector<RTTRExternMethodData*> extern_methods = {};

    // flag & attributes
    ERTTRRecordFlag flag = ERTTRRecordFlag::None;
    Vector<Any>     attrs;

    inline ~RTTRRecordData()
    {
        // delete bases
        for (auto base : bases_data)
        {
            SkrDelete(base);
        }

        // delete ctors
        for (auto ctor : ctor_data)
        {
            SkrDelete(ctor);
        }

        // delete methods & fields
        for (auto method : methods)
        {
            SkrDelete(method);
        }
        for (auto field : fields)
        {
            SkrDelete(field);
        }

        // delete static methods & static fields
        for (auto method : static_methods)
        {
            SkrDelete(method);
        }
        for (auto field : static_fields)
        {
            SkrDelete(field);
        }

        // delete extern methods
        for (auto method : extern_methods)
        {
            SkrDelete(method);
        }
    }
};
} // namespace skr

// enum
namespace skr
{
struct RTTREnumItemData {
    String    name  = {};
    EnumValue value = {};

    // flag & attributes
    ERTTREnumItemFlag flag = ERTTREnumItemFlag::None;
    Vector<Any>       attrs;

    inline ~RTTREnumItemData()
    {
    }
};
struct RTTREnumData {
    // basic
    String           name               = {};
    Vector<String>   name_space         = {};
    GUID             type_id            = {};
    size_t           size               = 0;
    size_t           alignment          = 0;
    MemoryTraitsData memory_traits_data = {};

    // underlying type
    GUID underlying_type_id = {};

    // items
    Vector<RTTREnumItemData*> items = {};

    // extern method
    Vector<RTTRExternMethodData*> extern_methods = {};

    // flag & attributes
    ERTTREnumFlag flag = ERTTREnumFlag::None;
    Vector<Any>   attrs;

    inline ~RTTREnumData()
    {
        // delete items
        for (auto item : items)
        {
            SkrDelete(item);
        }

        // delete extern methods
        for (auto method : extern_methods)
        {
            SkrDelete(method);
        }
    }
};
} // namespace skr

// primitive data
namespace skr
{
struct RTTRPrimitiveData {
    // basic
    String           name               = {};
    GUID             type_id            = {};
    size_t           size               = 0;
    size_t           alignment          = 0;
    MemoryTraitsData memory_traits_data = {};

    // extern method
    Vector<RTTRExternMethodData*> extern_methods;

    inline ~RTTRPrimitiveData()
    {
        // delete extern methods
        for (auto method : extern_methods)
        {
            SkrDelete(method);
        }
    }
};
} // namespace skr
