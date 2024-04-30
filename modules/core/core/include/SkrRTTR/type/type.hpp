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

    SKR_TYPE_CATEGORY_PRIMITIVE,
    SKR_TYPE_CATEGORY_ENUM,
    SKR_TYPE_CATEGORY_RECORD,
    SKR_TYPE_CATEGORY_GENERIC,
};

enum class ETypeFeature : uint32_t
{
    Constructor,
    Destructor,
    Copy,
    Move,
    Assign,
    MoveAssign,
    Hash,
    WriteBinary,
    ReadBinary,
    WriteJson,
    ReadJson
};

struct CPPExternMethods {
    // unary op
    const char* Minus    = "__MINUS__";     // -
    const char* BitNot   = "__BIT_NOT__";   // ~
    const char* LogicNot = "__LOGIC_NOT__"; // !

    // binary op
    const char* Add      = "__ADD__";       // +
    const char* Sub      = "__SUB__";       // -
    const char* Mul      = "__MUL__";       // *
    const char* Div      = "__DIV__";       // /
    const char* Rem      = "__REM__";       // %
    const char* Shl      = "__SHL__";       // <<
    const char* Shr      = "__SHR__";       // >>
    const char* BitAnd   = "__BIT_AND__";   // &
    const char* BitXor   = "__BIT_XOR__";   // ^
    const char* BitOr    = "__BIT_OR__";    // |
    const char* Eq       = "__EQ__";        // ==
    const char* Neq      = "__NEQ__";       // !=
    const char* Lt       = "__LT__";        // <
    const char* Leq      = "__LEQ__";       // <=
    const char* Gt       = "__GT__";        // >
    const char* Geq      = "__GEQ__";       // >=
    const char* LogicAnd = "__LOGIC_AND__"; // &&
    const char* LogicOr  = "__LOGIC_OR__";  // ||

    // assign op
    const char* Assign    = "__ASSIGN__";     // "=
    const char* AddAssign = "__ADD_ASSIGN__"; // +=
    const char* SubAssign = "__SUB_ASSIGN__"; // -=
    const char* MulAssign = "__MUL_ASSIGN__"; // *=
    const char* DivAssign = "__DIV_ASSIGN__"; // /=
    const char* RemAssign = "__REM_ASSIGN__"; // %=
    const char* ShlAssign = "__SHL_ASSIGN__"; // <<=
    const char* ShrAssign = "__SHR_ASSIGN__"; // >>=
    const char* AndAssign = "__AND_ASSIGN__"; // &=
    const char* XorAssign = "__XOR_ASSIGN__"; // ^=
    const char* OrAssign  = "__OR_ASSIGN__";  // |=
};

struct SKR_CORE_API Type {
    Type(ETypeCategory type_category, skr::String name, GUID type_id, size_t size, size_t alignment);
    virtual ~Type() = default;

    // getter
    SKR_INLINE ETypeCategory type_category() const { return _type_category; }
    SKR_INLINE const skr::String& name() const { return _name; }
    SKR_INLINE GUID               type_id() const { return _type_id; }
    SKR_INLINE size_t             size() const { return _size; }
    SKR_INLINE size_t             alignment() const { return _alignment; }

    // feature query
    virtual bool query_feature(ETypeFeature feature) const = 0;

    // call functions
    virtual void   call_ctor(void* ptr) const                    = 0;
    virtual void   call_dtor(void* ptr) const                    = 0;
    virtual void   call_copy(void* dst, const void* src) const   = 0;
    virtual void   call_move(void* dst, void* src) const         = 0;
    virtual void   call_assign(void* dst, const void* src) const = 0;
    virtual void   call_move_assign(void* dst, void* src) const  = 0;
    virtual size_t call_hash(const void* ptr) const              = 0;

    // serialize
    virtual int                   write_binary(const void* dst, skr_binary_writer_t* writer) const = 0;
    virtual int                   read_binary(void* dst, skr_binary_reader_t* reader) const        = 0;
    virtual void                  write_json(const void* dst, skr_json_writer_t* writer) const     = 0;
    virtual skr::json::error_code read_json(void* dst, skr::json::value_t&& reader) const          = 0;

private:
    // basic data
    ETypeCategory _type_category = ETypeCategory::SKR_TYPE_CATEGORY_INVALID;
    skr::String   _name          = {};
    GUID          _type_id       = {};
    size_t        _size          = 0;
    size_t        _alignment     = 0;
};
} // namespace skr::rttr