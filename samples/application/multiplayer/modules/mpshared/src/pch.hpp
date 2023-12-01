#pragma once
#include <chrono> // IWYU pragma: export
#include "steam/steamnetworkingtypes.h" // IWYU pragma: export

#include "SkrMemory/memory.h" // IWYU pragma: export
#include "SkrBase/misc/debug.h"  // IWYU pragma: export
#include <SkrRT/platform/filesystem.hpp> // IWYU pragma: export
#include "SkrRT/misc/make_zeroed.hpp" // IWYU pragma: export
#include "SkrRT/misc/types.h" // IWYU pragma: export
#include "SkrRT/misc/bits.hpp" // IWYU pragma: export
#include "SkrRT/misc/log.hpp" // IWYU pragma: export
#include "SkrRT/misc/parallel_for.hpp" // IWYU pragma: export
#include "SkrRT/io/ram_io.hpp" // IWYU pragma: export
#include "SkrRT/resource/resource_factory.h" // IWYU pragma: export
#include "SkrRT/resource/resource_handle.h" // IWYU pragma: export

#include "SkrBase/math/vector.h" // IWYU pragma: export
#include "SkrBase/math/quat.h" // IWYU pragma: export
#include "SkrBase/math/rtm/quatf.h" // IWYU pragma: export
#include "SkrBase/math/rtm/rtmx.h" // IWYU pragma: export

#include "SkrRT/containers/bitset.h" // IWYU pragma: export
#include <SkrRT/containers_new/sptr.hpp> // IWYU pragma: export
#include <SkrRT/containers_new/vector.hpp> // IWYU pragma: export
#include <SkrRT/containers_new/string.hpp> // IWYU pragma: export
#include <SkrRT/containers_new/hashmap.hpp> // IWYU pragma: export
#include <SkrRT/containers_new/stl_vector.hpp> // IWYU pragma: export
#include <SkrRT/containers_new/stl_string.hpp> // IWYU pragma: export

#include "SkrRT/ecs/dual.h" // IWYU pragma: export
#include "SkrRT/ecs/set.hpp" // IWYU pragma: export
#include "SkrRT/ecs/type_builder.hpp" // IWYU pragma: export

#include "SkrRT/serde/json/reader.h" // IWYU pragma: export
#include "SkrRT/serde/json/writer.h" // IWYU pragma: export

#include "SkrProfile/profile.h" // IWYU pragma: export

#ifdef VALVE_POSIX
	#include <unistd.h> // IWYU pragma: export
	#include <sys/socket.h> // IWYU pragma: export
	#include <sys/types.h> // IWYU pragma: export
	#include <netinet/in.h> // IWYU pragma: export
	#include <netdb.h> // IWYU pragma: export
	#include <sys/ioctl.h> // IWYU pragma: export
#endif
#ifdef _WIN32
	#include <winsock2.h> // IWYU pragma: export
	#include <ws2tcpip.h> // IWYU pragma: export
#endif