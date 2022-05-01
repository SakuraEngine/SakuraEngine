
#include "bitsery/deserializer.h"
#include "bitsery/adapter/buffer.h"
#include "bitsery/traits/core/std_defaults.h"
#include "EASTL/vector.h"
#include "gsl/span"

namespace skr
{
namespace utils
{

} // namespace utils
} // namespace skr

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

} // namespace traits

} // namespace bitsery