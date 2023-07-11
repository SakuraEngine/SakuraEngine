#pragma once
#include <chrono>

#include "SkrRT/platform/memory.h"
#include "SkrRT/platform/debug.h"
#include "SkrRT/io/io.h"
#include "SkrRT/misc/make_zeroed.hpp"
#include "SkrRT/misc/types.h"
#include "SkrRT/misc/log.hpp"

#include <SkrRT/platform/filesystem.hpp>
#include <containers/sptr.hpp>
#include <containers/vector.hpp>
#include <containers/string.hpp>
#include <containers/hashmap.hpp>

#include "serde/json/reader.h"
#include "serde/json/writer.h"

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
    #endif
    #include <atlbase.h>
#endif
#include "dxc/dxcapi.h"


#include "tracy/Tracy.hpp"