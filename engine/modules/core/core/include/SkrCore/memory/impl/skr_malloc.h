#pragma once
#include "SkrProfile/profile.h"
#include "SkrBase/misc/debug.h"
#include "SkrBase/config.h"
#include <string.h> // memset

//=======================basic alloc=======================
SKR_EXTERN_C SKR_CORE_API void* _sakura_malloc(size_t size, const char* pool_name);
SKR_EXTERN_C SKR_CORE_API void* _sakura_calloc(size_t count, size_t size, const char* pool_name);
SKR_EXTERN_C SKR_CORE_API void* _sakura_malloc_aligned(size_t size, size_t alignment, const char* pool_name);
SKR_EXTERN_C SKR_CORE_API void* _sakura_calloc_aligned(size_t count, size_t size, size_t alignment, const char* pool_name);
SKR_EXTERN_C SKR_CORE_API void* _sakura_new_n(size_t count, size_t size, const char* pool_name);
SKR_EXTERN_C SKR_CORE_API void* _sakura_new_aligned(size_t size, size_t alignment, const char* pool_name);
SKR_EXTERN_C SKR_CORE_API void _sakura_free(void* p, const char* pool_name) SKR_NOEXCEPT;
SKR_EXTERN_C SKR_CORE_API void _sakura_free_aligned(void* p, size_t alignment, const char* pool_name);
SKR_EXTERN_C SKR_CORE_API void* _sakura_realloc(void* p, size_t newsize, const char* pool_name);
SKR_EXTERN_C SKR_CORE_API void* _sakura_realloc_aligned(void* p, size_t newsize, size_t alignment, const char* pool_name);

SKR_EXTERN_C SKR_CORE_API void* traced_os_malloc(size_t size, const char* pool_name);
SKR_EXTERN_C SKR_CORE_API void* traced_os_calloc(size_t count, size_t size, const char* pool_name);
SKR_EXTERN_C SKR_CORE_API void* traced_os_malloc_aligned(size_t size, size_t alignment, const char* pool_name);
SKR_EXTERN_C SKR_CORE_API void* traced_os_calloc_aligned(size_t count, size_t size, size_t alignment, const char* pool_name);
SKR_EXTERN_C SKR_CORE_API void traced_os_free(void* p, const char* pool_name) SKR_NOEXCEPT;
SKR_EXTERN_C SKR_CORE_API void traced_os_free_aligned(void* p, size_t alignment, const char* pool_name);
SKR_EXTERN_C SKR_CORE_API void* traced_os_realloc(void* p, size_t newsize, const char* pool_name);
SKR_EXTERN_C SKR_CORE_API void* traced_os_realloc_aligned(void* p, size_t newsize, size_t alignment, const char* pool_name);

SKR_EXTERN_C SKR_CORE_API void* containers_malloc_aligned(size_t size, size_t alignment);
SKR_EXTERN_C SKR_CORE_API void containers_free_aligned(void* p, size_t alignment);

//=======================alloc with trace=======================
#if defined(SKR_PROFILE_ENABLE) && defined(TRACY_TRACE_ALLOCATION)
    #define SKR_ALLOC_TRACY_MARKER_COLOR 0xff0000
    #define SKR_DEALLOC_TRACY_MARKER_COLOR 0x0000ff

