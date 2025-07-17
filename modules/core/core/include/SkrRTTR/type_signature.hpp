#pragma once
#include "SkrBase/misc/integer_tools.hpp"
#include "SkrContainersDef/skr_allocator.hpp"
#include "SkrContainersDef/vector.hpp"
#include "SkrRTTR/rttr_traits.hpp"

namespace skr
{
// 每个标记后都会跟上相应的数据，一般结构为：
//  1. 非泛型情况 [modifiers...] [type_id], 必然以 type_id 结束，数据段到 type_id 为止
//  2. 泛型情况 [modifiers...] [generic_type_id] [data_count] [data/type_id...], 通过 data_count 决定数据段长度
//  3. 函数签名情况 [modifiers...] [FunctionSignature] [param_count] [ret_type] [param types...], 通过 param_count 决定数据段长度
//  TODO (没什么必要, 需要的话加) 4. 成员函数签名情况 [modifiers...] [MemberFunctionSignature] [type_id/generic_type_id] [param_count] [ret_type] [param types...], 通过 param_count 决定数据段长度
//  TODO (没什么必要, 需要的话加) 5. 字段签名情况 [modifiers...] [FieldSignature] [type_id/generic_type_id(owner)] [type_id/generic_type_id(type)], 固定两个类型数据段
enum class ETypeSignatureSignal : uint8_t
{
    // mark, means uninitialized or end of signature
    None = 0, // [data]: None

    // type
    TypeId,            // [data]: GUID
    GenericTypeId,     // [data]: GUID, uint32_t(data count)
    FunctionSignature, // [data]: uint32_t(param count)

    // modifier
    Const,     // [data]: None
    Pointer,   // [data]: None
    Ref,       // [data]: None
    RValueRef, // [data]: None
    ArrayDim,  // [data]: uint32_t(dim)

    // data
    Bool,   // [data]: bool
    Int8,   // [data]: int8_t
    Int16,  // [data]: int16_t
    Int32,  // [data]: int32_t
    Int64,  // [data]: int64_t
    UInt8,  // [data]: uint8_t
    UInt16, // [data]: uint16_t
    UInt32, // [data]: uint32_t
    UInt64, // [data]: uint64_t
    Float,  // [data]: float
    Double, // [data]: double
};
namespace __helper
{
inline constexpr size_t get_type_signature_size_of_signal(ETypeSignatureSignal signal)
{
    switch (signal)
    {
    // mark
    case ETypeSignatureSignal::None:
        return sizeof(ETypeSignatureSignal);
    // type info
    case ETypeSignatureSignal::TypeId:
        return sizeof(ETypeSignatureSignal) + sizeof(GUID);
    case ETypeSignatureSignal::GenericTypeId:
        return sizeof(ETypeSignatureSignal) + sizeof(GUID) + sizeof(uint32_t);
    case ETypeSignatureSignal::FunctionSignature:
        return sizeof(ETypeSignatureSignal) + sizeof(uint32_t);
    // modifier
    case ETypeSignatureSignal::Const:
    case ETypeSignatureSignal::Pointer:
    case ETypeSignatureSignal::Ref:
    case ETypeSignatureSignal::RValueRef:
        return sizeof(ETypeSignatureSignal);
    // array dim
    case ETypeSignatureSignal::ArrayDim:
        return sizeof(ETypeSignatureSignal) + sizeof(uint32_t);
    // data
    case ETypeSignatureSignal::Bool:
        return sizeof(ETypeSignatureSignal) + sizeof(bool);
    case ETypeSignatureSignal::Int8:
        return sizeof(ETypeSignatureSignal) + sizeof(int8_t);
    case ETypeSignatureSignal::Int16:
        return sizeof(ETypeSignatureSignal) + sizeof(int16_t);
    case ETypeSignatureSignal::Int32:
        return sizeof(ETypeSignatureSignal) + sizeof(int32_t);
    case ETypeSignatureSignal::Int64:
        return sizeof(ETypeSignatureSignal) + sizeof(int64_t);
    case ETypeSignatureSignal::UInt8:
        return sizeof(ETypeSignatureSignal) + sizeof(uint8_t);
    case ETypeSignatureSignal::UInt16:
        return sizeof(ETypeSignatureSignal) + sizeof(uint16_t);
    case ETypeSignatureSignal::UInt32:
        return sizeof(ETypeSignatureSignal) + sizeof(uint32_t);
    case ETypeSignatureSignal::UInt64:
        return sizeof(ETypeSignatureSignal) + sizeof(uint64_t);
    case ETypeSignatureSignal::Float:
        return sizeof(ETypeSignatureSignal) + sizeof(float);
    case ETypeSignatureSignal::Double:
        return sizeof(ETypeSignatureSignal) + sizeof(double);
    }
    SKR_UNREACHABLE_CODE()
    return SIZE_MAX;
}
} // namespace __helper

// 比较选项
enum class ETypeSignatureCompareFlag : uint8_t
{
    IgnoreConst        = 1 << 0, // const xxx == xxx
    IgnoreRValue       = 1 << 1, // && == &
    RefAsPointer       = 1 << 2, // & == *1
    RValueRefAsPointer = 1 << 3, // && == *1

    Strict          = 0,
    Relax           = IgnoreConst | IgnoreRValue | RefAsPointer | RValueRefAsPointer,
    AllRefAsPointer = RefAsPointer | RValueRefAsPointer | IgnoreRValue, // assume all ref as pointer, sync to decay
};

// 常规化选项
enum class ETypeSignatureDecayFlag : uint8_t
{
    IgnoreConst        = 1 << 0, // remove const
    IgnoreRvalue       = 1 << 1, // translate && to &, sync to compare
    RefAsPointer       = 1 << 2, // translate & to *, sync to compare
    RValueRefAsPointer = 1 << 3, // translate && to *, sync to compare

