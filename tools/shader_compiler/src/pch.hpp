#pragma once
#include <chrono>

#include "platform/memory.h"
#include "platform/debug.h"
#include "io/io.h"
#include "misc/make_zeroed.hpp"
#include "misc/types.h"
#include "misc/log.hpp"

#include <platform/filesystem.hpp>
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
    #include "dxc/dxcapi.h"

#endif

#include "tracy/Tracy.hpp"