#pragma once
#include "SkrContainersDef/set.hpp"

// rttr
#include "SkrRTTR/type_signature.hpp"
namespace skr
{
inline constexpr GUID kSetGenericId = u8"7218b701-572f-48f3-ab3a-d6c0b31efeb0"_guid;
template <typename T>
struct TypeSignatureTraits<::skr::Set<T>> {
    inline static constexpr size_t buffer_size = type_signature_size_v<ETypeSignatureSignal::GenericTypeId> + TypeSignatureTraits<T>::buffer_size;
    inline static uint8_t*         write(uint8_t* pos, uint8_t* end)
    {
        pos = TypeSignatureHelper::write_generic_type_id(pos, end, kSetGenericId, 1);
        return TypeSignatureTraits<T>::write(pos, end);
    }
};
} // namespace skr