    Strict          = 0,
    Relax           = IgnoreConst | IgnoreRvalue | RefAsPointer | RValueRefAsPointer,
    AllRefAsPointer = RefAsPointer | RValueRefAsPointer | IgnoreRvalue, // translate all ref to pointer, sync to compare
};

template <ETypeSignatureSignal... signals>
constexpr size_t type_signature_size_v = (__helper::get_type_signature_size_of_signal(signals) + ...);

struct TypeSignatureHelper {
#pragma region MISC HELPER
    // misc helper
    template <typename T>
    inline static uint8_t* write_buffer(uint8_t* pos, const T& value)
    {
        memcpy(pos, &value, sizeof(T));
        return pos + sizeof(T);
    }
    template <typename T>
    inline static const uint8_t* read_buffer(const uint8_t* pos, T& value)
    {
        memcpy(&value, pos, sizeof(T));
        return pos + sizeof(T);
    }
    inline static bool has_enough_buffer(const uint8_t* pos, const uint8_t* end, ETypeSignatureSignal signal)
    {
        return (pos + __helper::get_type_signature_size_of_signal(signal)) <= end;
    }
    inline static bool is_modifier(ETypeSignatureSignal signal)
    {
        switch (signal)
        {
        case ETypeSignatureSignal::Const:
        case ETypeSignatureSignal::Pointer:
        case ETypeSignatureSignal::Ref:
        case ETypeSignatureSignal::RValueRef:
        case ETypeSignatureSignal::ArrayDim:
            return true;
        default:
            return false;
        }
    }
    inline static bool is_data(ETypeSignatureSignal signal)
    {
        switch (signal)
        {
        case ETypeSignatureSignal::Bool:
        case ETypeSignatureSignal::Int8:
        case ETypeSignatureSignal::Int16:
        case ETypeSignatureSignal::Int32:
        case ETypeSignatureSignal::Int64:
        case ETypeSignatureSignal::UInt8:
        case ETypeSignatureSignal::UInt16:
        case ETypeSignatureSignal::UInt32:
        case ETypeSignatureSignal::UInt64:
        case ETypeSignatureSignal::Float:
        case ETypeSignatureSignal::Double:
            return true;
        default:
            return false;
        }
    }
    inline static bool is_reach_end(const uint8_t* pos, const uint8_t* end)
    {
        return pos == end || peek_signal(pos, end) == ETypeSignatureSignal::None;
    }
    inline static const uint8_t* jump_signal(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(pos < end && "invalid signature buffer");
        return pos + sizeof(ETypeSignatureSignal);
    }
    inline static const uint8_t* jump_modifiers(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(pos < end && "invalid signature buffer");
        while (pos < end && is_modifier(peek_signal(pos, end)))
        {
            pos += sizeof(ETypeSignatureSignal);
        }
        return pos;
    }
    inline static const uint8_t* jump_next_data(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(pos < end && "invalid signature buffer");
        ETypeSignatureSignal signal    = peek_signal(pos, end);
        auto                 jump_size = __helper::get_type_signature_size_of_signal(signal);
        SKR_ASSERT(has_enough_buffer(pos, end, signal));
        return pos + jump_size;
    }
    inline static const uint8_t* jump_next_type_or_data(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(pos < end && "invalid signature buffer");

        // jump modifiers
        pos = TypeSignatureHelper::jump_modifiers(pos, end);
        if (pos == end) return pos;

        // get type signal
        auto signal = peek_signal(pos, end);
        switch (signal)
        {
        case ETypeSignatureSignal::TypeId: {
            pos = jump_next_data(pos, end);
            break;
        }
        case ETypeSignatureSignal::GenericTypeId: {
            pos += sizeof(ETypeSignatureSignal) + sizeof(GUID);
            uint32_t data_count;
            pos = read_buffer(pos, data_count);
            for (uint32_t i = 0; i < data_count; ++i)
            {
                pos = jump_next_type_or_data(pos, end);
            }
            break;
        }
        case ETypeSignatureSignal::FunctionSignature: {
            pos += sizeof(ETypeSignatureSignal);
            uint32_t param_count;
            pos = read_buffer(pos, param_count);
            for (uint32_t i = 0; i < param_count + 1; ++i)
            {
                pos = jump_next_type_or_data(pos, end);
            }
            break;
        }
        default:
            // after modifiers, only type signal is valid
            // data signal only appears after generic type signal
            // None signal only appears at the end, after type signal
            SKR_UNREACHABLE_CODE()
        }
        return pos;
    }
#pragma endregion

#pragma region VALIDATE
    // RETURN: the signature type
    //  None: invalid signature
    //  TypeId: valid type signature
    //  GenericTypeId: valid generic type signature
    //  FunctionSignature: valid function signature
    inline static ETypeSignatureSignal validate_complete_signature(const uint8_t* pos, const uint8_t* end)
    {
        // jump modifiers
        pos = TypeSignatureHelper::jump_modifiers(pos, end);
        if (pos == end) return ETypeSignatureSignal::None;

        // get type signal
        auto signal = peek_signal(pos, end);
        switch (signal)
        {
        case ETypeSignatureSignal::TypeId: {
            pos = jump_next_type_or_data(pos, end);
            return is_reach_end(pos, end) ? ETypeSignatureSignal::TypeId : ETypeSignatureSignal::None;
        }
        case ETypeSignatureSignal::GenericTypeId: {
            pos = jump_next_type_or_data(pos, end);
            return is_reach_end(pos, end) ? ETypeSignatureSignal::GenericTypeId : ETypeSignatureSignal::None;
        }
        case ETypeSignatureSignal::FunctionSignature: {
            pos = jump_next_type_or_data(pos, end);
            return is_reach_end(pos, end) ? ETypeSignatureSignal::FunctionSignature : ETypeSignatureSignal::None;
        }
        default:
            return ETypeSignatureSignal::None;
        }
    }
#pragma endregion

#pragma region WRITE HELPER
    // write helper
    inline static uint8_t* write_none(uint8_t* pos, uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::None));
        return write_buffer(pos, ETypeSignatureSignal::None);
    }
    inline static uint8_t* write_type_id(uint8_t* pos, uint8_t* end, const GUID& guid)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::TypeId));
        pos = write_buffer(pos, ETypeSignatureSignal::TypeId);
        return write_buffer(pos, guid);
    }
    inline static uint8_t* write_generic_type_id(uint8_t* pos, uint8_t* end, const GUID& guid, uint32_t data_count)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::GenericTypeId));
        pos = write_buffer(pos, ETypeSignatureSignal::GenericTypeId);
        pos = write_buffer(pos, data_count);
        return write_buffer(pos, guid);
    }
    inline static uint8_t* write_function_signature(uint8_t* pos, uint8_t* end, uint32_t param_count)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::FunctionSignature));
        pos = write_buffer(pos, ETypeSignatureSignal::FunctionSignature);
        return write_buffer(pos, param_count);
    }
    inline static uint8_t* write_const(uint8_t* pos, uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Const));
        return write_buffer(pos, ETypeSignatureSignal::Const);
    }
    inline static uint8_t* write_pointer(uint8_t* pos, uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Pointer));
        return write_buffer(pos, ETypeSignatureSignal::Pointer);
    }
    inline static uint8_t* write_ref(uint8_t* pos, uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Ref));
        return write_buffer(pos, ETypeSignatureSignal::Ref);
    }
    inline static uint8_t* write_rvalue_ref(uint8_t* pos, uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::RValueRef));
        return write_buffer(pos, ETypeSignatureSignal::RValueRef);
    }
    inline static uint8_t* write_array_dim(uint8_t* pos, uint8_t* end, uint32_t dim)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::ArrayDim));
        pos = write_buffer(pos, ETypeSignatureSignal::ArrayDim);
        return write_buffer(pos, dim);
    }
    inline static uint8_t* write_bool(uint8_t* pos, uint8_t* end, bool value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Bool));
        pos = write_buffer(pos, ETypeSignatureSignal::Bool);
        return write_buffer(pos, value);
    }
    inline static uint8_t* write_int8(uint8_t* pos, uint8_t* end, int8_t value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Int8));
        pos = write_buffer(pos, ETypeSignatureSignal::Int8);
        return write_buffer(pos, value);
    }
    inline static uint8_t* write_int16(uint8_t* pos, uint8_t* end, int16_t value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Int16));
        pos = write_buffer(pos, ETypeSignatureSignal::Int16);
        return write_buffer(pos, value);
    }
    inline static uint8_t* write_int32(uint8_t* pos, uint8_t* end, int32_t value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Int32));
        pos = write_buffer(pos, ETypeSignatureSignal::Int32);
        return write_buffer(pos, value);
    }
    inline static uint8_t* write_int64(uint8_t* pos, uint8_t* end, int64_t value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Int64));
        pos = write_buffer(pos, ETypeSignatureSignal::Int64);
        return write_buffer(pos, value);
    }
    inline static uint8_t* write_uint8(uint8_t* pos, uint8_t* end, uint8_t value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::UInt8));
        pos = write_buffer(pos, ETypeSignatureSignal::UInt8);
        return write_buffer(pos, value);
    }
    inline static uint8_t* write_uint16(uint8_t* pos, uint8_t* end, uint16_t value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::UInt16));
        pos = write_buffer(pos, ETypeSignatureSignal::UInt16);
        return write_buffer(pos, value);
    }
    inline static uint8_t* write_uint32(uint8_t* pos, uint8_t* end, uint32_t value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::UInt32));
        pos = write_buffer(pos, ETypeSignatureSignal::UInt32);
        return write_buffer(pos, value);
    }
    inline static uint8_t* write_uint64(uint8_t* pos, uint8_t* end, uint64_t value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::UInt64));
        pos = write_buffer(pos, ETypeSignatureSignal::UInt64);
        return write_buffer(pos, value);
    }
    inline static uint8_t* write_float(uint8_t* pos, uint8_t* end, float value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Float));
        pos = write_buffer(pos, ETypeSignatureSignal::Float);
        return write_buffer(pos, value);
    }
    inline static uint8_t* write_double(uint8_t* pos, uint8_t* end, double value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Double));
        pos = write_buffer(pos, ETypeSignatureSignal::Double);
        return write_buffer(pos, value);
    }
