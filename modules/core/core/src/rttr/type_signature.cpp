#include "SkrRTTR/type_signature.hpp"
#include "SkrRTTR/type.hpp"
#include "SkrRTTR/type_registry.hpp"

namespace skr::rttr
{
inline static const uint8_t* _append_modifier(const uint8_t* pos, const uint8_t* end, String& result, ETypeSignatureSignal signal)
{
    switch (signal)
    {
        case ETypeSignatureSignal::Const: {
            pos    = TypeSignatureHelper::read_const(pos, end);
            result = skr::format(u8"const {}", result);
            break;
        }
        case ETypeSignatureSignal::Pointer: {
            pos    = TypeSignatureHelper::read_pointer(pos, end);
            result = skr::format(u8"* {}", result);
            break;
        }
        case ETypeSignatureSignal::Ref: {
            pos    = TypeSignatureHelper::read_ref(pos, end);
            result = skr::format(u8"& {}", result);
            break;
        }
        case ETypeSignatureSignal::RValueRef: {
            pos    = TypeSignatureHelper::read_rvalue_ref(pos, end);
            result = skr::format(u8"&& {}", result);
            break;
        }
        case ETypeSignatureSignal::ArrayDim: {
            uint32_t dim;
            pos    = TypeSignatureHelper::read_array_dim(pos, end, dim);
            result = skr::format(u8"[{}] {}", dim, result);
            break;
        }
    }
    return pos;
}

inline static const uint8_t* _append_data(const uint8_t* pos, const uint8_t* end, String& result, ETypeSignatureSignal signal)
{
    switch (signal)
    {
        case ETypeSignatureSignal::Bool: {
            bool value;
            pos    = TypeSignatureHelper::read_bool(pos, end, value);
            result = skr::format(u8"{}{}", result, (value ? "true" : "false"));
            break;
        }
        case ETypeSignatureSignal::UInt8: {
            uint8_t value;
            pos    = TypeSignatureHelper::read_uint8(pos, end, value);
            result = skr::format(u8"{}{}", result, value);
            break;
        }
        case ETypeSignatureSignal::UInt16: {
            uint16_t value;
            pos    = TypeSignatureHelper::read_uint16(pos, end, value);
            result = skr::format(u8"{}{}", result, value);
            break;
        }
        case ETypeSignatureSignal::UInt32: {
            uint32_t value;
            pos    = TypeSignatureHelper::read_uint32(pos, end, value);
            result = skr::format(u8"{}{}", result, value);
            break;
        }
        case ETypeSignatureSignal::UInt64: {
            uint64_t value;
            pos    = TypeSignatureHelper::read_uint64(pos, end, value);
            result = skr::format(u8"{}{}", result, value);
            break;
        }
        case ETypeSignatureSignal::Int8: {
            int8_t value;
            pos    = TypeSignatureHelper::read_int8(pos, end, value);
            result = skr::format(u8"{}{}", result, value);
            break;
        }
        case ETypeSignatureSignal::Int16: {
            int16_t value;
            pos    = TypeSignatureHelper::read_int16(pos, end, value);
            result = skr::format(u8"{}{}", result, value);
            break;
        }
        case ETypeSignatureSignal::Int32: {
            int32_t value;
            pos    = TypeSignatureHelper::read_int32(pos, end, value);
            result = skr::format(u8"{}{}", result, value);
            break;
        }
        case ETypeSignatureSignal::Int64: {
            int64_t value;
            pos    = TypeSignatureHelper::read_int64(pos, end, value);
            result = skr::format(u8"{}{}", result, value);
            break;
        }
        case ETypeSignatureSignal::Float: {
            float value;
            pos    = TypeSignatureHelper::read_float(pos, end, value);
            result = skr::format(u8"{}{}", result, value);
            break;
        }
        case ETypeSignatureSignal::Double: {
            double value;
            pos    = TypeSignatureHelper::read_double(pos, end, value);
            result = skr::format(u8"{}{}", result, value);
            break;
        }
    }
    return pos;
}

String TypeSignatureHelper::signal_to_string(const uint8_t* pos, const uint8_t* end)
{
    auto signature_type = validate_complete_signature(pos, end);
    SKR_ASSERT(signature_type != ETypeSignatureSignal::None && "Invalid type signature");

    String result;
    switch (signature_type)
    {
        case ETypeSignatureSignal::TypeId: {
            // append modifier
            while (true)
            {
                auto signal = peek_signal(pos, end);
                if (is_modifier(signal))
                {
                    pos = _append_modifier(pos, end, result, signal);
                }
                else
                {
                    break;
                }
            }
            if (!result.is_empty())
            {
                result.self_trim_end();
            }

            // append type name
            GUID type_id;
            pos    = read_type_id(pos, end, type_id);
            result = skr::format(u8"{} {}", get_type_from_guid(type_id)->name(), result);

            break;
        }
        case ETypeSignatureSignal::GenericTypeId: {
            // append modifier
            while (true)
            {
                auto signal = peek_signal(pos, end);
                if (is_modifier(signal))
                {
                    pos = _append_modifier(pos, end, result, signal);
                }
                else
                {
                    break;
                }
            }
            if (!result.is_empty())
            {
                result.self_trim_end();
            }

            // TODO. append generic type name

            break;
        }
        case ETypeSignatureSignal::FunctionSignature: {
            // append modifier
            while (true)
            {
                auto signal = peek_signal(pos, end);
                if (is_modifier(signal))
                {
                    pos = _append_modifier(pos, end, result, signal);
                }
                else
                {
                    break;
                }
            }
            if (!result.is_empty())
            {
                result.self_trim_end();
            }

            // append function signature
            uint32_t param_count;
            pos           = read_function_signature(pos, end, param_count);
            auto next_pos = jump_next_type_or_data(pos, end);
            result        = skr::format(u8"{}({})(", signal_to_string(pos, next_pos), result);
            for (uint32_t i = 0; i < param_count; ++i)
            {
                pos      = next_pos;
                next_pos = jump_next_type_or_data(pos, end);
                if (i == 0)
                {
                    result = skr::format(u8"{}{}", result, signal_to_string(pos, next_pos));
                }
                else
                {
                    result = skr::format(u8"{}, {}", result, signal_to_string(pos, next_pos));
                }
            }
            result += u8")";
            break;
        }
    }
    return result;
}
} // namespace skr::rttr