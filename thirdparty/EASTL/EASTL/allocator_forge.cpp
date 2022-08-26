/* clang-format off */
#include "internal/config.h"
#include "allocator_forge.h"
#include <stdlib.h>

#include "tracy/TracyC.h"

#ifdef _CRTDBG_MAP_ALLOC
    #include <crtdbg.h>
	#define core_malloc(n) _aligned_malloc((n), 1)
	#define core_memalign _aligned_malloc
	#define core_free _aligned_free
#else
	#if !defined(__EMSCRIPTEN__) && !defined(__APPLE__)
	extern "C"
	{
		extern void* mi_malloc(size_t size);
		extern void* mi_malloc_aligned(size_t size, size_t alignment);
		extern void mi_free(void* p);
	}
	#define core_malloc ::mi_malloc
	#define core_memalign ::mi_malloc_aligned
	#define core_free ::mi_free
	#else
	#define core_malloc malloc
	#ifdef _WINDOWS
	#define core_memalign _aligned_malloc
	#else
	#define core_memalign(size, alignment) aligned_alloc((alignment), (size))
	#endif
	#define core_free free
	#endif
#endif

#if EASTL_ALLOCATOR_FORGE
	namespace eastl
	{
		void* allocator_forge::allocate(size_t n, int /*flags*/)
		{ 
			void* p = core_malloc(n);
			TracyCAllocN(p, n, "EASTL");
			return p;
		}

		void* allocator_forge::allocate(size_t n, size_t alignment, size_t alignmentOffset, int /*flags*/)
		{
		    if ((alignmentOffset % alignment) == 0) // We check for (offset % alignmnent == 0) instead of (offset == 0) because any block which is
													// aligned on e.g. 64 also is aligned at an offset of 64 by definition.
			{
			    void* p = core_memalign(n, alignment);
				TracyCAllocN(p, n, "EASTL");
				return p;
			}

		    return NULL;
		}

		void allocator_forge::deallocate(void* p, size_t /*n*/)
		{ 
			TracyCFree(p);
			core_free(p);
		}

		/// gDefaultAllocator
		/// Default global allocator_forge instance. 
		EASTL_API allocator_forge  gDefaultAllocatorForge;
		EASTL_API allocator_forge* gpDefaultAllocatorForge = &gDefaultAllocatorForge;

		EASTL_API allocator_forge* GetDefaultAllocatorForge()
		{
			return gpDefaultAllocatorForge;
		}

		EASTL_API allocator_forge* SetDefaultAllocatorForge(allocator_forge* pAllocator)
		{
			allocator_forge* const pPrevAllocator = gpDefaultAllocatorForge;
			gpDefaultAllocatorForge = pAllocator;
			return pPrevAllocator;
		}

	} // namespace eastl


#endif // EASTL_USER_DEFINED_ALLOCATOR

/* clang-format on */