#pragma endregion

#pragma region READ HELPER
    // read helper
    inline static ETypeSignatureSignal peek_signal(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(pos + sizeof(ETypeSignatureSignal) <= end && "read buffer overflow");
        return *(ETypeSignatureSignal*)pos;
    }
    inline static const uint8_t* read_none(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::None));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::None);
        return jump_signal(pos, end);
    }
    inline static const uint8_t* read_type_id(const uint8_t* pos, const uint8_t* end, GUID& guid)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::TypeId));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::TypeId);
        pos = jump_signal(pos, end);
        return read_buffer(pos, guid);
    }
    inline static const uint8_t* read_generic_type_id(const uint8_t* pos, const uint8_t* end, GUID& guid, uint32_t& data_count)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::GenericTypeId));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::GenericTypeId);
        pos = jump_signal(pos, end);
        pos = read_buffer(pos, data_count);
        return read_buffer(pos, guid);
    }
    inline static const uint8_t* read_function_signature(const uint8_t* pos, const uint8_t* end, uint32_t& data_count)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::FunctionSignature));
        auto signal = peek_signal(pos, end);
        SKR_ASSERT(signal == ETypeSignatureSignal::FunctionSignature);
        pos = jump_signal(pos, end);
        return read_buffer(pos, data_count);
    }
    inline static const uint8_t* read_const(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Const));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Const);
        return jump_signal(pos, end);
    }
    inline static const uint8_t* read_pointer(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Pointer));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Pointer);
        return jump_signal(pos, end);
    }
    inline static const uint8_t* read_ref(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Ref));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Ref);
        return jump_signal(pos, end);
    }
    inline static const uint8_t* read_rvalue_ref(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::RValueRef));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::RValueRef);
        return jump_signal(pos, end);
    }
    inline static const uint8_t* read_array_dim(const uint8_t* pos, const uint8_t* end, uint32_t& dim)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::ArrayDim));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::ArrayDim);
        pos = jump_signal(pos, end);
        return read_buffer(pos, dim);
    }
    inline static const uint8_t* read_bool(const uint8_t* pos, const uint8_t* end, bool& value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Bool));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Bool);
        pos = jump_signal(pos, end);
        return read_buffer(pos, value);
    }
    inline static const uint8_t* read_int8(const uint8_t* pos, const uint8_t* end, int8_t& value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Int8));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Int8);
        pos = jump_signal(pos, end);
        return read_buffer(pos, value);
    }
    inline static const uint8_t* read_int16(const uint8_t* pos, const uint8_t* end, int16_t& value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Int16));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Int16);
        pos = jump_signal(pos, end);
        return read_buffer(pos, value);
    }
    inline static const uint8_t* read_int32(const uint8_t* pos, const uint8_t* end, int32_t& value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Int32));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Int32);
        pos = jump_signal(pos, end);
        return read_buffer(pos, value);
    }
    inline static const uint8_t* read_int64(const uint8_t* pos, const uint8_t* end, int64_t& value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Int64));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Int64);
        pos = jump_signal(pos, end);
        return read_buffer(pos, value);
    }
    inline static const uint8_t* read_uint8(const uint8_t* pos, const uint8_t* end, uint8_t& value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::UInt8));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::UInt8);
        pos = jump_signal(pos, end);
        return read_buffer(pos, value);
    }
    inline static const uint8_t* read_uint16(const uint8_t* pos, const uint8_t* end, uint16_t& value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::UInt16));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::UInt16);
        pos = jump_signal(pos, end);
        return read_buffer(pos, value);
    }
    inline static const uint8_t* read_uint32(const uint8_t* pos, const uint8_t* end, uint32_t& value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::UInt32));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::UInt32);
        pos = jump_signal(pos, end);
        return read_buffer(pos, value);
    }
    inline static const uint8_t* read_uint64(const uint8_t* pos, const uint8_t* end, uint64_t& value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::UInt64));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::UInt64);
        pos = jump_signal(pos, end);
        return read_buffer(pos, value);
    }
    inline static const uint8_t* read_float(const uint8_t* pos, const uint8_t* end, float& value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Float));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Float);
        pos = jump_signal(pos, end);
        return read_buffer(pos, value);
    }
    inline static const uint8_t* read_double(const uint8_t* pos, const uint8_t* end, double& value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Double));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Double);
        pos = jump_signal(pos, end);
        return read_buffer(pos, value);
    }
#pragma endregion

