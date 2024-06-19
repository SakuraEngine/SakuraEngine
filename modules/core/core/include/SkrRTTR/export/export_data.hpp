#pragma once
#include "SkrContainers/map.hpp"
#include "SkrContainers/optional.hpp"
#include "SkrContainers/vector.hpp"
#include "SkrGuid/guid.hpp"
#include "SkrRTTR/rttr_traits.hpp"
#include "SkrRTTR/type_signature.hpp"
#include "SkrRTTR/enum_value.hpp"
#include "SkrRTTR/export/stack_proxy.hpp"
#include "SkrRTTR/export/attribute.hpp"
// TODO. enable codegen for core
// #ifndef __meta__
//     #include "SkrRTTR/export//export_data.generated.h"
// #endif

// utils
namespace skr sreflect
{
namespace rttr sreflect
{
// basic enums
sreflect_enum_class("guid": "26ff0860-5d65-4ece-896e-225b3c083ecb")
EAccessLevel : uint8_t
{
    Public,
    Protected,
    Private
};

// flags
sreflect_enum_class("guid": "3874222c-79a4-4868-b146-d90c925914e0")
EParamFlag : uint32_t
{
    None = 0,      // default
    In   = 1 << 0, // is input native pointer/reference
    Out  = 1 << 1, // is output native pointer/reference
};
sreflect_enum_class("guid": "cf941db5-afa5-476e-9890-af836a373e73")
EFunctionFlag : uint32_t
{
    None          = 0,      // default
    ScriptVisible = 1 << 0, // can script visit this function
};
sreflect_enum_class("guid": "11511355-6fac-43d6-92f0-5bf3f0963855")
EMethodFlag : uint32_t
{
    None          = 0,      // default
    ScriptVisible = 1 << 0, // can script visit this method
};
sreflect_enum_class("guid": "52c78c63-9973-49f4-9118-a8b59b9ceb9e")
EStaticMethodFlag : uint32_t
{
    None          = 0,      // default
    ScriptVisible = 1 << 0, // can script visit this static method
};
sreflect_enum_class("guid": "f7199493-29f5-4235-86c1-13ec7541917b")
EExternMethodFlag : uint32_t
{
    None          = 0,      // default
    ScriptVisible = 1 << 0, // can script visit this static method
};
sreflect_enum_class("guid": "2f8c20d1-34b2-4ebe-bfd6-258a9e4c0c9e")
EFieldFlag : uint32_t
{
    None          = 0,      // default
    ScriptVisible = 1 << 0, // can script visit this field
};
sreflect_enum_class("guid": "396e9de1-ce51-4a65-8e47-09525e91207f")
EStaticFieldFlag : uint32_t
{
    None          = 0,      // default
    ScriptVisible = 1 << 0, // can script visit this static field
};
sreflect_enum_class("guid": "1f2aa88d-4d2f-47c0-8c97-b03cf574d673")
ECtorFlag : uint32_t
{
    None          = 0,
    ScriptVisible = 1 << 0, // can script visit this ctor
};
sreflect_enum_class("guid": "a1497c67-9865-44cb-b81d-08c4e9548ae9")
ERecordFlag : uint32_t
{
    None          = 0,      // default
    ScriptVisible = 1 << 0, // can script visit this record
    ScriptNewable = 1 << 1, // can script new this record
};
sreflect_enum_class("guid": "a2d04427-aa0a-43b4-9975-bc0e6b92120e")
EEnumItemFlag : uint32_t
{
    None          = 0,      // default
    ScriptVisible = 1 << 0, // can script visit this enum item
};
sreflect_enum_class("guid": "e76f7ad6-303d-4704-9065-827a7f6f270d")
EEnumFlag : uint32_t
{
    None          = 0,      // default
    ScriptVisible = 1 << 0, // can script visit this enum
    Flag          = 1 << 1, // is flag enum
};

// util data
struct ParamData {
    using MakeDefaultFunc = void (*)(void*);

    // signature
    String          name         = {};
    TypeSignature   type         = {};
    MakeDefaultFunc make_default = nullptr; // TODO. 处理在 reference 的情况下 xvalue 的 case

    // TODO. flag & Attribute
    EParamFlag             flag = EParamFlag::None;
    Map<GUID, IAttribute*> attributes;

    template <typename Arg>
    inline static ParamData Make()
    {
        ParamData result;
        result.type = type_signature_of<Arg>();
        return result;
    }
};

// help functions
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
} // namespace rttr sreflect
} // namespace skr sreflect

// functions and methods
namespace skr::rttr
{
struct FunctionData {
    // signature
    String            name       = {};
    Vector<String>    name_space = {};
    TypeSignature     ret_type   = {};
    Vector<ParamData> param_data = {};

    // [Provided by export Backend]
    void*                 native_invoke      = nullptr;
    FuncInvokerStackProxy stack_proxy_invoke = nullptr;

    // flag & attributes
    EFunctionFlag          flag       = EFunctionFlag::None;
    Map<GUID, IAttribute*> attributes = {};

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
struct MethodData {
    // signature
    String            name         = {};
    TypeSignature     ret_type     = {};
    Vector<ParamData> param_data   = {};
    bool              is_const     = false;
    EAccessLevel      access_level = EAccessLevel::Public;

