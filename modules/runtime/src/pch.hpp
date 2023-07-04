#pragma once

#ifdef _WIN32
#include <intrin.h>
#include <new.h>
#include "platform/windows/winheaders.h"
#endif

#include <chrono>
#include "platform/debug.h"
#include "platform/guid.hpp"
#include <platform/filesystem.hpp>

#include "misc/log.h"
#include "misc/log.hpp"

#include <containers/concurrent_queue.h>
#include <containers/sptr.hpp>
#include <containers/string.hpp>
#include <containers/vector.hpp>
#include <containers/hashmap.hpp>

#include "tracy/Tracy.hpp"