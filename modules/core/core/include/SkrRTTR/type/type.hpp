#pragma once
#include <cstdint>
#include "SkrGuid/guid.hpp"
#include "SkrContainers/string.hpp"
#include "SkrBase/types.h"

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
    inline static const char8_t* Minus    = u8"__MINUS__";     // -
    inline static const char8_t* BitNot   = u8"__BIT_NOT__";   // ~
    inline static const char8_t* LogicNot = u8"__LOGIC_NOT__"; // !

    // binary op
    inline static const char8_t* Add      = u8"__ADD__";       // +
    inline static const char8_t* Sub      = u8"__SUB__";       // -
    inline static const char8_t* Mul      = u8"__MUL__";       // *
    inline static const char8_t* Div      = u8"__DIV__";       // /
    inline static const char8_t* Rem      = u8"__REM__";       // %
    inline static const char8_t* Shl      = u8"__SHL__";       // <<
    inline static const char8_t* Shr      = u8"__SHR__";       // >>
    inline static const char8_t* BitAnd   = u8"__BIT_AND__";   // &
    inline static const char8_t* BitXor   = u8"__BIT_XOR__";   // ^
    inline static const char8_t* BitOr    = u8"__BIT_OR__";    // |
    inline static const char8_t* Eq       = u8"__EQ__";        // ==
    inline static const char8_t* Neq      = u8"__NEQ__";       // !=
    inline static const char8_t* Lt       = u8"__LT__";        // <
    inline static const char8_t* Leq      = u8"__LEQ__";       // <=
    inline static const char8_t* Gt       = u8"__GT__";        // >
    inline static const char8_t* Geq      = u8"__GEQ__";       // >=
    inline static const char8_t* LogicAnd = u8"__LOGIC_AND__"; // &&
    inline static const char8_t* LogicOr  = u8"__LOGIC_OR__";  // ||

    // assign op
    inline static const char8_t* Assign    = u8"__ASSIGN__";     // "=
    inline static const char8_t* AddAssign = u8"__ADD_ASSIGN__"; // +=
    inline static const char8_t* SubAssign = u8"__SUB_ASSIGN__"; // -=
    inline static const char8_t* MulAssign = u8"__MUL_ASSIGN__"; // *=
    inline static const char8_t* DivAssign = u8"__DIV_ASSIGN__"; // /=
    inline static const char8_t* RemAssign = u8"__REM_ASSIGN__"; // %=
    inline static const char8_t* ShlAssign = u8"__SHL_ASSIGN__"; // <<=
    inline static const char8_t* ShrAssign = u8"__SHR_ASSIGN__"; // >>=
    inline static const char8_t* AndAssign = u8"__AND_ASSIGN__"; // &=
    inline static const char8_t* XorAssign = u8"__XOR_ASSIGN__"; // ^=
    inline static const char8_t* OrAssign  = u8"__OR_ASSIGN__";  // |=
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
    //       不像 GenericType 一样，根据传入的 TypeDesc，还需要进行闭包封装
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
} // namespace skr::rttr