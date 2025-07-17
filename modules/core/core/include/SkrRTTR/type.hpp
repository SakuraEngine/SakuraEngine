#pragma once
#include <cstdint>
#include "SkrContainersDef/function_ref.hpp"
#include "SkrContainersDef/string.hpp"
#include "SkrContainersDef/string.hpp"
#include "SkrRTTR/enum_tools.hpp"
#include "SkrRTTR/export/export_data.hpp"
#include <SkrRTTR/export/export_accessor.hpp>

// !!!! RTTR 不考虑动态类型建立(从脚本建立), 一切类型都是 CPP 静态注册的 loader !!!!
namespace skr
{
enum class ERTTRTypeCategory
{
    Invalid = 0, // uninitialized bad type

    Primitive,
    Record,
    Enum,
};
struct RTTRTypeCaster {
    using CastFunc = void* (*)(void*);

    inline void* cast(void* p)
    {
        if (!is_valid)
        {
            return nullptr;
        }

        for (auto func : cast_funcs)
        {
            p = func(p);
        }
        return p;
    }

    bool                      is_valid   = true;
    InlineVector<CastFunc, 8> cast_funcs = {};
};
struct RTTRTypeEachConfig {
    // should each bases to find
    bool include_bases = true;
};
struct RTTRTypeFindConfig {
    // if setted, will filter by name
    Optional<StringView> name = {};

    // if setted, will filter by signature
    Optional<TypeSignatureView> signature              = {};
    ETypeSignatureCompareFlag   signature_compare_flag = ETypeSignatureCompareFlag::Strict;

    // should each bases to find
    bool include_bases = true;
};

using RTTRInvokerDefaultCtor = ExportCtorInvoker<void()>;
using RTTRInvokerCopyCtor    = ExportCtorInvoker<void(const void*)>;
using RTTRInvokerMoveCtor    = ExportCtorInvoker<void(void*)>;
using RTTRInvokerAssign      = ExportExternMethodInvoker<void(void*, const void*)>;
using RTTRInvokerMoveAssign  = ExportExternMethodInvoker<void(void*, void*)>;
using RTTRInvokerEqual       = ExportExternMethodInvoker<bool(const void*, const void*)>;
using RTTRInvokerHash        = ExportExternMethodInvoker<size_t(const void*)>;
using RTTRInvokerSwap        = ExportExternMethodInvoker<void(void*, void*)>;

struct SKR_CORE_API RTTRType final {
    // ctor & dtor
    RTTRType();
    ~RTTRType();

    // module
    void   set_module(String module);
    String module() const;

    // basic getter
    ERTTRTypeCategory  type_category() const;
    const skr::String& name() const;
    Vector<String>     name_space() const;
    String             name_space_str() const;
    GUID               type_id() const;
    size_t             size() const;
    size_t             alignment() const;
    void               each_name_space(FunctionRef<void(StringView)> each_func) const;
    MemoryTraitsData   memory_traits_data() const;

    // kind getter
    bool is_primitive() const;
    bool is_record() const;
    bool is_enum() const;

    // builders
    void build_primitive(FunctionRef<void(RTTRPrimitiveData* data)> func);
    void build_record(FunctionRef<void(RTTRRecordData* data)> func);
    void build_enum(FunctionRef<void(RTTREnumData* data)> func);

    // TODO. build optimize data
    //  1. fast base find table
    //  2. fast method/field table
    // void build_optimize_data();

    // TODO. check phase, used to check data conflict
    // void validate_export_data() const;

    // caster
    bool           based_on(GUID type_id, uint32_t* out_cast_count = 0) const;
    void*          cast_to_base(GUID type_id, void* p) const;
    RTTRTypeCaster caster_to_base(GUID type_id) const;

    // enum getter
    GUID enum_underlying_type_id() const;
    void each_enum_items(FunctionRef<void(const RTTREnumItemData*)> each_func) const;

    // get dtor
    Optional<RTTRDtorData> dtor_data() const;
    DtorInvoker            dtor_invoker() const;
    void                   invoke_dtor(void* p) const;

    // each method & field
    void each_bases(FunctionRef<void(const RTTRBaseData* base_data, const RTTRType* owner)> each_func, RTTRTypeEachConfig config = {}) const;
    void each_ctor(FunctionRef<void(const RTTRCtorData* ctor)> each_func, RTTRTypeEachConfig config = {}) const; // ignore [config.include_bases]x
    void each_method(FunctionRef<void(const RTTRMethodData* method, const RTTRType* owner)> each_func, RTTRTypeEachConfig config = {}) const;
    void each_field(FunctionRef<void(const RTTRFieldData* field, const RTTRType* owner)> each_func, RTTRTypeEachConfig config = {}) const;
    void each_static_method(FunctionRef<void(const RTTRStaticMethodData* method, const RTTRType* owner)> each_func, RTTRTypeEachConfig config = {}) const;
    void each_static_field(FunctionRef<void(const RTTRStaticFieldData* field, const RTTRType* owner)> each_func, RTTRTypeEachConfig config = {}) const;
    void each_extern_method(FunctionRef<void(const RTTRExternMethodData* method, const RTTRType* owner)> each_func, RTTRTypeEachConfig config = {}) const;