    // [Provided by export Backend]
    void*                   native_invoke      = nullptr;
    MethodInvokerStackProxy stack_proxy_invoke = nullptr;

    // flag & attributes
    EMethodFlag            flag       = EMethodFlag::None;
    Map<GUID, IAttribute*> attributes = {};

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
struct StaticMethodData {
    // signature
    String            name         = {};
    TypeSignature     ret_type     = {};
    Vector<ParamData> param_data   = {};
    EAccessLevel      access_level = EAccessLevel::Public;

    // [Provided by export Backend]
    void*                 native_invoke      = nullptr;
    FuncInvokerStackProxy stack_proxy_invoke = nullptr;

    // flag & attribute
    EStaticMethodFlag      flag       = EStaticMethodFlag::None;
    Map<GUID, IAttribute*> attributes = {};

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
struct ExternMethodData {
    // signature
    String            name         = {};
    TypeSignature     ret_type     = {};
    Vector<ParamData> param_data   = {};
    EAccessLevel      access_level = EAccessLevel::Public;

    // [Provided by export Backend]
    void*                 native_invoke      = nullptr;
    FuncInvokerStackProxy stack_proxy_invoke = nullptr;

    // flag & attribute
    EExternMethodFlag      flag       = EExternMethodFlag::None;
    Map<GUID, IAttribute*> attributes = {};

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
struct CtorData {
    // signature
    Vector<ParamData> param_data   = {};
    EAccessLevel      access_level = EAccessLevel::Public;

    // [Provided by export Backend]
    void*                   native_invoke      = nullptr;
    MethodInvokerStackProxy stack_proxy_invoke = nullptr;

    // flag & attribute
    ECtorFlag              flag       = ECtorFlag::None;
    Map<GUID, IAttribute*> attributes = {};

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
} // namespace skr::rttr

// fields
namespace skr::rttr
{
struct FieldData {
    using GetAddressFunc = void* (*)(void*);

    // signature
    String         name         = {};
    TypeSignature  type         = {};
    GetAddressFunc get_address  = nullptr;
    EAccessLevel   access_level = EAccessLevel::Public;

    // flag & attribute
    EFieldFlag             flag       = EFieldFlag::None;
    Map<GUID, IAttribute*> attributes = {};

    template <auto field, class T, typename Field>
    inline void fill_signature(Field T::*)
    {
        type        = type_signature_of<Field>();
        get_address = +[](void* p) -> void* {
            return &(reinterpret_cast<T*>(p)->*field);
        };
    }
};
struct StaticFieldData {
    // signature
    String        name         = {};
    TypeSignature type         = {};
    void*         address      = nullptr;
    EAccessLevel  access_level = EAccessLevel::Public;

    // flag & attributes
    EStaticFieldFlag       flag       = EStaticFieldFlag::None;
    Map<GUID, IAttribute*> attributes = {};

    template <typename T>
    inline void fill_signature(T* p_field)
    {
        type = type_signature_of<T>();
    }
};
} // namespace skr::rttr

// record
namespace skr::rttr
{
struct BaseData {
    using CastFunc = void* (*)(void*);

    GUID     type_id;
    CastFunc cast_to_base; // cast_to_derived 正向转换在虚继承的情况下会报错，尽量避免这类需求

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
using DtorInvoker = void (*)(void*);
struct DtorData {
    EAccessLevel access_level;

    // [Provided by export Backend]
    DtorInvoker native_invoke;
};
struct RecordData {
    // basic
    String         name       = {};
    Vector<String> name_space = {};
    GUID           type_id    = {};
    size_t         size       = 0;
    size_t         alignment  = 0;

    // bases
    Vector<BaseData> bases_data = {};

    // ctor & dtor
    Vector<CtorData> ctor_data = {};
    DtorData         dtor_data = {};

    // method & fields
    Vector<MethodData> methods = {};
    Vector<FieldData>  fields  = {};

    // static method & static fields
    Vector<StaticMethodData> static_methods = {};
    Vector<StaticFieldData>  static_fields  = {};

    // extern method
    Vector<ExternMethodData> extern_methods = {};

    // flag & attributes
    ERecordFlag            flag       = ERecordFlag::None;
    Map<GUID, IAttribute*> attributes = {};

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
} // namespace skr::rttr

// enum
namespace skr::rttr
{
struct EnumItemData {
    String    name  = {};
    EnumValue value = {};

    // flag & attribute
    EEnumItemFlag          flag       = EEnumItemFlag::None;
    Map<GUID, IAttribute*> attributes = {};
};
struct EnumData {
    // basic
    String         name       = {};
    Vector<String> name_space = {};
    GUID           type_id    = {};
    size_t         size       = 0;
    size_t         alignment  = 0;

    // underlying type
    GUID underlying_type_id = {};

    // items
    Vector<EnumItemData> items = {};

    // extern method
    Vector<ExternMethodData> extern_methods = {};

    // flag & attributes
    EEnumFlag              flag       = EEnumFlag::None;
    Map<GUID, IAttribute*> attributes = {};

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
} // namespace skr::rttr

// primitive data
namespace skr::rttr
{
struct PrimitiveData {
    // basic
    String name;
    GUID   type_id;
    size_t size;
    size_t alignment;

    // extern method
    Vector<ExternMethodData> extern_methods;

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
