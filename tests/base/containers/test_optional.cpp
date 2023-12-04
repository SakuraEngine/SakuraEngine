#include "SkrTestFramework/framework.hpp"

#include "SkrBase/containers/array/array.hpp"
#include "SkrBase/containers/misc/optional.hpp"
#include "skr_test_allocator.hpp"

TEST_CASE("test optional")
{
    using namespace skr;
    using namespace skr::container;
    using TestArray = Array<Optional<uint32_t>, SkrTestAllocator>;
}