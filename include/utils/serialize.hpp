
#pragma once
#include "bitsery/deserializer.h"
#include "bitsery/adapter/buffer.h"
#include "bitsery/traits/core/std_defaults.h"
#include "EASTL/vector.h"
#include "EASTL/fixed_vector.h"
#include "gsl/span"
#include "platform/guid.h"

namespace bitsery
{

namespace traits
{

template <typename T>
struct ContainerTraits<gsl::span<T>>
    : public StdContainer<gsl::span<T>, true, true> {
};

template <typename T, typename Allocator>
struct ContainerTraits<eastl::vector<T, Allocator>>
    : public StdContainer<eastl::vector<T, Allocator>, true, true> {
};

template <typename T, typename Allocator>
struct BufferAdapterTraits<eastl::vector<T, Allocator>>
    : public StdContainerForBufferAdapter<eastl::vector<T, Allocator>> {
};

template <typename T, size_t count>
struct ContainerTraits<eastl::fixed_vector<T, count>>
    : public StdContainer<eastl::fixed_vector<T, count>, true, true> {
};

template <typename T, size_t count>
struct BufferAdapterTraits<eastl::fixed_vector<T, count>>
    : public StdContainerForBufferAdapter<eastl::fixed_vector<T, count>> {
};

} // namespace traits

} // namespace bitsery

namespace bitsery
{
template <class S>
void serialize(S& s, skr_guid_t& guid)
{
    s.value4b(guid.Data1);
    s.value2b(guid.Data2);
    s.value2b(guid.Data3);
    for (int i = 0; i < 8; ++i)
        s.value1b(guid.Data4[i]);
}
} // namespace bitsery