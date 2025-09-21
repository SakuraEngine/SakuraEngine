#pragma once
#include "SkrContainersDef/vector.hpp" // IWYU pragma: export

// rttr
#include "SkrRTTR/type_signature.hpp"
namespace skr
{
static constexpr GUID kVectorGenericId = u8"3e982879-a893-4975-aca6-ed8627f33f91"_guid;
template <typename T>
struct TypeSignatureTraits<::skr::Vector<T>>
{
    inline static constexpr bool is_supported = concepts::WithRTTRTraits<T>;
    inline static constexpr size_t buffer_size = type_signature_size_v<ETypeSignatureSignal::GenericTypeId> + TypeSignatureTraits<T>::buffer_size;
    inline static uint8_t* write(uint8_t* pos, uint8_t* end)
    {
        pos = TypeSignatureHelper::write_generic_type_id(pos, end, kVectorGenericId, 1);
        return TypeSignatureTraits<T>::write(pos, end);
    }
};
} // namespace skr

// serialize
#include <SkrCore/serialize/serialize_traits.hpp>
namespace skr
{
template <typename Vector>
struct SerializeSkrVectorImpl
{
    using DataType = typename Vector::DataType;

    inline static void read(ArchiveRead& r, Vector& v)
    {
        SkrZoneScopedN("Serialize<Vector>::read");

        Archive::ArrayScope arr_scope{ r };
        SKR_FAST_CHECK(arr_scope.is_success(), );

        // reserve
        v.clear();
        uint64_t arr_size;
        SKR_FAST_CHECK(r.array_size<uint64_t>(arr_size), );
        v.reserve(arr_size);

        // read content
        for (uint64_t i = 0; i < arr_size; ++i)
        {
            DataType data;
            SKR_FAST_CHECK(r.value<DataType>(data), );
            v.add(std::move(data));
        }
    }
    inline static void write(ArchiveWrite& w, const Vector& v)
    {
        SkrZoneScopedN("Serialize<Vector>::write");

        Archive::ArrayScope arr_scope{ w };
        SKR_FAST_CHECK(arr_scope.is_success(), );

        // write count
        SKR_FAST_CHECK(w.array_size<uint64_t>((uint64_t)v.size()), );

        // write content
        for (const auto& data : v)
        {
            SKR_FAST_CHECK(w.value<DataType>(data), );
        }
    }
};

template <typename T, typename Allocator>
struct Serialize<skr::Vector<T, Allocator>>
    : public SerializeSkrVectorImpl<skr::Vector<T, Allocator>>
{
};
template <typename T, uint64_t kCount>
struct Serialize<skr::FixedVector<T, kCount>>
    : public SerializeSkrVectorImpl<skr::FixedVector<T, kCount>>
{
};
template <typename T, uint64_t kCount, typename Allocator>
struct Serialize<skr::InlineVector<T, kCount, Allocator>>
    : public SerializeSkrVectorImpl<skr::InlineVector<T, kCount, Allocator>>
{
};
} // namespace skr