#pragma region OPERATOR
    inline static bool signal_equal(
        const uint8_t*&           lhs,
        const uint8_t*            lhs_end,
        const uint8_t*&           rhs,
        const uint8_t*            rhs_end,
        ETypeSignatureCompareFlag flag
    )
    {
        SKR_ASSERT(has_enough_buffer(lhs, lhs_end, peek_signal(lhs, lhs_end)));
        SKR_ASSERT(has_enough_buffer(rhs, rhs_end, peek_signal(rhs, rhs_end)));

        ETypeSignatureSignal lhs_signal = peek_signal(lhs, lhs_end);
        ETypeSignatureSignal rhs_signal = peek_signal(rhs, rhs_end);

        // ref as pointer
        if (flag_any(flag, ETypeSignatureCompareFlag::RefAsPointer))
        {
            lhs_signal = (lhs_signal == ETypeSignatureSignal::Ref) ? ETypeSignatureSignal::Pointer : lhs_signal;
            rhs_signal = (rhs_signal == ETypeSignatureSignal::Ref) ? ETypeSignatureSignal::Pointer : rhs_signal;
        }

        // rvalue ref as pointer
        if (flag_any(flag, ETypeSignatureCompareFlag::RValueRefAsPointer))
        {
            lhs_signal = (lhs_signal == ETypeSignatureSignal::RValueRef) ? ETypeSignatureSignal::Pointer : lhs_signal;
            rhs_signal = (rhs_signal == ETypeSignatureSignal::RValueRef) ? ETypeSignatureSignal::Pointer : rhs_signal;
        }

        // remove rvalue ref
        if (flag_any(flag, ETypeSignatureCompareFlag::IgnoreRValue))
        {
            lhs_signal = (lhs_signal == ETypeSignatureSignal::RValueRef) ? ETypeSignatureSignal::Ref : lhs_signal;
            rhs_signal = (rhs_signal == ETypeSignatureSignal::RValueRef) ? ETypeSignatureSignal::Ref : rhs_signal;
        }

        if (lhs_signal != rhs_signal)
        {
            return false;
        }
        else
        {
            switch (lhs_signal)
            {
            case ETypeSignatureSignal::TypeId: {
                GUID lhs_guid;
                GUID rhs_guid;
                lhs = read_type_id(lhs, lhs_end, lhs_guid);
                rhs = read_type_id(rhs, rhs_end, rhs_guid);
                return lhs_guid == rhs_guid;
            }
            case ETypeSignatureSignal::GenericTypeId: {
                GUID     lhs_guid, rhs_guid;
                uint32_t lhs_count, rhs_count;
                lhs = read_generic_type_id(lhs, lhs_end, lhs_guid, lhs_count);
                rhs = read_generic_type_id(rhs, rhs_end, rhs_guid, rhs_count);
                return lhs_guid == rhs_guid && lhs_count == rhs_count;
            }
            case ETypeSignatureSignal::FunctionSignature: {
                uint32_t lhs_count;
                uint32_t rhs_count;
                lhs = read_function_signature(lhs, lhs_end, lhs_count);
                rhs = read_function_signature(rhs, rhs_end, rhs_count);
                return lhs_count == rhs_count;
            }
            case ETypeSignatureSignal::ArrayDim: {
                uint32_t lhs_dim;
                uint32_t rhs_dim;
                lhs = read_array_dim(lhs, lhs_end, lhs_dim);
                rhs = read_array_dim(rhs, rhs_end, rhs_dim);
                return lhs_dim == rhs_dim;
            }
            case ETypeSignatureSignal::Bool: {
                bool lhs_value;
                bool rhs_value;
                lhs = read_bool(lhs, lhs_end, lhs_value);
                rhs = read_bool(rhs, rhs_end, rhs_value);
                return lhs_value == rhs_value;
            }
            case ETypeSignatureSignal::Int8: {
                int8_t lhs_value;
                int8_t rhs_value;
                lhs = read_int8(lhs, lhs_end, lhs_value);
                rhs = read_int8(rhs, rhs_end, rhs_value);
                return lhs_value == rhs_value;
            }
            case ETypeSignatureSignal::Int16: {
                int16_t lhs_value;
                int16_t rhs_value;
                lhs = read_int16(lhs, lhs_end, lhs_value);
                rhs = read_int16(rhs, rhs_end, rhs_value);
                return lhs_value == rhs_value;
            }
            case ETypeSignatureSignal::Int32: {
                int32_t lhs_value;
                int32_t rhs_value;
                lhs = read_int32(lhs, lhs_end, lhs_value);
                rhs = read_int32(rhs, rhs_end, rhs_value);
                return lhs_value == rhs_value;
            }
            case ETypeSignatureSignal::Int64: {
                int64_t lhs_value;
                int64_t rhs_value;
                lhs = read_int64(lhs, lhs_end, lhs_value);
                rhs = read_int64(rhs, rhs_end, rhs_value);
                return lhs_value == rhs_value;
            }
            case ETypeSignatureSignal::UInt8: {
                uint8_t lhs_value;
                uint8_t rhs_value;
                lhs = read_uint8(lhs, lhs_end, lhs_value);
                rhs = read_uint8(rhs, rhs_end, rhs_value);
                return lhs_value == rhs_value;
            }
            case ETypeSignatureSignal::UInt16: {
                uint16_t lhs_value;
                uint16_t rhs_value;
                lhs = read_uint16(lhs, lhs_end, lhs_value);
                rhs = read_uint16(rhs, rhs_end, rhs_value);
                return lhs_value == rhs_value;
            }
            case ETypeSignatureSignal::UInt32: {
                uint32_t lhs_value;
                uint32_t rhs_value;
                lhs = read_uint32(lhs, lhs_end, lhs_value);
                rhs = read_uint32(rhs, rhs_end, rhs_value);
                return lhs_value == rhs_value;
            }
            case ETypeSignatureSignal::UInt64: {
                uint64_t lhs_value;
                uint64_t rhs_value;
                lhs = read_uint64(lhs, lhs_end, lhs_value);
                rhs = read_uint64(rhs, rhs_end, rhs_value);
                return lhs_value == rhs_value;
            }
            case ETypeSignatureSignal::Float: {
                float lhs_value;
                float rhs_value;
                lhs = read_float(lhs, lhs_end, lhs_value);
                rhs = read_float(rhs, rhs_end, rhs_value);
                return lhs_value == rhs_value;
            }
            case ETypeSignatureSignal::Double: {
                double lhs_value;
                double rhs_value;
                lhs = read_double(lhs, lhs_end, lhs_value);
                rhs = read_double(rhs, rhs_end, rhs_value);
                return lhs_value == rhs_value;
            }
            default: {
                lhs = jump_signal(lhs, lhs_end);
                rhs = jump_signal(rhs, rhs_end);
                return true;
            }
            }
        }
    }
    inline static bool signature_equal(
        const uint8_t*            lhs,
        const uint8_t*            lhs_end,
        const uint8_t*            rhs,
        const uint8_t*            rhs_end,
        ETypeSignatureCompareFlag flag
    )
    {
        SKR_ASSERT(lhs < lhs_end && rhs < rhs_end && "invalid signature buffer");

        while (lhs < lhs_end && rhs < rhs_end)
        {
            // skip const
            if (flag_any(flag, ETypeSignatureCompareFlag::IgnoreConst))
            {
                while (peek_signal(lhs, lhs_end) == ETypeSignatureSignal::Const && lhs < lhs_end)
                    lhs = jump_signal(lhs, lhs_end);
                while (peek_signal(rhs, rhs_end) == ETypeSignatureSignal::Const && rhs < rhs_end)
                    rhs = jump_signal(rhs, rhs_end);
                if (lhs == lhs_end || rhs == rhs_end)
                {
                    // const modifier will never be the last one and we checked empty above
                    SKR_UNREACHABLE_CODE()
                }
            }

            // compare
            if (!signal_equal(lhs, lhs_end, rhs, rhs_end, flag))
            {
                return false;
            }
        }

        // check if both reach the end
        bool lhs_reach_end = lhs == lhs_end || peek_signal(lhs, lhs_end) == ETypeSignatureSignal::None;
        bool rhs_reach_end = rhs == rhs_end || peek_signal(rhs, rhs_end) == ETypeSignatureSignal::None;
        return lhs_reach_end && rhs_reach_end;
    }
    inline static void decay_signature(
        uint8_t*                pos,
        uint8_t*                end,
        ETypeSignatureDecayFlag flag
    )
    {
        uint8_t *read_pos = pos, *write_pos = pos;
        while (read_pos < end)
        {
            // skip const
            if (flag_any(flag, ETypeSignatureDecayFlag::IgnoreConst))
            {
                while (peek_signal(read_pos, end) == ETypeSignatureSignal::Const && read_pos < end)
                    read_pos = const_cast<uint8_t*>(jump_signal(read_pos, end));
                if (read_pos == end)
                    break;
            }

            // get jump size
            auto signal    = peek_signal(read_pos, end);
            auto move_size = __helper::get_type_signature_size_of_signal(signal);

            // write
            switch (signal)
            {
            case ETypeSignatureSignal::Ref: {
                write_buffer(write_pos, flag_any(flag, ETypeSignatureDecayFlag::RefAsPointer) ? ETypeSignatureSignal::Pointer : signal);
                break;
            }
            case ETypeSignatureSignal::RValueRef: {
                write_buffer(write_pos, flag_any(flag, ETypeSignatureDecayFlag::RValueRefAsPointer) ? ETypeSignatureSignal::Pointer : flag_any(flag, ETypeSignatureDecayFlag::IgnoreRvalue) ? ETypeSignatureSignal::Ref :
                                                                                                                                                                                              signal);
                break;
            }
            default: {
                if (read_pos != write_pos)
                {
                    memmove(write_pos, read_pos, move_size);
                }
            }
            }

            // move next
            read_pos += move_size;
            write_pos += move_size;
        }

        // clean end space
        if (write_pos < end)
        {
            memset(write_pos, 0, end - write_pos);
        }
    }
    SKR_CORE_API static String signal_to_string(const uint8_t* pos, const uint8_t* end);
