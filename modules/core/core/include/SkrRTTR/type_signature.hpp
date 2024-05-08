#pragma once
#include "SkrContainers/skr_allocator.hpp"
#include "SkrGuid/guid.hpp"
#include "SkrRTTR/rttr_traits.hpp"

namespace skr::rttr
{
// 每个标记后都会跟上相应的数据，一般结构为：
//  1. 非泛型情况 [modifiers...] [type_id], 必然以 type_id 结束
//  2. 泛型情况 [modifiers...] [generic_type_id] [data, type_id...], 第一个碰到的 generic_type_id 决定了后续的解析方法
//  3. 函数签名情况 [FunctionSignature] [ret_type] [param types...], FunctionSignature 标记后不跟随数据, 仅作为标记，参数间以 separator 分割
enum class ETypeSignatureSignal : uint8_t
{
    // mark, means uninitialized or end of signature
    None = 0, // [data]: None

    // used for function signature, split return type and param types
    Separator, // [data]: None

    // type
    TypeId,            // [data]: GUID
    GenericTypeId,     // [data]: GUID
    FunctionSignature, // [data]: None

    // modifier
    Const,     // [data]: None
    Pointer,   // [data]: None
    Ref,       // [data]: None
    RValueRef, // [data]: None
    ArrayDim,  // [data]: uint32_t

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
        case ETypeSignatureSignal::Separator:
            return sizeof(ETypeSignatureSignal);
        // type guid
        case ETypeSignatureSignal::TypeId:
        case ETypeSignatureSignal::GenericTypeId:
            return sizeof(ETypeSignatureSignal) + sizeof(GUID);
        // type mark
        case ETypeSignatureSignal::FunctionSignature:
            return sizeof(ETypeSignatureSignal);
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
}
} // namespace __helper

template <ETypeSignatureSignal... signals>
constexpr size_t type_signature_size_v = (__helper::get_type_signature_size_of_signal(signals) + ...);

