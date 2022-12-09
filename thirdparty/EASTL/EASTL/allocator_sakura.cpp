/* clang-format off */
#include "internal/config.h"
#include "allocator_sakura.h"
#include <stdlib.h>

#include "platform/memory.h"

#define core_malloc sakura_malloc
#define core_memalign sakura_malloc_aligned
#define core_free sakura_free
#define core_free_aligned sakura_free_aligned

	namespace eastl
	{
		void* allocator_sakura::allocate(size_t n, int /*flags*/)
		{ 
			ZoneScopedNS("EASTL::allocate", 16);
			void* p = core_memalign(n, 1);
			return p;
		}

		void* allocator_sakura::allocate(size_t n, size_t alignment, size_t alignmentOffset, int /*flags*/)
		{
		    if ((alignmentOffset % alignment) == 0) // We check for (offset % alignmnent == 0) instead of (offset == 0) because any block which is
													// aligned on e.g. 64 also is aligned at an offset of 64 by definition.
			{
				ZoneScopedNS("EASTL::allocate(aligned)", 16);
			    void* p = core_memalign(n, alignment);
				return p;
			}

		    return NULL;
		}

		void allocator_sakura::deallocate(void* p, size_t /*n*/)
		{ 
			ZoneScopedNS("EASTL::deallocate", 8);

			core_free_aligned(p, 1);
		}

		/// gDefaultAllocator
		/// Default global allocator_sakura instance. 
		EASTL_API allocator_sakura  gDefaultAllocatorsakura;
		EASTL_API allocator_sakura* gpDefaultAllocatorsakura = &gDefaultAllocatorsakura;

		EASTL_API allocator_sakura* GetDefaultAllocatorSakura()
		{
			return gpDefaultAllocatorsakura;
		}

		EASTL_API allocator_sakura* SetDefaultAllocatorSakura(allocator_sakura* pAllocator)
		{
			allocator_sakura* const pPrevAllocator = gpDefaultAllocatorsakura;
			gpDefaultAllocatorsakura = pAllocator;
			return pPrevAllocator;
		}

	} // namespace eastl


/* clang-format on */