#pragma endregion
};

#pragma region TYPE SIGNATURE VIEW
struct TypeSignatureView {
    // ctor & dtor
    inline TypeSignatureView() = default;
    inline TypeSignatureView(uint8_t* data, uint64_t size)
        : _data(data)
        , _size(size)
    {
    }
    inline ~TypeSignatureView() = default;

    // copy & move
    inline TypeSignatureView(const TypeSignatureView& other) = default;
    inline TypeSignatureView(TypeSignatureView&& other)      = default;

    // assign & move assign
    inline TypeSignatureView& operator=(const TypeSignatureView& other) = default;
    inline TypeSignatureView& operator=(TypeSignatureView&& other)      = default;

    // subviews
    inline TypeSignatureView first(size_t count) const
    {
        SKR_ASSERT(count <= size() && "undefined behavior accessing out of bounds");
        return { data(), count };
    }
    inline TypeSignatureView last(size_t count) const
    {
        SKR_ASSERT(count <= size() && "undefined behavior accessing out of bounds");
        return { data() + size() - count, count };
    }
    inline TypeSignatureView subview(size_t offset, size_t count) const
    {
        SKR_ASSERT(offset <= size() && "undefined behaviour accessing out of bounds");
        return { data() + offset, size() - offset };
    }

    // operator
    inline bool is_complete() const
    {
        return TypeSignatureHelper::validate_complete_signature(_data, _data + _size) != ETypeSignatureSignal::None;
    }
    inline bool is_type() const
    {
        auto pos        = _data;
        auto end        = _data + _size;
        auto jumped_pos = TypeSignatureHelper::jump_modifiers(pos, end);
        return jumped_pos < end && TypeSignatureHelper::peek_signal(jumped_pos, end) == ETypeSignatureSignal::TypeId;
    }
    inline bool is_generic_type() const
    {
        auto pos        = _data;
        auto end        = _data + _size;
        auto jumped_pos = TypeSignatureHelper::jump_modifiers(pos, end);
        return jumped_pos < end && TypeSignatureHelper::peek_signal(jumped_pos, end) == ETypeSignatureSignal::GenericTypeId;
    }
    inline bool is_function() const
    {
        auto pos        = _data;
        auto end        = _data + _size;
        auto jumped_pos = TypeSignatureHelper::jump_modifiers(pos, end);
        return jumped_pos < end && TypeSignatureHelper::peek_signal(jumped_pos, end) == ETypeSignatureSignal::FunctionSignature;
    }
    inline bool has_modifier(ETypeSignatureSignal signal) const
    {
        SKR_ASSERT(TypeSignatureHelper::is_modifier(signal));
        const uint8_t* pos         = _data;
        const uint8_t* end         = _data + _size;
        auto           peek_signal = TypeSignatureHelper::peek_signal(pos, end);
        while (TypeSignatureHelper::is_modifier(peek_signal))
        {
            if (peek_signal == signal)
            {
                return true;
            }
            pos         = TypeSignatureHelper::jump_signal(pos, end);
            peek_signal = TypeSignatureHelper::peek_signal(pos, end);
        }
        return false;
    }
    inline bool is_decayed_pointer() const
    {
        const uint8_t* pos         = _data;
        const uint8_t* end         = _data + _size;
        auto           peek_signal = TypeSignatureHelper::peek_signal(pos, end);
        while (TypeSignatureHelper::is_modifier(peek_signal))
        {
            if (peek_signal == ETypeSignatureSignal::Pointer ||
                peek_signal == ETypeSignatureSignal::Ref ||
                peek_signal == ETypeSignatureSignal::RValueRef)
            {
                return true;
            }
            pos         = TypeSignatureHelper::jump_signal(pos, end);
            peek_signal = TypeSignatureHelper::peek_signal(pos, end);
        }
        return false;
    }
    inline uint32_t decayed_pointer_level()
    {
        uint32_t       level       = 0;
        const uint8_t* pos         = _data;
        const uint8_t* end         = _data + _size;
        auto           peek_signal = TypeSignatureHelper::peek_signal(pos, end);
        while (TypeSignatureHelper::is_modifier(peek_signal))
        {
            if (peek_signal == ETypeSignatureSignal::Pointer ||
                peek_signal == ETypeSignatureSignal::Ref ||
                peek_signal == ETypeSignatureSignal::RValueRef)
            {
                ++level;
                peek_signal = TypeSignatureHelper::peek_signal(pos, end);
            }
            pos         = TypeSignatureHelper::jump_signal(pos, end);
            peek_signal = TypeSignatureHelper::peek_signal(pos, end);
        }
        return level;
    }
    inline bool is_any_ref() const
    {
        return is_ref() || is_rvalue_ref();
    }
    inline bool is_pointer() const
    {
        return has_modifier(ETypeSignatureSignal::Pointer);
    }
    inline bool is_ref() const
    {
        return has_modifier(ETypeSignatureSignal::Ref);
    }
    inline bool is_rvalue_ref() const
    {
        return has_modifier(ETypeSignatureSignal::RValueRef);
    }
    inline bool is_const() const
    {
        return has_modifier(ETypeSignatureSignal::Const);
    }
    inline ETypeSignatureSignal validate_complete_signature() const
    {
        return TypeSignatureHelper::validate_complete_signature(_data, _data + _size);
    }
    inline bool equal(
        const TypeSignatureView&  rhs,
        ETypeSignatureCompareFlag flag = ETypeSignatureCompareFlag::Strict
    ) const
    {
        if (is_empty() && rhs.is_empty())
        {
            // both empty
            return true;
        }
        else if (is_empty() || rhs.is_empty())
        {
            // one of them is empty
            return false;
        }

        return TypeSignatureHelper::signature_equal(
            _data,
            _data + _size,
            rhs._data,
            rhs._data + rhs._size,
            flag
        );
    }
    inline String to_string() const
    {
        return TypeSignatureHelper::signal_to_string(_data, _data + _size);
    }