    // find method & field
    const RTTRCtorData*         find_ctor(RTTRTypeFindConfig config) const; // ignore [config.name/config.include_bases]
    const RTTRMethodData*       find_method(RTTRTypeFindConfig config) const;
    const RTTRFieldData*        find_field(RTTRTypeFindConfig config) const;
    const RTTRStaticMethodData* find_static_method(RTTRTypeFindConfig config) const;
    const RTTRStaticFieldData*  find_static_field(RTTRTypeFindConfig config) const;
    const RTTRExternMethodData* find_extern_method(RTTRTypeFindConfig config) const;

    // template find method & field
    // TODO. method & field 需要根据 Owner 信息进行访问
    template <typename Func>
    ExportCtorInvoker<Func> find_ctor_t(ETypeSignatureCompareFlag flag = ETypeSignatureCompareFlag::Strict) const;
    template <typename Func>
    ExportMethodInvoker<Func> find_method_t(StringView name, ETypeSignatureCompareFlag flag = ETypeSignatureCompareFlag::Strict, bool include_base = true) const;
    template <typename Field>
    const RTTRFieldData* find_field_t(StringView name, ETypeSignatureCompareFlag flag = ETypeSignatureCompareFlag::Strict, bool include_base = true) const;
    template <typename Func>
    ExportStaticMethodInvoker<Func> find_static_method_t(StringView name, ETypeSignatureCompareFlag flag = ETypeSignatureCompareFlag::Strict, bool include_base = true) const;
    template <typename Field>
    const RTTRStaticFieldData* find_static_field_t(StringView name, ETypeSignatureCompareFlag flag = ETypeSignatureCompareFlag::Strict, bool include_base = true) const;
    template <typename Func>
    ExportExternMethodInvoker<Func> find_extern_method_t(StringView name, ETypeSignatureCompareFlag flag = ETypeSignatureCompareFlag::Strict, bool include_base = true) const;

    // find basic functions
    RTTRInvokerDefaultCtor find_default_ctor() const;
    RTTRInvokerCopyCtor    find_copy_ctor() const;
    RTTRInvokerMoveCtor    find_move_ctor() const;
    RTTRInvokerAssign      find_assign() const;
    RTTRInvokerMoveAssign  find_move_assign() const;
    RTTRInvokerEqual       find_equal() const;
    RTTRInvokerHash        find_hash() const;
    RTTRInvokerSwap        find_swap() const;

    // flag & attribute
    ERTTRRecordFlag record_flag() const;
    ERTTREnumFlag   enum_flag() const;
    const Any*      find_attribute(TypeSignatureView signature) const;
    void            each_attribute(FunctionRef<void(const Any&)> each_func) const;
    void            each_attribute(FunctionRef<void(const Any&)> each_func, TypeSignatureView signature) const;

private:
    // helpers
    bool _build_caster(RTTRTypeCaster& caster, const GUID& type_id) const;

private:
    ERTTRTypeCategory _type_category = ERTTRTypeCategory::Invalid;
    String            _module        = {};
    union
    {
        RTTRPrimitiveData _primitive_data;
        RTTRRecordData    _record_data;
        RTTREnumData      _enum_data;
    };
};
} // namespace skr

