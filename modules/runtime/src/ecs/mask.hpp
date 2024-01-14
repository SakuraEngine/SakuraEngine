#pragma once
#include "concurrentbitset.h" // IWYU pragma: export

namespace sugoi
{
    using mask_t = std::atomic<uint32_t>;
}