    // jump
    inline TypeSignatureView jump_modifier()
    {
        auto old_data = _data;
        auto old_end  = _data + _size;
        _data         = const_cast<uint8_t*>(TypeSignatureHelper::jump_modifiers(_data, _data + _size));
        _size         = static_cast<size_t>(old_end - _data);
        return { old_data, static_cast<size_t>(_data - old_data) };
    }
    inline TypeSignatureView jump_next_data()
    {
        auto old_data = _data;
        auto old_end  = _data + _size;
        _data         = const_cast<uint8_t*>(TypeSignatureHelper::jump_next_data(_data, _data + _size));
        _size         = static_cast<size_t>(old_end - _data);
        return { old_data, static_cast<size_t>(_data - old_data) };
    }
    inline TypeSignatureView jump_next_type_or_data()
    {
        auto old_data = _data;
        auto old_end  = _data + _size;
        _data         = const_cast<uint8_t*>(TypeSignatureHelper::jump_next_type_or_data(_data, _data + _size));
        _size         = static_cast<size_t>(old_end - _data);
        return { old_data, static_cast<size_t>(_data - old_data) };
    }

    // getter
    inline uint8_t* data() const { return _data; }
    inline uint64_t size() const { return _size; }
    inline bool     is_empty() const { return _size == 0; }

    // read
    inline ETypeSignatureSignal peek_signal() const
    {
        SKR_ASSERT(!is_empty() && "undefined behavior accessing empty view");
        return TypeSignatureHelper::peek_signal(_data, _data + _size);
    }
    inline bool peek_next_is_modifier() const
    {
        return TypeSignatureHelper::is_modifier(peek_signal());
    }
    inline TypeSignatureView read_none() const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = const_cast<uint8_t*>(TypeSignatureHelper::read_none(pos, end));
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView read_type_id(GUID& guid) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = const_cast<uint8_t*>(TypeSignatureHelper::read_type_id(pos, end, guid));
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView read_generic_type_id(GUID& guid, uint32_t& data_count) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = const_cast<uint8_t*>(TypeSignatureHelper::read_generic_type_id(pos, end, guid, data_count));
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView read_function_signature(uint32_t& data_count) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = const_cast<uint8_t*>(TypeSignatureHelper::read_function_signature(pos, end, data_count));
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView read_const() const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = const_cast<uint8_t*>(TypeSignatureHelper::read_const(pos, end));
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView read_pointer() const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = const_cast<uint8_t*>(TypeSignatureHelper::read_pointer(pos, end));
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView read_ref() const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = const_cast<uint8_t*>(TypeSignatureHelper::read_ref(pos, end));
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView read_rvalue_ref() const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = const_cast<uint8_t*>(TypeSignatureHelper::read_rvalue_ref(pos, end));
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView read_array_dim(uint32_t& dim) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = const_cast<uint8_t*>(TypeSignatureHelper::read_array_dim(pos, end, dim));
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView read_bool(bool& value) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = const_cast<uint8_t*>(TypeSignatureHelper::read_bool(pos, end, value));
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView read_int8(int8_t& value) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = const_cast<uint8_t*>(TypeSignatureHelper::read_int8(pos, end, value));
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView read_int16(int16_t& value) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = const_cast<uint8_t*>(TypeSignatureHelper::read_int16(pos, end, value));
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView read_int32(int32_t& value) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = const_cast<uint8_t*>(TypeSignatureHelper::read_int32(pos, end, value));
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView read_int64(int64_t& value) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = const_cast<uint8_t*>(TypeSignatureHelper::read_int64(pos, end, value));
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView read_uint8(uint8_t& value) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = const_cast<uint8_t*>(TypeSignatureHelper::read_uint8(pos, end, value));
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView read_uint16(uint16_t& value) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = const_cast<uint8_t*>(TypeSignatureHelper::read_uint16(pos, end, value));
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView read_uint32(uint32_t& value) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = const_cast<uint8_t*>(TypeSignatureHelper::read_uint32(pos, end, value));
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView read_uint64(uint64_t& value) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = const_cast<uint8_t*>(TypeSignatureHelper::read_uint64(pos, end, value));
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView read_float(float& value) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = const_cast<uint8_t*>(TypeSignatureHelper::read_float(pos, end, value));
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView read_double(double& value) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = const_cast<uint8_t*>(TypeSignatureHelper::read_double(pos, end, value));
        return { pos, static_cast<size_t>(end - pos) };
    }

private:
    uint8_t* _data = nullptr;
    uint64_t _size = 0;
};
#pragma endregion

#pragma region TYPE SIGNATURE TRAITS
// default as type_id
template <typename T>
struct TypeSignatureTraits {
    inline static constexpr size_t buffer_size = type_signature_size_v<ETypeSignatureSignal::TypeId>;
    inline static uint8_t*         write(uint8_t* pos, uint8_t* end)
    {
        return TypeSignatureHelper::write_type_id(pos, end, RTTRTraits<T>::get_guid());
    }
};

// ignore volatile
template <typename T>
struct TypeSignatureTraits<volatile T> : TypeSignatureTraits<T> {
};

// [MODIFIER] const
template <typename T>
struct TypeSignatureTraits<const T> : TypeSignatureTraits<T> {
    inline static constexpr size_t buffer_size = type_signature_size_v<ETypeSignatureSignal::Const> + TypeSignatureTraits<T>::buffer_size;
    inline static uint8_t*         write(uint8_t* pos, uint8_t* end)
    {
        pos = TypeSignatureHelper::write_const(pos, end);
        return TypeSignatureTraits<T>::write(pos, end);
    }
};

// [MODIFIER] pointer
template <typename T>
struct TypeSignatureTraits<T*> {
    inline static constexpr size_t buffer_size = type_signature_size_v<ETypeSignatureSignal::Pointer> + TypeSignatureTraits<T>::buffer_size;
    inline static uint8_t*         write(uint8_t* pos, uint8_t* end)
    {
        pos = TypeSignatureHelper::write_pointer(pos, end);
        return TypeSignatureTraits<T>::write(pos, end);
    }
};

// [MODIFIER] reference
template <typename T>
struct TypeSignatureTraits<T&> {
    inline static constexpr size_t buffer_size = type_signature_size_v<ETypeSignatureSignal::Ref> + TypeSignatureTraits<T>::buffer_size;
    inline static uint8_t*         write(uint8_t* pos, uint8_t* end)
    {
        pos = TypeSignatureHelper::write_ref(pos, end);
        return TypeSignatureTraits<T>::write(pos, end);
    }
};

// [MODIFIER] rvalue reference
template <typename T>
struct TypeSignatureTraits<T&&> {
    inline static constexpr size_t buffer_size = type_signature_size_v<ETypeSignatureSignal::RValueRef> + TypeSignatureTraits<T>::buffer_size;
    inline static uint8_t*         write(uint8_t* pos, uint8_t* end)
    {
        pos = TypeSignatureHelper::write_rvalue_ref(pos, end);
        return TypeSignatureTraits<T>::write(pos, end);
    }
};

// [MODIFIER] no dim array
template <typename T>
struct TypeSignatureTraits<T[]> {
    inline static constexpr size_t buffer_size = type_signature_size_v<ETypeSignatureSignal::ArrayDim> + TypeSignatureTraits<T>::buffer_size;
    inline static uint8_t*         write(uint8_t* pos, uint8_t* end)
    {
        pos = TypeSignatureHelper::write_pointer(pos, end); // as pointer
        return TypeSignatureTraits<T>::write(pos, end);
    }
};

