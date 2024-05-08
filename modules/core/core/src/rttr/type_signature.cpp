#include "SkrRTTR/type_signature.hpp"
#include "SkrRTTR/type/type.hpp"
#include "SkrRTTR/type_registry.hpp"

namespace skr::rttr
{
String TypeSignatureHelper::signal_to_string(const uint8_t* pos, const uint8_t* end)
{
    String result;
    while (pos < end)
    {
        auto signal = peek_signal(pos, end);
        switch (signal)
        {
            case ETypeSignatureSignal::None: {
                pos = read_none(pos, end);
                break;
            }
            case ETypeSignatureSignal::Separator: {
                pos = read_separator(pos, end);
                break;
            }
            case ETypeSignatureSignal::TypeId: {
                GUID type_id;
                pos    = read_type_id(pos, end, type_id);
                result = skr::format(u8"{} {}", get_type_from_guid(type_id)->name(), result);
                break;
            }
            case ETypeSignatureSignal::GenericTypeId: {
                GUID type_id;
                pos = read_generic_type_id(pos, end, type_id);
                // TODO. generic id
                SKR_UNIMPLEMENTED_FUNCTION()
                break;
            }
            case ETypeSignatureSignal::FunctionSignature: {
                // TODO. function signature
                SKR_UNIMPLEMENTED_FUNCTION()
            }
            case ETypeSignatureSignal::Const: {
                pos    = read_const(pos, end);
                result = skr::format(u8"const {}", result);
                break;
            }
            case ETypeSignatureSignal::Pointer: {
                pos    = read_pointer(pos, end);
                result = skr::format(u8"* {}", result);
                break;
            }
            case ETypeSignatureSignal::Ref: {
                pos    = read_ref(pos, end);
                result = skr::format(u8"& {}", result);
                break;
            }
            case ETypeSignatureSignal::RValueRef: {
                pos    = read_rvalue_ref(pos, end);
                result = skr::format(u8"&& {}", result);
                break;
            }
            case ETypeSignatureSignal::ArrayDim: {
                uint32_t dim;
                pos    = read_array_dim(pos, end, dim);
                result = skr::format(u8"[{}] {}", dim, result);
                break;
            }
            default:
                SKR_UNREACHABLE_CODE();
        }
    }
    return result;
}
} // namespace skr::rttr