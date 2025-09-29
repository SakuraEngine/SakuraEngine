#pragma once
#include "./skr_malloc.h"
#include <SkrBase/misc/integer_tools.hpp>
#include <cstddef> // std::size_t
#include <cstdint> // PTRDIFF_MAX
#include <new>     // 'operator new' function for non-allocating placement new expression

#if defined(SKR_PROFILE_ENABLE) && defined(TRACY_TRACE_ALLOCATION)
    #include "SkrBase/misc/demangle.hpp"
#endif

//=======================new core=======================
#if defined(SKR_PROFILE_ENABLE) && defined(TRACY_TRACE_ALLOCATION)
struct SkrNewCore
{
    const std::string_view sourcelocation;
    const char* poolname;
    SkrNewCore(std::string_view sourcelocation) noexcept
        : sourcelocation(sourcelocation)
        , poolname(NULL)
    {
    }
    SkrNewCore(std::string_view sourcelocation, const char* poolname) noexcept
        : sourcelocation(sourcelocation)
        , poolname(poolname)
    {
    }

    template <class T>
    [[nodiscard]] SKR_FORCEINLINE T* Alloc()
    {
        const std::string_view name = skr::demangle<T>();
        SkrCMessage(name.data(), name.size());
        void* p = SkrNewAlignedWithCZone(sizeof(T), alignof(T), sourcelocation.data(), poolname);
        SKR_ASSERT(p != nullptr);
        return static_cast<T*>(p);
    };

    template <class T>
    [[nodiscard]] SKR_FORCEINLINE T* AllocSized(size_t size)
    {
        const std::string_view name = skr::demangle<T>();
        SkrCMessage(name.data(), name.size());
        SKR_ASSERT(size >= sizeof(T));
        void* p = SkrNewAlignedWithCZone(size, alignof(T), sourcelocation.data(), poolname);
        SKR_ASSERT(p != nullptr);
        return static_cast<T*>(p);
    };

    template <class T>
    SKR_FORCEINLINE void Free(T* p)
    {
        if (p != nullptr)
        {
            const std::string_view name = skr::demangle<T>();
            SkrCMessage(name.data(), name.size());
            SkrFreeAlignedWithCZone((void*)p, alignof(T), sourcelocation.data(), poolname);
        }
    }
};
    #define SKR_MAKE_NEW_CORE \
        SkrNewCore { SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__), SKR_ALLOC_STRINGFY(__LINE__)) }
    #define SKR_MAKE_NEW_CORE_N(__N) \
        SkrNewCore { SKR_ALLOC_CAT(SKR_ALLOC_STRINGFY(__FILE__), SKR_ALLOC_STRINGFY(__LINE__)), __N }
#else
struct SkrNewCore
{
    template <class T>
    [[nodiscard]] SKR_FORCEINLINE T* Alloc()
    {
        void* p = sakura_new_aligned(sizeof(T), alignof(T));
        SKR_ASSERT(p != nullptr);
        return static_cast<T*>(p);
    };

    template <class T>
    [[nodiscard]] SKR_FORCEINLINE T* AllocSized(size_t size)
    {
        SKR_ASSERT(size >= sizeof(T));
        void* p = sakura_new_aligned(size, alignof(T));
        SKR_ASSERT(p != nullptr);
        return static_cast<T*>(p);
    };

    template <class T>
    SKR_FORCEINLINE void Free(T* p)
    {
        if (p != nullptr)
        {
            sakura_free_aligned((void*)p, alignof(T));
        }
    }
};
    #define SKR_MAKE_NEW_CORE \
        SkrNewCore {}
    #define SKR_MAKE_NEW_CORE_N(__N) \
        SkrNewCore {}
#endif

//=======================new/delete flag=======================
enum SkrNewFlag
{
    SkrNewFlag_None = 0,
};
enum SkrDeleteFlag
{
    SkrDeleteFlag_None = 0,
    SkrDeleteFlag_No_Dtor = 1 << 0,
};

