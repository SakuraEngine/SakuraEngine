#pragma once
#include "SkrContainersDef/sparse_vector.hpp"

// rttr
#include "SkrRTTR/type_signature.hpp"
namespace skr
{
static constexpr GUID kSparseVectorGenericId = u8"3cc29798-9798-412c-b493-beefe6653356"_guid;
template <typename T>
struct TypeSignatureTraits<::skr::SparseVector<T>>
{
    inline static constexpr bool is_supported = concepts::WithRTTRTraits<T>;
    inline static constexpr size_t buffer_size = type_signature_size_v<ETypeSignatureSignal::GenericTypeId> + TypeSignatureTraits<T>::buffer_size;
    inline static uint8_t* write(uint8_t* pos, uint8_t* end)
    {
        pos = TypeSignatureHelper::write_generic_type_id(pos, end, kSparseVectorGenericId, 1);
        return TypeSignatureTraits<T>::write(pos, end);
    }
};
} // namespace skr

// serialize
#include <SkrCore/serialize/serialize_traits.hpp>
namespace skr
{
template <typename SparseVector>
struct SerializeSkrSparseVectorImpl
{
    inline static void read(ArchiveRead& r, SparseVector& v)
    {
        using DataType = typename SparseVector::value_type;
        if (r.is_structured())
        {
            Archive::ArrayScope arr_scope{ r };
            SKR_FAST_CHECK(arr_scope.is_success(), );

            // reserve
            v.clear();
            uint64_t arr_size;
            SKR_FAST_CHECK(r.array_size_structured(arr_size), );
            v.reserve(arr_size);

            // read content
            for (uint64_t i = 0; i < arr_size; ++i)
            {
                DataType value;
                SKR_FAST_CHECK(r.value<DataType>(value), );
                v.add(std::move(value));
            }
        }
        else
        {
            // read count
            uint64_t count = 0;
            SKR_FAST_CHECK(r.value<uint64_t>(count), );

            // reserve
            v.clear();
            v.reserve(count);

            // read content
            for (uint64_t i = 0; i < count; ++i)
            {
                DataType value;
                SKR_FAST_CHECK(r.value<DataType>(value), );
                v.add(std::move(value));
            }
        }
    }
    inline static void write(ArchiveWrite& w, const SparseVector& v)
    {
        using DataType = typename SparseVector::value_type;
        if (w.is_structured())
        {
            Archive::ArrayScope arr_scope{ w };
            SKR_FAST_CHECK(arr_scope.is_success(), );

            // write content
            for (const auto& slot : v)
            {
                SKR_FAST_CHECK(w.value<DataType>(slot), );
            }
        }
        else
        {
            // write count
            SKR_FAST_CHECK(w.value<uint64_t>(static_cast<uint64_t>(v.size())), );

            // write content
            for (const auto& slot : v)
            {
                SKR_FAST_CHECK(w.value<DataType>(slot), );
            }
        }
    }
};
} // namespace skr