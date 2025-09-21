#pragma once
#include "SkrContainersDef/ring_buffer.hpp"

// serialize
#include <SkrCore/serialize/serialize_traits.hpp>
namespace skr
{
template <typename RingBuffer>
struct SerializeSkrRingBufferImpl
{
    using DataType = typename RingBuffer::DataType;

    inline static void read(ArchiveRead& r, RingBuffer& v)
    {
        if (r.is_structured())
        {
            Archive::ArrayScope arr_scope{ r };
            SKR_FAST_CHECK(arr_scope.is_success(), );

            // reserve ring buffer
            v.clear();
            uint64_t arr_size;
            SKR_FAST_CHECK(r.array_size_structured(arr_size), );
            v.reserve(arr_size);

            // read content
            for (uint64_t i = 0; i < arr_size; ++i)
            {
                DataType value;
                SKR_FAST_CHECK(r.value<DataType>(value), );
                v.push_back(std::move(value));
            }
        }
        else
        {
            // read size
            uint64_t size = 0;
            SKR_FAST_CHECK(r.value<uint64_t>(size), );

            // reserve ring buffer
            v.clear();
            v.reserve(size);

            // read content in order
            for (uint64_t i = 0; i < size; ++i)
            {
                DataType value;
                SKR_FAST_CHECK(r.value<DataType>(value), );
                v.push_back(std::move(value));
            }
        }
    }
    inline static void write(ArchiveWrite& w, const RingBuffer& v)
    {
        if (w.is_structured())
        {
            Archive::ArrayScope arr_scope(w);
            SKR_FAST_CHECK(arr_scope.is_success(), );

            for (const auto& value : v)
            {
                SKR_FAST_CHECK(w.value<DataType>(value), );
            }
        }
        else
        {
            // write size
            SKR_FAST_CHECK(w.value<uint64_t>(uint64_t(v.size())), );

            // write content in order
            for (const auto& value : v)
            {
                SKR_FAST_CHECK(w.value<DataType>(value), );
            }
        }
    }
};
} // namespace skr