struct TypeSignatureHelper {
#pragma region BUFFER HELPER
    // buffer helper
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
    template <typename T>
    inline static uint8_t* jump_buffer(uint8_t* pos)
    {
        return pos + sizeof(T);
    }
    template <typename T>
    inline static const uint8_t* jump_buffer(const uint8_t* pos)
    {
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
#pragma endregion

#pragma region WRITE HELPER
    // write helper
    inline static uint8_t* write_none(uint8_t* pos, uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::None));
        return write_buffer(pos, ETypeSignatureSignal::None);
    }
    inline static uint8_t* write_separator(uint8_t* pos, uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Separator));
        return write_buffer(pos, ETypeSignatureSignal::Separator);
    }
    inline static uint8_t* write_type_id(uint8_t* pos, uint8_t* end, const GUID& guid)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::TypeId));
        pos = write_buffer(pos, ETypeSignatureSignal::TypeId);
        return write_buffer(pos, guid);
    }
    inline static uint8_t* write_generic_type_id(uint8_t* pos, uint8_t* end, const GUID& guid)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::GenericTypeId));
        pos = write_buffer(pos, ETypeSignatureSignal::GenericTypeId);
        return write_buffer(pos, guid);
    }
    inline static uint8_t* write_function_signature(uint8_t* pos, uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::FunctionSignature));
        return write_buffer(pos, ETypeSignatureSignal::FunctionSignature);
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
        return jump_buffer<ETypeSignatureSignal>(pos);
    }
    inline static const uint8_t* read_separator(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Separator));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Separator);
        return jump_buffer<ETypeSignatureSignal>(pos);
    }
    inline static const uint8_t* read_type_id(const uint8_t* pos, const uint8_t* end, GUID& guid)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::TypeId));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::TypeId);
        pos = jump_buffer<ETypeSignatureSignal>(pos);
        return read_buffer(pos, guid);
    }
    inline static const uint8_t* read_generic_type_id(const uint8_t* pos, const uint8_t* end, GUID& guid)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::GenericTypeId));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::GenericTypeId);
        pos = jump_buffer<ETypeSignatureSignal>(pos);
        return read_buffer(pos, guid);
    }
    inline static const uint8_t* read_function_signature(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::FunctionSignature));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::FunctionSignature);
        return jump_buffer<ETypeSignatureSignal>(pos);
    }
    inline static const uint8_t* read_const(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Const));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Const);
        return jump_buffer<ETypeSignatureSignal>(pos);
    }
    inline static const uint8_t* read_pointer(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Pointer));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Pointer);
        return jump_buffer<ETypeSignatureSignal>(pos);
    }
    inline static const uint8_t* read_ref(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Ref));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Ref);
        return jump_buffer<ETypeSignatureSignal>(pos);
    }
    inline static const uint8_t* read_rvalue_ref(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::RValueRef));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::RValueRef);
        return jump_buffer<ETypeSignatureSignal>(pos);
    }
    inline static const uint8_t* read_array_dim(const uint8_t* pos, const uint8_t* end, uint32_t& dim)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::ArrayDim));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::ArrayDim);
        pos = jump_buffer<ETypeSignatureSignal>(pos);
        return read_buffer(pos, dim);
    }
    inline static const uint8_t* read_bool(const uint8_t* pos, const uint8_t* end, bool& value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Bool));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Bool);
        pos = jump_buffer<ETypeSignatureSignal>(pos);
        return read_buffer(pos, value);
    }
    inline static const uint8_t* read_int8(const uint8_t* pos, const uint8_t* end, int8_t& value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Int8));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Int8);
        pos = jump_buffer<ETypeSignatureSignal>(pos);
        return read_buffer(pos, value);
    }
    inline static const uint8_t* read_int16(const uint8_t* pos, const uint8_t* end, int16_t& value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Int16));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Int16);
        pos = jump_buffer<ETypeSignatureSignal>(pos);
        return read_buffer(pos, value);
    }
    inline static const uint8_t* read_int32(const uint8_t* pos, const uint8_t* end, int32_t& value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Int32));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Int32);
        pos = jump_buffer<ETypeSignatureSignal>(pos);
        return read_buffer(pos, value);
    }
    inline static const uint8_t* read_int64(const uint8_t* pos, const uint8_t* end, int64_t& value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Int64));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Int64);
        pos = jump_buffer<ETypeSignatureSignal>(pos);
        return read_buffer(pos, value);
    }
    inline static const uint8_t* read_uint8(const uint8_t* pos, const uint8_t* end, uint8_t& value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::UInt8));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::UInt8);
        pos = jump_buffer<ETypeSignatureSignal>(pos);
        return read_buffer(pos, value);
    }
    inline static const uint8_t* read_uint16(const uint8_t* pos, const uint8_t* end, uint16_t& value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::UInt16));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::UInt16);
        pos = jump_buffer<ETypeSignatureSignal>(pos);
        return read_buffer(pos, value);
    }
    inline static const uint8_t* read_uint32(const uint8_t* pos, const uint8_t* end, uint32_t& value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::UInt32));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::UInt32);
        pos = jump_buffer<ETypeSignatureSignal>(pos);
        return read_buffer(pos, value);
    }
    inline static const uint8_t* read_uint64(const uint8_t* pos, const uint8_t* end, uint64_t& value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::UInt64));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::UInt64);
        pos = jump_buffer<ETypeSignatureSignal>(pos);
        return read_buffer(pos, value);
    }
    inline static const uint8_t* read_float(const uint8_t* pos, const uint8_t* end, float& value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Float));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Float);
        pos = jump_buffer<ETypeSignatureSignal>(pos);
        return read_buffer(pos, value);
    }
    inline static const uint8_t* read_double(const uint8_t* pos, const uint8_t* end, double& value)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Double));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Double);
        pos = jump_buffer<ETypeSignatureSignal>(pos);
        return read_buffer(pos, value);
    }
#pragma endregion

