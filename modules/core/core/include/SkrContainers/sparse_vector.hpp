#pragma once
#include "SkrContainersDef/sparse_vector.hpp"

// rttr
#include "SkrRTTR/type_signature.hpp"
namespace skr
{
static constexpr GUID kSparseVectorGenericId = u8"3cc29798-9798-412c-b493-beefe6653356"_guid;
template <typename T>
struct TypeSignatureTraits<::skr::SparseVector<T>> {
    inline static constexpr size_t buffer_size = type_signature_size_v<ETypeSignatureSignal::GenericTypeId> + TypeSignatureTraits<T>::buffer_size;
    inline static uint8_t*         write(uint8_t* pos, uint8_t* end)
    {
        pos = TypeSignatureHelper::write_generic_type_id(pos, end, kSparseVectorGenericId, 1);
        return TypeSignatureTraits<T>::write(pos, end);
    }
};
} // namespace skr