#pragma once
#include <type_traits>
#include <atomic>

#if defined(_DEBUG) && !defined(NDEBUG)	// Use !defined(NDEBUG) to check to see if we actually are linking with Debug third party libraries (bDebugBuildsActuallyUseDebugCRT)
	#ifndef TBB_USE_DEBUG
		#define TBB_USE_DEBUG 1
	#endif
#endif

namespace skd
{
	template<typename Traits>
	struct USDWrapperWithRC : public Traits
	{
		uint32_t add_refcount()
		{
			rc++;
			return rc;
		}
		uint32_t release()
		{
			rc--;
			return rc;
		}
		uint32_t use_count() const
		{
			return rc;
		}

	    std::atomic_uint32_t rc = 0;
	};
}