#pragma region JUMP HELPER
    // jump helper
    inline static const uint8_t* jump_none(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::None));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::None);
        return jump_buffer<ETypeSignatureSignal>(pos);
    }
    inline static const uint8_t* jump_separator(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Separator));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Separator);
        return jump_buffer<ETypeSignatureSignal>(pos);
    }
    inline static const uint8_t* jump_type_id(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::TypeId));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::TypeId);
        pos = jump_buffer<ETypeSignatureSignal>(pos);
        return jump_buffer<GUID>(pos);
    }
    inline static const uint8_t* jump_generic_type_id(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::GenericTypeId));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::GenericTypeId);
        pos = jump_buffer<ETypeSignatureSignal>(pos);
        return jump_buffer<GUID>(pos);
    }
    inline static const uint8_t* jump_function_signature(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::FunctionSignature));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::FunctionSignature);
        return jump_buffer<ETypeSignatureSignal>(pos);
    }
    inline static const uint8_t* jump_const(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Const));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Const);
        return jump_buffer<ETypeSignatureSignal>(pos);
    }
    inline static const uint8_t* jump_pointer(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Pointer));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Pointer);
        return jump_buffer<ETypeSignatureSignal>(pos);
    }
    inline static const uint8_t* jump_ref(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Ref));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Ref);
        return jump_buffer<ETypeSignatureSignal>(pos);
    }
    inline static const uint8_t* jump_rvalue_ref(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::RValueRef));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::RValueRef);
        return jump_buffer<ETypeSignatureSignal>(pos);
    }
    inline static const uint8_t* jump_array_dim(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::ArrayDim));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::ArrayDim);
        pos = jump_buffer<ETypeSignatureSignal>(pos);
        return jump_buffer<uint32_t>(pos);
    }
    inline static const uint8_t* jump_bool(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Bool));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Bool);
        pos = jump_buffer<ETypeSignatureSignal>(pos);
        return jump_buffer<bool>(pos);
    }
    inline static const uint8_t* jump_int8(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Int8));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Int8);
        pos = jump_buffer<ETypeSignatureSignal>(pos);
        return jump_buffer<int8_t>(pos);
    }
    inline static const uint8_t* jump_int16(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Int16));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Int16);
        pos = jump_buffer<ETypeSignatureSignal>(pos);
        return jump_buffer<int16_t>(pos);
    }
    inline static const uint8_t* jump_int32(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Int32));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Int32);
        pos = jump_buffer<ETypeSignatureSignal>(pos);
        return jump_buffer<int32_t>(pos);
    }
    inline static const uint8_t* jump_int64(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Int64));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Int64);
        pos = jump_buffer<ETypeSignatureSignal>(pos);
        return jump_buffer<int64_t>(pos);
    }
    inline static const uint8_t* jump_uint8(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::UInt8));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::UInt8);
        pos = jump_buffer<ETypeSignatureSignal>(pos);
        return jump_buffer<uint8_t>(pos);
    }
    inline static const uint8_t* jump_uint16(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::UInt16));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::UInt16);
        pos = jump_buffer<ETypeSignatureSignal>(pos);
        return jump_buffer<uint16_t>(pos);
    }
    inline static const uint8_t* jump_uint32(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::UInt32));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::UInt32);
        pos = jump_buffer<ETypeSignatureSignal>(pos);
        return jump_buffer<uint32_t>(pos);
    }
    inline static const uint8_t* jump_uint64(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::UInt64));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::UInt64);
        pos = jump_buffer<ETypeSignatureSignal>(pos);
        return jump_buffer<uint64_t>(pos);
    }
    inline static const uint8_t* jump_float(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Float));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Float);
        pos = jump_buffer<ETypeSignatureSignal>(pos);
        return jump_buffer<float>(pos);
    }
    inline static const uint8_t* jump_double(const uint8_t* pos, const uint8_t* end)
    {
        SKR_ASSERT(has_enough_buffer(pos, end, ETypeSignatureSignal::Double));
        SKR_ASSERT(peek_signal(pos, end) == ETypeSignatureSignal::Double);
        pos = jump_buffer<ETypeSignatureSignal>(pos);
        return jump_buffer<double>(pos);
    }
    inline static const uint8_t* jump_next_data(const uint8_t* pos, const uint8_t* end)
    {
        ETypeSignatureSignal signal    = peek_signal(pos, end);
        auto                 jump_size = __helper::get_type_signature_size_of_signal(signal);
        SKR_ASSERT(has_enough_buffer(pos, end, signal));
        return pos + jump_size;
    }
    inline static const uint8_t* jump_modifiers(const uint8_t* pos, const uint8_t* end)
    {
        while (pos < end && is_modifier(peek_signal(pos, end)))
        {
            pos = jump_buffer<ETypeSignatureSignal>(pos);
        }
        return pos;
    }
