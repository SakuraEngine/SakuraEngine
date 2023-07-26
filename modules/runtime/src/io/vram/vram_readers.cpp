#include "../../pch.hpp"
#include "SkrRT/platform/debug.h"
#include "vram_readers.hpp"
#include "SkrRT/async/thread_job.hpp"

// VFS READER IMPLEMENTATION

namespace skr {
namespace io {

using VFSReaderFutureLauncher = skr::FutureLauncher<bool>;

} // namespace io
} // namespace skr