// [MODIFIER] array
template <typename T, size_t N>
struct TypeSignatureTraits<T[N]> {
    inline static constexpr size_t buffer_size = type_signature_size_v<ETypeSignatureSignal::ArrayDim> + TypeSignatureTraits<T>::buffer_size;
    inline static uint8_t*         write(uint8_t* pos, uint8_t* end)
    {
        pos = TypeSignatureHelper::write_array_dim(pos, end, N);
        return TypeSignatureTraits<T>::write(pos, end);
    }
};

// [MODIFIER] const array
template <typename T, size_t N>
struct TypeSignatureTraits<const T[N]> {
    inline static constexpr size_t buffer_size = type_signature_size_v<ETypeSignatureSignal::ArrayDim> + TypeSignatureTraits<T>::buffer_size;
    inline static uint8_t*         write(uint8_t* pos, uint8_t* end)
    {
        pos = TypeSignatureHelper::write_const(pos, end);
        pos = TypeSignatureHelper::write_array_dim(pos, end, N);
        return TypeSignatureTraits<T>::write(pos, end);
    }
};

// function pointer
template <typename Ret>
struct TypeSignatureTraits<Ret(void)> {
    inline static constexpr size_t buffer_size =
        type_signature_size_v<ETypeSignatureSignal::FunctionSignature> +
        TypeSignatureTraits<Ret>::buffer_size;
    inline static uint8_t* write(uint8_t* pos, uint8_t* end)
    {
        // write function signature
        pos = TypeSignatureHelper::write_function_signature(pos, end, 0);

        // write return type
        pos = TypeSignatureTraits<Ret>::write(pos, end);

        return pos;
    }
};

template <typename Ret, typename... Args>
struct TypeSignatureTraits<Ret(Args...)> {
    inline static constexpr size_t buffer_size =
        type_signature_size_v<ETypeSignatureSignal::FunctionSignature> +
        TypeSignatureTraits<Ret>::buffer_size +
        (TypeSignatureTraits<Args>::buffer_size + ...);
    inline static uint8_t* write(uint8_t* pos, uint8_t* end)
    {
        // write function signature
        pos = TypeSignatureHelper::write_function_signature(pos, end, static_cast<uint32_t>(sizeof...(Args)));

        // write return type
        pos = TypeSignatureTraits<Ret>::write(pos, end);

        // write args
        auto write_arg_helper = []<typename Arg>(uint8_t*& pos, uint8_t* end) {
            pos = TypeSignatureTraits<Arg>::write(pos, end);
        };
        (write_arg_helper.template operator()<Args>(pos, end), ...);

        return pos;
    }
};

#pragma endregion

#pragma region TYPE SIGNATURE
struct TypeSignature : private SkrAllocator {
    friend struct TypeSignatureBuilder;

    // ctor & dtor
    inline TypeSignature() = default;
    inline TypeSignature(size_t size)
        : _data(alloc<uint8_t>(size))
        , _size(size)
    {
        // ETypeSignatureSignal::None = 0
        memset(_data, 0, _size);
    }
    inline TypeSignature(TypeSignatureView view)
        : _data(alloc<uint8_t>(view.size()))
        , _size(view.size())
    {
        memcpy(_data, view.data(), _size);
    }
    inline ~TypeSignature()
    {
        reset();
    }

    // copy & move
    inline TypeSignature(const TypeSignature& other)
        : _data(alloc<uint8_t>(other._size))
        , _size(other._size)
    {
        memcpy(_data, other._data, _size);
    }
    inline TypeSignature(TypeSignature&& other)
        : _data(other._data)
        , _size(other._size)
    {
        other._data = nullptr;
        other._size = 0;
    }

    // assign & move assign
    inline TypeSignature& operator=(const TypeSignature& other)
    {
        if (this != &other)
        {
            reset();
            _data = alloc<uint8_t>(other._size);
            _size = other._size;
            memcpy(_data, other._data, _size);
        }
        return *this;
    }
    inline TypeSignature& operator=(TypeSignature&& other)
    {
        if (this != &other)
        {
            reset();
            _data       = other._data;
            _size       = other._size;
            other._data = nullptr;
            other._size = 0;
        }
        return *this;
    }

    // to view
    inline                   operator TypeSignatureView() const { return { _data, _size }; }
    inline TypeSignatureView view() const { return { _data, _size }; }

    // getter
    inline uint8_t* data() const { return _data; }
    inline size_t   size() const { return _size; }
    inline bool     is_empty() const { return _size == 0; }

    // decay
    inline void decay(ETypeSignatureDecayFlag flag = ETypeSignatureDecayFlag::Relax)
    {
        TypeSignatureHelper::decay_signature(
            data(),
            data() + size(),
            flag
        );
    }

