#define SKR_MEMORY_IMPL
#include "SkrMemory/smart_ptr/cdrc/internal/utils.h"

namespace cdrc {
namespace utils {

std::vector<std::atomic<bool>> ThreadID::in_use = std::vector<std::atomic<bool>>(utils::num_threads()); // initialize to false

ThreadID ThreadID::GetThreadID()
{
    static thread_local ThreadID threadID{};
    return threadID;
}

}
}