SKR_FORCEINLINE void* SkrMallocWithCZone(size_t size, const char* line, const char* pool_name)
{
    SkrCZoneC(z, SKR_ALLOC_TRACY_MARKER_COLOR, 1);
    SkrCZoneText(z, line, strlen(line));
    SkrCZoneName(z, line, strlen(line));
    void* ptr = _sakura_malloc(size, pool_name);
    SkrCZoneEnd(z);
    return ptr;
}
SKR_FORCEINLINE void* SkrCallocWithCZone(size_t count, size_t size, const char* line, const char* pool_name)
{
    SkrCZoneC(z, SKR_ALLOC_TRACY_MARKER_COLOR, 1);
    SkrCZoneText(z, line, strlen(line));
    SkrCZoneName(z, line, strlen(line));
    void* ptr = _sakura_calloc(count, size, pool_name);
    SkrCZoneEnd(z);
    return ptr;
}
SKR_FORCEINLINE void* SkrMallocAlignedWithCZone(size_t size, size_t alignment, const char* line, const char* pool_name)
{
    SkrCZoneC(z, SKR_ALLOC_TRACY_MARKER_COLOR, 1);
    SkrCZoneText(z, line, strlen(line));
    SkrCZoneName(z, line, strlen(line));
    void* ptr = _sakura_malloc_aligned(size, alignment, pool_name);
    SkrCZoneEnd(z);
    return ptr;
}
SKR_FORCEINLINE void* SkrCallocAlignedWithCZone(size_t count, size_t size, size_t alignment, const char* line, const char* pool_name)
{
    SkrCZoneC(z, SKR_ALLOC_TRACY_MARKER_COLOR, 1);
    SkrCZoneText(z, line, strlen(line));
    SkrCZoneName(z, line, strlen(line));
    void* ptr = _sakura_calloc_aligned(count, size, alignment, pool_name);
    SkrCZoneEnd(z);
    return ptr;
}
SKR_FORCEINLINE void* SkrNewNWithCZone(size_t count, size_t size, const char* line, const char* pool_name)
{
    SkrCZoneC(z, SKR_ALLOC_TRACY_MARKER_COLOR, 1);
    SkrCZoneText(z, line, strlen(line));
    SkrCZoneName(z, line, strlen(line));
    void* ptr = _sakura_new_n(count, size, pool_name);
    SkrCZoneEnd(z);
    return ptr;
}
SKR_FORCEINLINE void* SkrNewAlignedWithCZone(size_t size, size_t alignment, const char* line, const char* pool_name)
{
    SkrCZoneC(z, SKR_ALLOC_TRACY_MARKER_COLOR, 1);
    SkrCZoneText(z, line, strlen(line));
    SkrCZoneName(z, line, strlen(line));
    void* ptr = _sakura_new_aligned(size, alignment, pool_name);
    SkrCZoneEnd(z);
    return ptr;
}
SKR_FORCEINLINE void SkrFreeWithCZone(void* p, const char* line, const char* pool_name)
{
    SkrCZoneC(z, SKR_DEALLOC_TRACY_MARKER_COLOR, 1);
    SkrCZoneText(z, line, strlen(line));
    SkrCZoneName(z, line, strlen(line));
    _sakura_free(p, pool_name);
    SkrCZoneEnd(z);
}
SKR_FORCEINLINE void SkrFreeAlignedWithCZone(void* p, size_t alignment, const char* line, const char* pool_name)
{
    SkrCZoneC(z, SKR_DEALLOC_TRACY_MARKER_COLOR, 1);
    SkrCZoneText(z, line, strlen(line));
    SkrCZoneName(z, line, strlen(line));
    _sakura_free_aligned(p, alignment, pool_name);
    SkrCZoneEnd(z);
}
SKR_FORCEINLINE void* SkrReallocWithCZone(void* p, size_t newsize, const char* line, const char* pool_name)
{
    SkrCZoneC(z, SKR_ALLOC_TRACY_MARKER_COLOR, 1);
    SkrCZoneText(z, line, strlen(line));
    SkrCZoneName(z, line, strlen(line));
    void* ptr = _sakura_realloc(p, newsize, pool_name);
    SkrCZoneEnd(z);
    return ptr;
}
SKR_FORCEINLINE void* SkrReallocAlignedWithCZone(void* p, size_t newsize, size_t alignment, const char* line, const char* pool_name)
{
    SkrCZoneC(z, SKR_ALLOC_TRACY_MARKER_COLOR, 1);
    SkrCZoneText(z, line, strlen(line));
    SkrCZoneName(z, line, strlen(line));
    void* ptr = _sakura_realloc_aligned(p, newsize, alignment, pool_name);
    SkrCZoneEnd(z);
    return ptr;
}
#endif

