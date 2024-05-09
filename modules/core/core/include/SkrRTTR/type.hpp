#pragma once
#include <cstdint>
#include "SkrGuid/guid.hpp"
#include "SkrContainers/string.hpp"
#include "SkrBase/types.h"
#include "SkrContainers/multi_map.hpp"
#include "SkrContainers/map.hpp"
#include "SkrContainers/string.hpp"
#include "SkrContainers/span.hpp"
#include "SkrContainers/vector.hpp"
#include "SkrRTTR/enum_value.hpp"
#include "SkrRTTR/enum_traits.hpp"

namespace skr::rttr
{
enum ETypeCategory
{
    SKR_TYPE_CATEGORY_INVALID,

    SKR_TYPE_CATEGORY_PRIMITIVE, // PrimitiveType
    SKR_TYPE_CATEGORY_ENUM,      // EnumType
    SKR_TYPE_CATEGORY_RECORD,    // RecordType
    SKR_TYPE_CATEGORY_GENERIC,   // TODO. remove it
};

// TODO. 移动到新文件中，与模板匹配导出放在一起
struct CPPExternMethods {
    // unary op
    static constexpr const char8_t* Minus    = u8"__MINUS__";     // -
    static constexpr const char8_t* BitNot   = u8"__BIT_NOT__";   // ~
    static constexpr const char8_t* LogicNot = u8"__LOGIC_NOT__"; // !

    // binary op
    static constexpr const char8_t* Add      = u8"__ADD__";       // +
    static constexpr const char8_t* Sub      = u8"__SUB__";       // -
    static constexpr const char8_t* Mul      = u8"__MUL__";       // *
    static constexpr const char8_t* Div      = u8"__DIV__";       // /
    static constexpr const char8_t* Rem      = u8"__REM__";       // %
    static constexpr const char8_t* Shl      = u8"__SHL__";       // <<
    static constexpr const char8_t* Shr      = u8"__SHR__";       // >>
    static constexpr const char8_t* BitAnd   = u8"__BIT_AND__";   // &
    static constexpr const char8_t* BitXor   = u8"__BIT_XOR__";   // ^
    static constexpr const char8_t* BitOr    = u8"__BIT_OR__";    // |
    static constexpr const char8_t* Eq       = u8"__EQ__";        // ==
    static constexpr const char8_t* Neq      = u8"__NEQ__";       // !=
    static constexpr const char8_t* Lt       = u8"__LT__";        // <
    static constexpr const char8_t* Leq      = u8"__LEQ__";       // <=
    static constexpr const char8_t* Gt       = u8"__GT__";        // >
    static constexpr const char8_t* Geq      = u8"__GEQ__";       // >=
    static constexpr const char8_t* LogicAnd = u8"__LOGIC_AND__"; // &&
    static constexpr const char8_t* LogicOr  = u8"__LOGIC_OR__";  // ||

    // assign op
    static constexpr const char8_t* Assign    = u8"__ASSIGN__";     // "=
    static constexpr const char8_t* AddAssign = u8"__ADD_ASSIGN__"; // +=
    static constexpr const char8_t* SubAssign = u8"__SUB_ASSIGN__"; // -=
    static constexpr const char8_t* MulAssign = u8"__MUL_ASSIGN__"; // *=
    static constexpr const char8_t* DivAssign = u8"__DIV_ASSIGN__"; // /=
    static constexpr const char8_t* RemAssign = u8"__REM_ASSIGN__"; // %=
    static constexpr const char8_t* ShlAssign = u8"__SHL_ASSIGN__"; // <<=
    static constexpr const char8_t* ShrAssign = u8"__SHR_ASSIGN__"; // >>=
    static constexpr const char8_t* AndAssign = u8"__AND_ASSIGN__"; // &=
    static constexpr const char8_t* XorAssign = u8"__XOR_ASSIGN__"; // ^=
    static constexpr const char8_t* OrAssign  = u8"__OR_ASSIGN__";  // |=
};

// TODO. PrimitiveType 也使用 RecordType 导出数据结构，可以方便进行扩展
struct SKR_CORE_API Type {
    Type(ETypeCategory type_category, skr::String name, GUID type_id, size_t size, size_t alignment);
    virtual ~Type() = default;

    // getter
    SKR_INLINE ETypeCategory type_category() const { return _type_category; }
    SKR_INLINE const skr::String& name() const { return _name; }
    SKR_INLINE GUID               type_id() const { return _type_id; }
    SKR_INLINE size_t             size() const { return _size; }
    SKR_INLINE size_t             alignment() const { return _alignment; }

