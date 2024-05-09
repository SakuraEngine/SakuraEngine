#pragma once

namespace skr::rttr
{
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

// TODO. count extern methods
// TODO. load extern methods

} // namespace skr::rttr