    // ops
    inline void reset()
    {
        if (_data)
        {
            free(_data);
        }
        _size = 0;
    }

private:
    uint8_t* _data = nullptr;
    size_t   _size = 0;
};
struct TypeSignatureBuilder {
    // write helper
    inline void write_none()
    {
        auto add_result = data.add_unsafe(__helper::get_type_signature_size_of_signal(ETypeSignatureSignal::None));
        TypeSignatureHelper::write_none(add_result.ptr(), data.end());
    }
    inline void write_type_id(const GUID& guid)
    {
        auto add_result = data.add_unsafe(__helper::get_type_signature_size_of_signal(ETypeSignatureSignal::TypeId));
        TypeSignatureHelper::write_type_id(add_result.ptr(), data.end(), guid);
    }
    inline void write_generic_type_id(const GUID& guid, uint32_t data_count)
    {
        auto add_result = data.add_unsafe(__helper::get_type_signature_size_of_signal(ETypeSignatureSignal::GenericTypeId));
        TypeSignatureHelper::write_generic_type_id(add_result.ptr(), data.end(), guid, data_count);
    }
    inline void write_function_signature(uint32_t param_count)
    {
        auto add_result = data.add_unsafe(__helper::get_type_signature_size_of_signal(ETypeSignatureSignal::FunctionSignature));
        TypeSignatureHelper::write_function_signature(add_result.ptr(), data.end(), param_count);
    }
    inline void write_const()
    {
        auto add_result = data.add_unsafe(__helper::get_type_signature_size_of_signal(ETypeSignatureSignal::Const));
        TypeSignatureHelper::write_const(add_result.ptr(), data.end());
    }
    inline void write_pointer()
    {
        auto add_result = data.add_unsafe(__helper::get_type_signature_size_of_signal(ETypeSignatureSignal::Pointer));
        TypeSignatureHelper::write_pointer(add_result.ptr(), data.end());
    }
    inline void write_ref()
    {
        auto add_result = data.add_unsafe(__helper::get_type_signature_size_of_signal(ETypeSignatureSignal::Ref));
        TypeSignatureHelper::write_ref(add_result.ptr(), data.end());
    }
    inline void write_rvalue_ref()
    {
        auto add_result = data.add_unsafe(__helper::get_type_signature_size_of_signal(ETypeSignatureSignal::RValueRef));
        TypeSignatureHelper::write_rvalue_ref(add_result.ptr(), data.end());
    }
    inline void write_array_dim(uint32_t dim)
    {
        auto add_result = data.add_unsafe(__helper::get_type_signature_size_of_signal(ETypeSignatureSignal::ArrayDim));
        TypeSignatureHelper::write_array_dim(add_result.ptr(), data.end(), dim);
    }
    inline void write_bool(bool value)
    {
        auto add_result = data.add_unsafe(__helper::get_type_signature_size_of_signal(ETypeSignatureSignal::Bool));
        TypeSignatureHelper::write_bool(add_result.ptr(), data.end(), value);
    }
    inline void write_int8(int8_t value)
    {
        auto add_result = data.add_unsafe(__helper::get_type_signature_size_of_signal(ETypeSignatureSignal::Int8));
        TypeSignatureHelper::write_int8(add_result.ptr(), data.end(), value);
    }
    inline void write_int16(int16_t value)
    {
        auto add_result = data.add_unsafe(__helper::get_type_signature_size_of_signal(ETypeSignatureSignal::Int16));
        TypeSignatureHelper::write_int16(add_result.ptr(), data.end(), value);
    }
    inline void write_int32(int32_t value)
    {
        auto add_result = data.add_unsafe(__helper::get_type_signature_size_of_signal(ETypeSignatureSignal::Int32));
        TypeSignatureHelper::write_int32(add_result.ptr(), data.end(), value);
    }
    inline void write_int64(int64_t value)
    {
        auto add_result = data.add_unsafe(__helper::get_type_signature_size_of_signal(ETypeSignatureSignal::Int64));
        TypeSignatureHelper::write_int64(add_result.ptr(), data.end(), value);
    }
    inline void write_uint8(uint8_t value)
    {
        auto add_result = data.add_unsafe(__helper::get_type_signature_size_of_signal(ETypeSignatureSignal::UInt8));
        TypeSignatureHelper::write_uint8(add_result.ptr(), data.end(), value);
    }
    inline void write_uint16(uint16_t value)
    {
        auto add_result = data.add_unsafe(__helper::get_type_signature_size_of_signal(ETypeSignatureSignal::UInt16));
        TypeSignatureHelper::write_uint16(add_result.ptr(), data.end(), value);
    }
    inline void write_uint32(uint32_t value)
    {
        auto add_result = data.add_unsafe(__helper::get_type_signature_size_of_signal(ETypeSignatureSignal::UInt32));
        TypeSignatureHelper::write_uint32(add_result.ptr(), data.end(), value);
    }
    inline void write_uint64(uint64_t value)
    {
        auto add_result = data.add_unsafe(__helper::get_type_signature_size_of_signal(ETypeSignatureSignal::UInt64));
        TypeSignatureHelper::write_uint64(add_result.ptr(), data.end(), value);
    }
    inline void write_float(float value)
    {
        auto add_result = data.add_unsafe(__helper::get_type_signature_size_of_signal(ETypeSignatureSignal::Float));
        TypeSignatureHelper::write_float(add_result.ptr(), data.end(), value);
    }
    inline void write_double(double value)
    {
        auto add_result = data.add_unsafe(__helper::get_type_signature_size_of_signal(ETypeSignatureSignal::Double));
        TypeSignatureHelper::write_double(add_result.ptr(), data.end(), value);
    }

    // write helpers for const ref and const pointer, to prevent mistake, because const modifier need after pointer or ref
    inline void write_const_ref()
    {
        write_ref();
        write_const();
    }
    inline void write_const_pointer()
    {
        write_pointer();
        write_const();
    }

    // make
    inline TypeSignature type_signature()
    {
        return { TypeSignatureView{ data.data(), data.size() } };
    }
    inline TypeSignatureView type_signature_view()
    {
        return { data.data(), data.size() };
    }

    InlineVector<uint8_t, 256> data;
};
#pragma endregion

#pragma region TYPE SIGNATURE TYPED
template <typename T>
struct TypeSignatureTyped {
    // ctor
    inline TypeSignatureTyped()
    {
        uint8_t* pos = TypeSignatureTraits<T>::write(_data, _data + TypeSignatureTraits<T>::buffer_size);
        SKR_ASSERT(pos == (data() + size()) && "type signature did not write enough data, please check the writer function");
    }

    // copy & move
    inline TypeSignatureTyped(const TypeSignatureTyped& other)
    {
        memcpy(_data, other._data, TypeSignatureTraits<T>::buffer_size);
    }
    inline TypeSignatureTyped(TypeSignatureTyped&& other)
    {
        memcpy(_data, other._data, TypeSignatureTraits<T>::buffer_size);
    }

    // assign & move assign
    inline TypeSignatureTyped& operator=(const TypeSignatureTyped& other)
    {
        if (this != &other)
        {
            memcpy(_data, other._data, TypeSignatureTraits<T>::buffer_size);
        }
        return *this;
    }
    inline TypeSignatureTyped& operator=(TypeSignatureTyped&& other)
    {
        if (this != &other)
        {
            memcpy(_data, other._data, TypeSignatureTraits<T>::buffer_size);
        }
        return *this;
    }

    // to view
    inline                   operator TypeSignatureView() const { return { data(), TypeSignatureTraits<T>::buffer_size }; }
    inline TypeSignatureView view() const { return { data(), TypeSignatureTraits<T>::buffer_size }; }

    // getter
    inline uint8_t* data() const { return const_cast<uint8_t*>(&_data[0]); }
    inline size_t   size() const { return TypeSignatureTraits<T>::buffer_size; }
    inline bool     is_empty() const { return TypeSignatureTraits<T>::buffer_size == 0; }

    // decay
    inline void decay(ETypeSignatureDecayFlag flag = ETypeSignatureDecayFlag::Relax)
    {
        TypeSignatureHelper::decay_signature(
            data(),
            data() + size(),
            flag
        );
    }

private:
    uint8_t _data[TypeSignatureTraits<T>::buffer_size];
};
#pragma endregion

// TODO. type signature builder

// make signature
template <typename T>
inline TypeSignature type_signature_of()
{
    TypeSignature result{ TypeSignatureTraits<T>::buffer_size };
    auto          pos = TypeSignatureTraits<T>::write(result.data(), result.data() + result.size());
    SKR_ASSERT(pos == (result.data() + result.size()) && "type signature did not write enough data, please check the writer function");
    return result;
}

namespace __helper
{
template <typename T, typename Ret, typename... Args>
inline Ret (*decay_method(Ret (T::*)(Args...)))(Args...)
{
}

template <typename T, typename Field>
inline Field decay_field(Field T::*)
{
}
} // namespace __helper

// make signature for function
template <auto func>
inline TypeSignature type_signature_of_function()
{
    return type_signature_of_function<decltype(func)>();
}
template <auto func>
inline TypeSignatureTyped<decltype(func)> type_signature_of_function_typed()
{
    return {};
}

// make signature for method
template <auto method>
inline TypeSignature type_signature_of_method()
{
    return type_signature_of<decltype(__helper::decay_method(method))>();
}
template <auto method>
inline TypeSignatureTyped<decltype(__helper::decay_method(method))> type_signature_of_method_typed()
{
    return {};
}

// make signature for field
template <auto field>
inline TypeSignature type_signature_of_field()
{
    return type_signature_of<decltype(__helper::decay_field(field))>();
}
template <auto field>
inline TypeSignatureTyped<decltype(__helper::decay_field(field))> type_signature_of_field_typed()
{
    return {};
}

} // namespace skr