    // TODO. uniform invoke interface
    // TODO. API 查找直接返回函数指针，这一限制的理由是 type 都在 CPP 中生产，完全拥有制造函数指针的能力
    //       不像 GenericType 一样，根据传入的 TypeSignature，还需要进行闭包封装
    // ctor & dtor
    // find_ctor(TypeDescView params, EMethodInvokeFlag flag)
    // find_dtor()
    //
    // method & field
    // find_method(String name, TypeDescView ret, span<TypeDescView> params, EMethodInvokeFlag flag)
    // find_static_method(String name, TypeDescView ret, span<TypeDescView> params, EMethodInvokeFlag flag)
    // find_field(String name, TypeDescValue field_type)
    // find_static_field(String name, TypeDescValue field_type)
    //
    // bit_field & extern_method
    // find_bit_field(String name)
    // find_extern_method(String name, TypeDescView ret, span<TypeDescView> params, EMethodInvokeFlag flag)

private:
    // basic data
    ETypeCategory _type_category = ETypeCategory::SKR_TYPE_CATEGORY_INVALID;
    skr::String   _name          = {};
    GUID          _type_id       = {};
    size_t        _size          = 0;
    size_t        _alignment     = 0;
};

struct SKR_CORE_API TypeLoader {
    virtual ~TypeLoader() = default;

    // 这里的二段式加载是为了解决 RecordType 对自身循环依赖的问题
    virtual Type* create()            = 0;
    virtual void  load(Type* type)    = 0;
    virtual void  destroy(Type* type) = 0;
};
} // namespace skr::rttr

// record type
namespace skr::rttr
{
struct BaseInfo {
    Type* type                     = nullptr;
    void* (*cast_func)(void* self) = nullptr;
};
struct Field {
    skr::String name   = {};
    Type*       type   = nullptr;
    size_t      offset = 0;
};
struct ParameterInfo {
    skr::String name = {};
    Type*       type = nullptr;
};
struct Method {
    using ExecutableType = void (*)(void* self, void* parameters, void* return_value);

    skr::String           name            = {};
    Type*                 return_info     = nullptr;
    Vector<ParameterInfo> parameters_info = {};
    ExecutableType        executable      = {};
};

struct SKR_CORE_API RecordType : public Type {
    RecordType(skr::String name, GUID type_id, size_t size, size_t alignment);

    // setup
    void set_base_types(Map<GUID, BaseInfo> base_types);
    void set_fields(MultiMap<skr::String, Field> fields);
    void set_methods(MultiMap<skr::String, Method> methods);

    // getter
    SKR_INLINE const Map<GUID, BaseInfo>& base_types() const { return _base_types_map; }
    SKR_INLINE const MultiMap<skr::String, Field>& fields() const { return _fields_map; }
    SKR_INLINE const MultiMap<skr::String, Method>& methods() const { return _methods_map; }

    // find base
    void* cast_to(const Type* target_type, void* p_self) const;

    // find methods
    // find fields

private:
    Map<GUID, BaseInfo>           _base_types_map = {};
    MultiMap<skr::String, Field>  _fields_map     = {};
    MultiMap<skr::String, Method> _methods_map    = {};
};
} // namespace skr::rttr

// enum type
namespace skr::rttr
{
struct SKR_CORE_API EnumType : public Type {
    EnumType(Type* underlying_type, GUID type_id, String name);

    SKR_INLINE Type* underlying_type() const { return _underlying_type; }

    virtual EnumValue value_from_string(StringView str) const       = 0;
    virtual String    value_to_string(const EnumValue& value) const = 0;

private:
    Type* _underlying_type;
};
} // namespace skr::rttr

// enum type tools
namespace skr::rttr
{
template <typename T>
struct EnumTypeFromTraits : public EnumType {
    EnumTypeFromTraits()
        : EnumType(type_of<std::underlying_type_t<T>>(), type_id_of<T>(), type_name_of<T>())
    {
    }

    EnumValue value_from_string(StringView str) const override
    {
        T result;
        if (EnumTraits<T>::from_string(str, result))
        {
            return EnumValue(static_cast<std::underlying_type_t<T>>(result));
        }
        else
        {
            return {};
        }
    }
    String value_to_string(const EnumValue& value) const override
    {
        T result;
        if (value.cast_to(result))
        {
            return EnumTraits<T>::to_string(result);
        }
        else
        {
            return u8"";
        }
    }
};
} // namespace skr::rttr

// enum type loader help
namespace skr::rttr
{
template <typename T>
struct EnumTypeFromTraitsLoader final : public TypeLoader {
    Type* create() override
    {
        return SkrNew<EnumTypeFromTraits<T>>();
    }
    void load(Type* type) override {}
    void destroy(Type* type) override
    {
        SkrDelete(type);
    }
};
} // namespace skr::rttr