// type inline impl
namespace skr
{
// module
inline void RTTRType::set_module(String module)
{
    _module = module;
}
inline String RTTRType::module() const
{
    return _module;
}

// basic getter
inline ERTTRTypeCategory RTTRType::type_category() const
{
    return _type_category;
}
inline const skr::String& RTTRType::name() const
{
    switch (_type_category)
    {
    case ERTTRTypeCategory::Primitive:
        return _primitive_data.name;
    case ERTTRTypeCategory::Record:
        return _record_data.name;
    case ERTTRTypeCategory::Enum:
        return _enum_data.name;
    default:
        SKR_UNREACHABLE_CODE()
        return _primitive_data.name;
    }
}
inline Vector<String> RTTRType::name_space() const
{
    switch (_type_category)
    {
    case ERTTRTypeCategory::Primitive:
        return {};
    case ERTTRTypeCategory::Record:
        return _record_data.name_space;
    case ERTTRTypeCategory::Enum:
        return _enum_data.name_space;
    default:
        SKR_UNREACHABLE_CODE()
        return {};
    }
}
inline String RTTRType::name_space_str() const
{
    switch (_type_category)
    {
    case ERTTRTypeCategory::Primitive:
        return {};
    case ERTTRTypeCategory::Record:
        return String::Join(_record_data.name_space, u8"::");
    case ERTTRTypeCategory::Enum:
        return String::Join(_enum_data.name_space, u8"::");
    default:
        SKR_UNREACHABLE_CODE()
        return {};
    }
}
inline GUID RTTRType::type_id() const
{
    switch (_type_category)
    {
    case ERTTRTypeCategory::Primitive:
        return _primitive_data.type_id;
    case ERTTRTypeCategory::Record:
        return _record_data.type_id;
    case ERTTRTypeCategory::Enum:
        return _enum_data.type_id;
    default:
        SKR_UNREACHABLE_CODE()
        return _primitive_data.type_id;
    }
}
inline size_t RTTRType::size() const
{
    switch (_type_category)
    {
    case ERTTRTypeCategory::Primitive:
        return _primitive_data.size;
    case ERTTRTypeCategory::Record:
        return _record_data.size;
    case ERTTRTypeCategory::Enum:
        return _enum_data.size;
    default:
        SKR_UNREACHABLE_CODE()
        return _primitive_data.size;
    }
}
inline size_t RTTRType::alignment() const
{
    switch (_type_category)
    {
    case ERTTRTypeCategory::Primitive:
        return _primitive_data.alignment;
    case ERTTRTypeCategory::Record:
        return _record_data.alignment;
    case ERTTRTypeCategory::Enum:
        return _enum_data.alignment;
    default:
        SKR_UNREACHABLE_CODE()
        return _primitive_data.alignment;
    }
}
inline void RTTRType::each_name_space(FunctionRef<void(StringView)> each_func) const
{
    switch (_type_category)
    {
    case ERTTRTypeCategory::Primitive:
        break;
    case ERTTRTypeCategory::Record:
        for (const String& ns : _record_data.name_space)
        {
            each_func(ns);
        }
        break;
    case ERTTRTypeCategory::Enum:
        for (const String& ns : _enum_data.name_space)
        {
            each_func(ns);
        }
        break;
    default:
        SKR_UNREACHABLE_CODE()
        break;
    }
}
inline MemoryTraitsData RTTRType::memory_traits_data() const
{
    switch (_type_category)
    {
    case ERTTRTypeCategory::Primitive:
        return _primitive_data.memory_traits_data;
    case ERTTRTypeCategory::Record:
        return _record_data.memory_traits_data;
    case ERTTRTypeCategory::Enum:
        return _enum_data.memory_traits_data;
    default:
        SKR_UNREACHABLE_CODE()
        return {};
    }
}

// kind getter
inline bool RTTRType::is_primitive() const
{
    return _type_category == ERTTRTypeCategory::Primitive;
}
inline bool RTTRType::is_record() const
{
    return _type_category == ERTTRTypeCategory::Record;
}
inline bool RTTRType::is_enum() const
{
    return _type_category == ERTTRTypeCategory::Enum;
}

// template find method & field
template <typename Func>
inline ExportCtorInvoker<Func> RTTRType::find_ctor_t(ETypeSignatureCompareFlag flag) const
{
    TypeSignatureTyped<Func> signature;
    return find_ctor(RTTRTypeFindConfig{
        .signature              = signature.view(),
        .signature_compare_flag = flag,
    });
}
template <typename Func>
inline ExportMethodInvoker<Func> RTTRType::find_method_t(StringView name, ETypeSignatureCompareFlag flag, bool include_base) const
{
    TypeSignatureTyped<Func> signature;
    return find_method(RTTRTypeFindConfig{
        .signature              = signature.view(),
        .signature_compare_flag = flag,
        .name                   = name,
        .include_bases          = include_base,
    });
}
template <typename Field>
inline const RTTRFieldData* RTTRType::find_field_t(StringView name, ETypeSignatureCompareFlag flag, bool include_base) const
{
    TypeSignatureTyped<Field> signature;
    return find_field(RTTRTypeFindConfig{
        .signature              = signature.view(),
        .signature_compare_flag = flag,
        .name                   = name,
        .include_bases          = include_base,
    });
}
template <typename Func>
inline ExportStaticMethodInvoker<Func> RTTRType::find_static_method_t(StringView name, ETypeSignatureCompareFlag flag, bool include_base) const
{
    TypeSignatureTyped<Func> signature;
    return find_static_method(RTTRTypeFindConfig{
        .signature              = signature.view(),
        .signature_compare_flag = flag,
        .name                   = name,
        .include_bases          = include_base,
    });
}
template <typename Field>
inline const RTTRStaticFieldData* RTTRType::find_static_field_t(StringView name, ETypeSignatureCompareFlag flag, bool include_base) const
{
    TypeSignatureTyped<Field> signature;
    return find_static_field(RTTRTypeFindConfig{
        .name                   = name,
        .signature              = signature.view(),
        .signature_compare_flag = flag,
        .include_bases          = include_base,
    });
}
template <typename Func>
inline ExportExternMethodInvoker<Func> RTTRType::find_extern_method_t(StringView name, ETypeSignatureCompareFlag flag, bool include_base) const
{
    TypeSignatureTyped<Func> signature;
    return find_extern_method(RTTRTypeFindConfig{
        .name                   = name,
        .signature              = signature.view(),
        .signature_compare_flag = flag,
        .include_bases          = include_base,
    });
}
} // namespace skr