#pragma endregion

#pragma region OPERATOR
    inline static bool signal_equal(
        const uint8_t*& lhs,
        const uint8_t*  lhs_end,
        const uint8_t*& rhs,
        const uint8_t*  rhs_end,
        bool            ref_as_pointer,
        bool            rvalue_ref_as_pointer)
    {
        SKR_ASSERT(has_enough_buffer(lhs, lhs_end, peek_signal(lhs, lhs_end)));
        SKR_ASSERT(has_enough_buffer(rhs, rhs_end, peek_signal(rhs, rhs_end)));

        ETypeSignatureSignal lhs_signal = peek_signal(lhs, lhs_end);
        ETypeSignatureSignal rhs_signal = peek_signal(rhs, rhs_end);

        if (ref_as_pointer)
        {
            lhs_signal = (lhs_signal == ETypeSignatureSignal::Ref) ? ETypeSignatureSignal::Pointer : lhs_signal;
            rhs_signal = (rhs_signal == ETypeSignatureSignal::Ref) ? ETypeSignatureSignal::Pointer : rhs_signal;
        }

        if (rvalue_ref_as_pointer)
        {
            lhs_signal = (lhs_signal == ETypeSignatureSignal::RValueRef) ? ETypeSignatureSignal::Pointer : lhs_signal;
            rhs_signal = (rhs_signal == ETypeSignatureSignal::RValueRef) ? ETypeSignatureSignal::Pointer : rhs_signal;
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
                    GUID lhs_guid;
                    GUID rhs_guid;
                    lhs = read_generic_type_id(lhs, lhs_end, lhs_guid);
                    rhs = read_generic_type_id(rhs, rhs_end, rhs_guid);
                    return lhs_guid == rhs_guid;
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
                    lhs = jump_buffer<ETypeSignatureSignal>(lhs);
                    rhs = jump_buffer<ETypeSignatureSignal>(rhs);
                    return true;
                }
            }
        }
    }
    inline static bool signature_equal(
        const uint8_t* lhs,
        const uint8_t* lhs_end,
        const uint8_t* rhs,
        const uint8_t* rhs_end,
        bool           ref_as_pointer,
        bool           rvalue_ref_as_pointer,
        bool           ignore_const)
    {
        SKR_ASSERT(lhs < lhs_end && rhs < rhs_end && "invalid signature buffer");

        while (lhs < lhs_end && rhs < rhs_end)
        {
            // skip const
            if (ignore_const)
            {
                while (peek_signal(lhs, lhs_end) == ETypeSignatureSignal::Const && lhs < lhs_end)
                    lhs = jump_const(lhs, lhs_end);
                while (peek_signal(rhs, rhs_end) == ETypeSignatureSignal::Const && rhs < rhs_end)
                    rhs = jump_const(rhs, rhs_end);
                if (lhs == lhs_end || rhs == rhs_end)
                {
                    // const modifier will never be the last one and we checked empty above
                    SKR_UNREACHABLE_CODE()
                }
            }

            // compare
            if (!signal_equal(lhs, lhs_end, rhs, rhs_end, ref_as_pointer, rvalue_ref_as_pointer))
            {
                return false;
            }
        }

        // check if both reach the end
        bool lhs_reach_end = lhs == lhs_end || peek_signal(lhs, lhs_end) == ETypeSignatureSignal::None;
        bool rhs_reach_end = rhs == rhs_end || peek_signal(rhs, rhs_end) == ETypeSignatureSignal::None;
        return lhs_reach_end && rhs_reach_end;
    }
    inline static void normalize_signal(
        uint8_t* pos,
        uint8_t* end,
        bool     ref_as_pointer,
        bool     rvalue_ref_as_pointer,
        bool     ignore_const)
    {
        uint8_t *read_pos = pos, *write_pos = pos;
        while (read_pos < end)
        {
            // skip const
            if (ignore_const)
            {
                while (peek_signal(read_pos, end) == ETypeSignatureSignal::Const && read_pos < end)
                    read_pos = const_cast<uint8_t*>(jump_const(read_pos, end));
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
                    write_buffer(write_pos, ref_as_pointer ? ETypeSignatureSignal::Pointer : signal);
                    break;
                }
                case ETypeSignatureSignal::RValueRef: {
                    write_buffer(write_pos, rvalue_ref_as_pointer ? ETypeSignatureSignal::Pointer : signal);
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
    inline bool is_function_signature() const
    {
        auto pos = _data;
        auto end = _data + _size;
        return pos < end && TypeSignatureHelper::peek_signal(pos, end) == ETypeSignatureSignal::FunctionSignature;
    }
    inline bool split_by_separator(TypeSignatureView& left, TypeSignatureView& right) const
    {
        auto pos = _data;
        auto end = _data + _size;
        while (pos < end)
        {
            auto signal = TypeSignatureHelper::peek_signal(pos, end);
            if (signal == ETypeSignatureSignal::Separator)
            {
                auto right_begin = const_cast<uint8_t*>(TypeSignatureHelper::jump_separator(pos, end));
                left             = { _data, static_cast<size_t>(pos - _data) };
                right            = { right_begin, static_cast<size_t>(end - right_begin) };
                return true;
            }
            else
            {
                pos = const_cast<uint8_t*>(TypeSignatureHelper::jump_next_data(pos, end));
            }
        }
        return false;
    }
    inline bool equal(
        const TypeSignatureView& rhs,
        bool                     ref_as_pointer        = true,
        bool                     rvalue_ref_as_pointer = true,
        bool                     ignore_const          = true) const
    {
        if (empty() && rhs.empty())
        {
            // both empty
            return true;
        }
        else if (empty() || rhs.empty())
        {
            // one of them is empty
            return false;
        }

        return TypeSignatureHelper::signature_equal(
            _data,
            _data + _size,
            rhs._data,
            rhs._data + rhs._size,
            ref_as_pointer,
            rvalue_ref_as_pointer,
            ignore_const);
    }
    inline void normalize(
        bool ref_as_pointer        = true,
        bool rvalue_ref_as_pointer = true,
        bool ignore_const          = true)
    {
        TypeSignatureHelper::normalize_signal(
            _data,
            _data + _size,
            ref_as_pointer,
            rvalue_ref_as_pointer,
            ignore_const);
    }
    inline String to_string() const
    {
        return TypeSignatureHelper::signal_to_string(_data, _data + _size);
    }

    // getter
    inline uint8_t* data() const { return _data; }
    inline uint64_t size() const { return _size; }
    inline bool     empty() const { return _size == 0; }

    // read
    inline ETypeSignatureSignal peek_signal() const
    {
        SKR_ASSERT(!empty() && "undefined behavior accessing empty view");
        return TypeSignatureHelper::peek_signal(_data, _data + _size);
    }
    inline TypeSignatureView read_none() const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = const_cast<uint8_t*>(TypeSignatureHelper::read_none(pos, end));
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView read_separator() const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = const_cast<uint8_t*>(TypeSignatureHelper::read_separator(pos, end));
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView read_type_id(GUID& guid) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = const_cast<uint8_t*>(TypeSignatureHelper::read_type_id(pos, end, guid));
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView read_generic_type_id(GUID& guid) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = const_cast<uint8_t*>(TypeSignatureHelper::read_generic_type_id(pos, end, guid));
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView read_function_signature() const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = const_cast<uint8_t*>(TypeSignatureHelper::read_function_signature(pos, end));
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

    // write
    inline TypeSignatureView write_none() const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = TypeSignatureHelper::write_none(pos, end);
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView write_separator() const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = TypeSignatureHelper::write_separator(pos, end);
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView write_type_id(const GUID& guid) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = TypeSignatureHelper::write_type_id(pos, end, guid);
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView write_generic_type_id(const GUID& guid) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = TypeSignatureHelper::write_generic_type_id(pos, end, guid);
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView write_function_signature() const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = TypeSignatureHelper::write_function_signature(pos, end);
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView write_const() const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = TypeSignatureHelper::write_const(pos, end);
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView write_pointer() const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = TypeSignatureHelper::write_pointer(pos, end);
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView write_ref() const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = TypeSignatureHelper::write_ref(pos, end);
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView write_rvalue_ref() const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = TypeSignatureHelper::write_rvalue_ref(pos, end);
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView write_array_dim(uint32_t dim) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = TypeSignatureHelper::write_array_dim(pos, end, dim);
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView write_bool(bool value) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = TypeSignatureHelper::write_bool(pos, end, value);
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView write_int8(int8_t value) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = TypeSignatureHelper::write_int8(pos, end, value);
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView write_int16(int16_t value) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = TypeSignatureHelper::write_int16(pos, end, value);
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView write_int32(int32_t value) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = TypeSignatureHelper::write_int32(pos, end, value);
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView write_int64(int64_t value) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = TypeSignatureHelper::write_int64(pos, end, value);
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView write_uint8(uint8_t value) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = TypeSignatureHelper::write_uint8(pos, end, value);
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView write_uint16(uint16_t value) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = TypeSignatureHelper::write_uint16(pos, end, value);
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView write_uint32(uint32_t value) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = TypeSignatureHelper::write_uint32(pos, end, value);
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView write_uint64(uint64_t value) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = TypeSignatureHelper::write_uint64(pos, end, value);
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView write_float(float value) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = TypeSignatureHelper::write_float(pos, end, value);
        return { pos, static_cast<size_t>(end - pos) };
    }
    inline TypeSignatureView write_double(double value) const
    {
        auto pos = _data;
        auto end = _data + _size;
        pos      = TypeSignatureHelper::write_double(pos, end, value);
        return { pos, static_cast<size_t>(end - pos) };
    }

private:
    uint8_t* _data = nullptr;
    uint64_t _size = 0;
};
#pragma endregion

#pragma regin TYPE SIGNATURE TRAITS
// default as type_id
template <typename T>
struct TypeSignatureTraits {
    inline static constexpr size_t buffer_size = type_signature_size_v<ETypeSignatureSignal::TypeId>;
    inline static void             write(uint8_t* pos, uint8_t* end)
    {
        TypeSignatureHelper::write_type_id(pos, end, RTTRTraits<T>::get_guid());
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
    inline static void             write(uint8_t* pos, uint8_t* end)
    {
        pos = TypeSignatureHelper::write_const(pos, end);
        TypeSignatureTraits<T>::write(pos, end);
    }
};

// [MODIFIER] pointer
template <typename T>
struct TypeSignatureTraits<T*> {
    inline static constexpr size_t buffer_size = type_signature_size_v<ETypeSignatureSignal::Pointer> + TypeSignatureTraits<T>::buffer_size;
    inline static void             write(uint8_t* pos, uint8_t* end)
    {
        pos = TypeSignatureHelper::write_pointer(pos, end);
        TypeSignatureTraits<T>::write(pos, end);
    }
};

// [MODIFIER] reference
template <typename T>
struct TypeSignatureTraits<T&> {
    inline static constexpr size_t buffer_size = type_signature_size_v<ETypeSignatureSignal::Ref> + TypeSignatureTraits<T>::buffer_size;
    inline static void             write(uint8_t* pos, uint8_t* end)
    {
        pos = TypeSignatureHelper::write_ref(pos, end);
        TypeSignatureTraits<T>::write(pos, end);
    }
};

// [MODIFIER] rvalue reference
template <typename T>
struct TypeSignatureTraits<T&&> {
    inline static constexpr size_t buffer_size = type_signature_size_v<ETypeSignatureSignal::RValueRef> + TypeSignatureTraits<T>::buffer_size;
    inline static void             write(uint8_t* pos, uint8_t* end)
    {
        pos = TypeSignatureHelper::write_rvalue_ref(pos, end);
        TypeSignatureTraits<T>::write(pos, end);
    }
};

// [MODIFIER] no dim array
template <typename T>
struct TypeSignatureTraits<T[]> {
    inline static constexpr size_t buffer_size = type_signature_size_v<ETypeSignatureSignal::ArrayDim> + TypeSignatureTraits<T>::buffer_size;
    inline static void             write(uint8_t* pos, uint8_t* end)
    {
        pos = TypeSignatureHelper::write_pointer(pos, end); // as pointer
        TypeSignatureTraits<T>::write(pos, end);
    }
};

// [MODIFIER] array
template <typename T, size_t N>
struct TypeSignatureTraits<T[N]> {
    inline static constexpr size_t buffer_size = type_signature_size_v<ETypeSignatureSignal::ArrayDim> + TypeSignatureTraits<T>::buffer_size;
    inline static void             write(uint8_t* pos, uint8_t* end)
    {
        pos = TypeSignatureHelper::write_array_dim(pos, end, N);
        TypeSignatureTraits<T>::write(pos, end);
    }
};

// [MODIFIER] const array
template <typename T, size_t N>
struct TypeSignatureTraits<const T[N]> {
    inline static constexpr size_t buffer_size = type_signature_size_v<ETypeSignatureSignal::ArrayDim> + TypeSignatureTraits<T>::buffer_size;
    inline static void             write(uint8_t* pos, uint8_t* end)
    {
        pos = TypeSignatureHelper::write_const(pos, end);
        pos = TypeSignatureHelper::write_array_dim(pos, end, N);
        TypeSignatureTraits<T>::write(pos, end);
    }
};
#pragma endregion

#pragma regin TYPE SIGNATURE
struct TypeSignature : private SkrAllocator {
    using TypeSignatureWriter = void (*)(uint8_t* pos, uint8_t* end);

    // ctor & dtor
    inline TypeSignature() = default;
    inline TypeSignature(size_t size, TypeSignatureWriter writer)
        : _data(alloc<uint8_t>(size))
        , _size(size)
    {
        // ETypeSignatureSignal::None = 0
        memset(_data, 0, _size);
        writer(_data, _data + _size);
    }
    inline TypeSignature(TypeSignatureView view)
        : _data(alloc<uint8_t>(view.size()))
        , _size(view.size())
    {
        memcpy(_data, view.data(), _size);
    }
    inline ~TypeSignature()
    {
        if (_data)
        {
            free(_data);
        }
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
            if (_data)
            {
                free(_data);
            }
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
            if (_data)
            {
                free(_data);
            }
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
    inline bool     empty() const { return _size == 0; }

private:
    uint8_t* _data = nullptr;
    size_t   _size = 0;
};
#pragma endregion

#pragma regin TYPE SIGNATURE TYPED
template <typename T>
struct TypeSignatureTyped {
    // ctor
    inline TypeSignatureTyped() { TypeSignatureTraits<T>::write(_data, _data + TypeSignatureTraits<T>::buffer_size); }

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
    inline bool     empty() const { return TypeSignatureTraits<T>::buffer_size == 0; }

private:
    uint8_t _data[TypeSignatureTraits<T>::buffer_size];
};
#pragma endregion

// make signature
template <typename T>
inline TypeSignature type_signature_of()
{
    return TypeSignature(TypeSignatureTraits<T>::buffer_size, TypeSignatureTraits<T>::write);
}
} // namespace skr::rttr