#pragma once
#include "SkrContainersDef/set.hpp"

// rttr
#include "SkrRTTR/type_signature.hpp"
namespace skr
{
inline constexpr GUID kSetGenericId = u8"7218b701-572f-48f3-ab3a-d6c0b31efeb0"_guid;
template <typename T>
struct TypeSignatureTraits<::skr::Set<T>>
{
    inline static constexpr bool is_supported = concepts::WithRTTRTraits<T>;
    inline static constexpr size_t buffer_size = type_signature_size_v<ETypeSignatureSignal::GenericTypeId> + TypeSignatureTraits<T>::buffer_size;
    inline static uint8_t* write(uint8_t* pos, uint8_t* end)
    {
        pos = TypeSignatureHelper::write_generic_type_id(pos, end, kSetGenericId, 1);
        return TypeSignatureTraits<T>::write(pos, end);
    }
};
} // namespace skr

// serialize
#include <SkrCore/serialize/serialize_traits.hpp>
namespace skr
{
template <typename Set>
struct SerializeSkrSetImpl
{
    using DataType = typename Set::SetDataType;

    inline static void read(ArchiveRead& r, Set& v)
    {
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
            DataType value;
            SKR_FAST_CHECK(r.value<DataType>(value), );
            v.add(std::move(value));
        }
    }
    inline static void write(ArchiveWrite& w, const Set& v)
    {

        Archive::ArrayScope arr_scope{ w };
        SKR_FAST_CHECK(arr_scope.is_success(), );

        // write count
        SKR_FAST_CHECK(w.array_size<uint64_t>(v.size()), );

        // write content
        for (const auto& value : v)
        {
            SKR_FAST_CHECK(w.value<DataType>(value), );
        }
    }
};

template <typename T, typename HashTraits, typename Allocator>
struct Serialize<skr::Set<T, HashTraits, Allocator>>
    : public SerializeSkrSetImpl<skr::Set<T, HashTraits, Allocator>>
{
};
template <typename T, uint64_t kCount, typename HashTraits>
struct Serialize<skr::FixedSet<T, kCount, HashTraits>>
    : public SerializeSkrSetImpl<skr::FixedSet<T, kCount, HashTraits>>
{
};
template <typename T, uint64_t kInlineCount, typename HashTraits, typename Allocator>
struct Serialize<skr::InlineSet<T, kInlineCount, HashTraits, Allocator>>
    : public SerializeSkrSetImpl<skr::InlineSet<T, kInlineCount, HashTraits, Allocator>>
{
};
} // namespace skr