//=======================delete traits=======================
template <typename T>
struct SkrDeleteTraits
{
    SKR_FORCEINLINE static void* get_free_ptr(T* p)
    {
        return reinterpret_cast<void*>(p);
    }
};

//=======================new traits=======================
#ifdef _CRTDBG_MAP_ALLOC
    #define SKR_DEBUG_NEW_SOURCE_LINE (_NORMAL_BLOCK, __FILE__, __LINE__)
#else
    #define SKR_DEBUG_NEW_SOURCE_LINE
#endif
template <typename T>
struct SkrNewImpl
{
    SkrNewCore core;
    SKR_FORCEINLINE SkrNewImpl(SkrNewCore core)
        : core(core)
    {
    }

    template <typename... Args>
    [[nodiscard]] SKR_FORCEINLINE T* New(Args&&... args)
    {
        T* p = core.Alloc<T>();
        return new (p) SKR_DEBUG_NEW_SOURCE_LINE T{ std::forward<Args>(args)... };
    }
    template <typename... Args>
    [[nodiscard]] SKR_FORCEINLINE T* NewZeroed(Args&&... args)
    {
        T* p = core.Alloc<T>();
        memset((void*)p, 0, sizeof(T));
        return new (p) SKR_DEBUG_NEW_SOURCE_LINE T{ std::forward<Args>(args)... };
    }
    template <class F>
    [[nodiscard]] SKR_FORCEINLINE F* NewLambda(F&& lambda)
    {
        F* p = core.Alloc<F>();
        return new (p) SKR_DEBUG_NEW_SOURCE_LINE auto(std::forward<F>(lambda));
    }
    void Delete(T* p, SkrDeleteFlag flags)
    {
        SKR_ASSERT(p != nullptr);
        if (!::skr::flag_all(flags, SkrDeleteFlag_No_Dtor))
        {
            p->~T();
        }
        core.Free<T>(reinterpret_cast<T*>(SkrDeleteTraits<T>::get_free_ptr(p)));
    }
};
#undef SKR_DEBUG_NEW_SOURCE_LINE

//=======================impl SkrNew/SkrDelete=======================
struct SkrNewWrapper
{
    SkrNewCore core;
    SKR_FORCEINLINE SkrNewWrapper(SkrNewCore core)
        : core(core)
    {
    }

    template <typename T, typename... Args>
    [[nodiscard]] SKR_FORCEINLINE T* New(Args&&... args)
    {
        return SkrNewImpl<T>(core).New(std::forward<Args>(args)...);
    }
    template <typename T, typename... Args>
    [[nodiscard]] SKR_FORCEINLINE T* NewZeroed(Args&&... args)
    {
        return SkrNewImpl<T>(core).NewZeroed(std::forward<Args>(args)...);
    }
    template <class F>
    [[nodiscard]] SKR_FORCEINLINE F* NewLambda(F&& lambda)
    {
        return SkrNewImpl<F>(core).NewLambda(std::forward<F>(lambda));
    }
    template <typename T>
    void Delete(T* p, SkrDeleteFlag flags = SkrDeleteFlag_None)
    {
        return SkrNewImpl<T>(core).Delete(p, flags);
    }
};

#define SkrNew SkrNewWrapper(SKR_MAKE_NEW_CORE).New
#define SkrNewZeroed SkrNewWrapper(SKR_MAKE_NEW_CORE).NewZeroed
#define SkrNewLambda SkrNewWrapper(SKR_MAKE_NEW_CORE).NewLambda
#define SkrDelete SkrNewWrapper(SKR_MAKE_NEW_CORE).Delete

#define SkrNewN(__N) SkrNewWrapper(SKR_MAKE_NEW_CORE_N(__N)).New
#define SkrNewZeroedN(__N) SkrNewWrapper(SKR_MAKE_NEW_CORE_N(__N)).NewZeroed
#define SkrNewLambdaN(__N) SkrNewWrapper(SKR_MAKE_NEW_CORE_N(__N)).NewLambda
#define SkrDeleteN(__N) SkrNewWrapper(SKR_MAKE_NEW_CORE_N(__N)).Delete