//=======================alloc defines=======================
#if defined(SKR_PROFILE_ENABLE) && defined(TRACY_TRACE_ALLOCATION)
    #define SKR_ALLOC_STRINGFY_IMPL(X) #X
    #define SKR_ALLOC_STRINGFY(X) SKR_ALLOC_STRINGFY_IMPL(X)
    #define SKR_ALLOC_CAT_IMPL(X, Y) X Y
    #define SKR_ALLOC_CAT(X, Y) SKR_ALLOC_CAT_IMPL(X, Y)

    #define sakura_malloc(size) SkrMallocWithCZone((size), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__), SKR_ALLOC_STRINGFY(__LINE__)), NULL)
    #define sakura_calloc(count, size) SkrCallocWithCZone((count), (size), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__), SKR_ALLOC_STRINGFY(__LINE__)), NULL)
    #define sakura_malloc_aligned(size, alignment) SkrMallocAlignedWithCZone((size), (alignment), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__), SKR_ALLOC_STRINGFY(__LINE__)), NULL)
    #define sakura_calloc_aligned(count, size, alignment) SkrCallocAlignedWithCZone((count), (size), (alignment), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__), SKR_ALLOC_STRINGFY(__LINE__)), NULL)
    #define sakura_new_n(count, size) SkrNewNWithCZone((count), (size), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__), SKR_ALLOC_STRINGFY(__LINE__)), NULL)
    #define sakura_new_aligned(size, alignment) SkrNewAlignedWithCZone((size), (alignment), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__), SKR_ALLOC_STRINGFY(__LINE__)), NULL)
    #define sakura_realloc(p, newsize) SkrReallocWithCZone((p), (newsize), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__), SKR_ALLOC_STRINGFY(__LINE__)), NULL)
    #define sakura_realloc_aligned(p, newsize, align) SkrReallocAlignedWithCZone((p), (newsize), (align), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__), SKR_ALLOC_STRINGFY(__LINE__)), NULL)
    #define sakura_free(p) SkrFreeWithCZone((p), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__), SKR_ALLOC_STRINGFY(__LINE__)), NULL)
    #define sakura_free_aligned(p, alignment) SkrFreeAlignedWithCZone((p), (alignment), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__), SKR_ALLOC_STRINGFY(__LINE__)), NULL)

    #define sakura_mallocN(size, ...) SkrMallocWithCZone((size), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__), SKR_ALLOC_STRINGFY(__LINE__)), __VA_ARGS__)
    #define sakura_callocN(count, size, ...) SkrCallocWithCZone((count), (size), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__), SKR_ALLOC_STRINGFY(__LINE__)), __VA_ARGS__)
    #define sakura_malloc_alignedN(size, alignment, ...) SkrMallocAlignedWithCZone((size), (alignment), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__), SKR_ALLOC_STRINGFY(__LINE__)), __VA_ARGS__)
    #define sakura_calloc_alignedN(count, size, alignment, ...) SkrCallocAlignedWithCZone((count), (size), (alignment), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__), SKR_ALLOC_STRINGFY(__LINE__)), __VA_ARGS__)
    #define sakura_new_nN(count, size, ...) SkrNewNWithCZone((count), (size), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__), SKR_ALLOC_STRINGFY(__LINE__)), __VA_ARGS__)
    #define sakura_new_alignedN(size, alignment, ...) SkrNewAlignedWithCZone((size), (alignment), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__), SKR_ALLOC_STRINGFY(__LINE__)), __VA_ARGS__)
    #define sakura_realloc_alignedN(p, newsize, align, ...) SkrReallocAlignedWithCZone((p), (newsize), (align), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__), SKR_ALLOC_STRINGFY(__LINE__)), __VA_ARGS__)
    #define sakura_reallocN(p, newsize, ...) SkrReallocWithCZone((p), (newsize), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__), SKR_ALLOC_STRINGFY(__LINE__)), __VA_ARGS__)
    #define sakura_freeN(p, ...) SkrFreeWithCZone((p), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__), SKR_ALLOC_STRINGFY(__LINE__)), __VA_ARGS__)
    #define sakura_free_alignedN(p, alignment, ...) SkrFreeAlignedWithCZone((p), (alignment), SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__), SKR_ALLOC_STRINGFY(__LINE__)), __VA_ARGS__)
#else
    #define sakura_malloc(size) _sakura_malloc((size), NULL)
    #define sakura_calloc(count, size) _sakura_calloc((count), (size), NULL)
    #define sakura_malloc_aligned(size, alignment) _sakura_malloc_aligned((size), (alignment), NULL)
    #define sakura_calloc_aligned(count, size, alignment) _sakura_calloc_aligned((count), (size), (alignment), NULL)
    #define sakura_new_n(count, size) _sakura_new_n((count), (size), NULL)
    #define sakura_new_aligned(size, alignment) _sakura_new_aligned((size), (alignment), NULL)
    #define sakura_realloc(p, newsize) _sakura_realloc((p), (newsize), NULL)
    #define sakura_realloc_aligned(p, newsize, align) _sakura_realloc_aligned((p), (newsize), (align), NULL)
    #define sakura_free(p) _sakura_free((p), NULL)
    #define sakura_free_aligned(p, alignment) _sakura_free_aligned((p), (alignment), NULL)

    #define sakura_mallocN(size, ...) _sakura_malloc((size), __VA_ARGS__)
    #define sakura_callocN(count, size, ...) _sakura_calloc((count), (size), __VA_ARGS__)
    #define sakura_malloc_alignedN(size, alignment, ...) _sakura_malloc_aligned((size), (alignment), __VA_ARGS__)
    #define sakura_calloc_alignedN(count, size, alignment, ...) _sakura_calloc_aligned((count), (size), (alignment), __VA_ARGS__)
    #define sakura_new_nN(count, size, ...) _sakura_new_n((count), (size), __VA_ARGS__)
    #define sakura_new_alignedN(size, alignment, ...) _sakura_new_aligned((size), (alignment), __VA_ARGS__)
    #define sakura_reallocN(p, newsize, ...) _sakura_realloc((p), (newsize), __VA_ARGS__)
    #define sakura_realloc_alignedN(p, newsize, align, ...) _sakura_realloc_aligned((p), (newsize), (align), __VA_ARGS__)
    #define sakura_freeN(p, ...) _sakura_free((p), __VA_ARGS__)
    #define sakura_free_alignedN(p, alignment, ...) _sakura_free_aligned((p), (alignment), __VA_ARGS__)
#endif