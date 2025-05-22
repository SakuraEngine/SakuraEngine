#pragma once
#include <SkrBase/atomic/atomic_mutex.hpp>
#include <SkrCore/memory/memory.h>
#include <SkrBase/config.h>
#include <atomic>
#include <SkrBase/types/expected.hpp>
#include <concepts>

namespace skr
{
using RCCounterType = uint64_t;

// constants
inline static constexpr RCCounterType kRCCounterUniqueFlag = 1 << 31;
inline static constexpr RCCounterType kRCCounterMax        = kRCCounterUniqueFlag - 1;

// operations
inline static RCCounterType rc_ref_count(const std::atomic<RCCounterType>& counter)
{
    return counter.load(std::memory_order_relaxed) & ~kRCCounterUniqueFlag;
}
inline static bool rc_is_unique(RCCounterType counter)
{
    return (counter & kRCCounterUniqueFlag) != 0;
}
inline static RCCounterType rc_add_ref(std::atomic<RCCounterType>& counter)
{
    RCCounterType old = counter.load(std::memory_order_relaxed);
    SKR_ASSERT(!rc_is_unique(old) && "try to add ref on a unique object");
    while (!counter.compare_exchange_weak(
        old,
        old + 1,
        std::memory_order_relaxed
    ))
    {
        SKR_ASSERT(!rc_is_unique(old) && "try to add ref on a unique object");
    }
    return old + 1;
}
inline static RCCounterType rc_weak_lock(std::atomic<RCCounterType>& counter)
{
    for (RCCounterType old = counter.load(std::memory_order_relaxed); old != 0;)
    {
        SKR_ASSERT(!rc_is_unique(old));
        if (counter.compare_exchange_weak(
                old,
                old + 1,
                std::memory_order_relaxed
            ))
        {
            return old;
        }
    }
    return 0;
}
inline static RCCounterType rc_add_ref_unique(std::atomic<RCCounterType>& counter)
{
    RCCounterType old = counter.load(std::memory_order_relaxed);
    SKR_ASSERT(old == 0 && "try to add ref on a non-unique object");
    while (!counter.compare_exchange_weak(
        old,
        kRCCounterUniqueFlag | 1,
        std::memory_order_relaxed
    ))
    {
        SKR_ASSERT(old == 0 && "try to add ref on a non-unique object");
    }
    return kRCCounterUniqueFlag;
}
inline static RCCounterType rc_release_unique(std::atomic<RCCounterType>& counter)
{
    RCCounterType old = counter.load(std::memory_order_relaxed);
    SKR_ASSERT(rc_is_unique(old) && "try to release a non-unique object");
    while (!counter.compare_exchange_weak(
        old,
        0,
        std::memory_order_relaxed
    ))
    {
        SKR_ASSERT(rc_is_unique(old) && "try to release a non-unique object");
    }
    return 0;
}
inline static RCCounterType rc_release(std::atomic<RCCounterType>& counter)
{
    RCCounterType old = counter.fetch_sub(1, std::memory_order_release);
    if (rc_is_unique(old))
    {
        return 0;
    }
    else
    {
        return old - 1;
    }
}

// weak block
struct RCWeakRefCounter {
    inline RCCounterType ref_count() const
    {
        return _ref_count.load(std::memory_order_relaxed);
    }
    inline void add_ref()
    {
        _ref_count.fetch_add(1, std::memory_order_relaxed);
    }
    inline void release()
    {
        SKR_ASSERT(_ref_count.load(std::memory_order_relaxed) > 0);
        if (_ref_count.fetch_sub(1, std::memory_order_release) == 1)
        {
            std::atomic_thread_fence(std::memory_order_acquire);
            SkrDelete(this);
        }
    }
    inline void notify_dead()
    {
        SKR_ASSERT(_is_alive.load(std::memory_order_relaxed) == true);
        _is_alive.store(false, std::memory_order_relaxed);
    }
    inline bool is_alive() const
    {
        return _is_alive.load(std::memory_order_relaxed);
    }
    inline void lock_for_use()
    {
        _delete_mutex.lock_shared();
    }
    inline void unlock_for_use()
    {
        _delete_mutex.unlock_shared();
    }
    inline void lock_for_delete()
    {
        _delete_mutex.lock();
    }
    inline void unlock_for_delete()
    {
        _delete_mutex.unlock();
    }

private:
    std::atomic<RCCounterType> _ref_count    = 0;
    shared_atomic_mutex        _delete_mutex = {};
    std::atomic<bool>          _is_alive     = true;
};
inline static RCWeakRefCounter* rc_get_weak_ref_released()
{
    return reinterpret_cast<RCWeakRefCounter*>(size_t(-1));
}
inline static bool rc_is_weak_ref_released(RCWeakRefCounter* counter)
{
    return reinterpret_cast<size_t>(counter) == size_t(-1);
}
inline static RCCounterType rc_weak_ref_count(
    std::atomic<RCWeakRefCounter*>& counter
)
{
    RCWeakRefCounter* weak_counter = counter.load(std::memory_order_relaxed);
    if (!weak_counter || rc_is_weak_ref_released(weak_counter))
    {
        return 0;
    }
    else
    {
        std::atomic_thread_fence(std::memory_order_acquire);
        return weak_counter->ref_count();
    }
}
inline static RCWeakRefCounter* rc_get_or_new_weak_ref_counter(
    std::atomic<RCWeakRefCounter*>& counter
)
{
    RCWeakRefCounter* weak_counter = counter.load(std::memory_order_relaxed);
    if (weak_counter)
    {
        return rc_is_weak_ref_released(weak_counter) ? nullptr : weak_counter;
    }
    else
    {
        RCWeakRefCounter* new_counter = SkrNew<RCWeakRefCounter>();
        if (counter.compare_exchange_weak(
                weak_counter,
                new_counter,
                std::memory_order_release
            ))
        {
            std::atomic_thread_fence(std::memory_order_acquire);
            // add ref for keep it alive until any weak ref and self released
            new_counter->add_ref();
            return new_counter;
        }
        else
        {
            std::atomic_thread_fence(std::memory_order_acquire);
            if (rc_is_weak_ref_released(weak_counter))
            {
                // object has been released, delete the new one
                SkrDelete(new_counter);
                return nullptr;
            }
            else
            {
                // another thread created a weak counter, delete the new one
                SkrDelete(new_counter);
                return weak_counter;
            }
        }
    }
}
inline static void rc_notify_weak_ref_counter_dead(
    std::atomic<RCWeakRefCounter*>& counter
)
{
    RCWeakRefCounter* weak_counter = counter.load(std::memory_order_relaxed);

    // take release permissions
    while (!counter.compare_exchange_weak(
        weak_counter,
        rc_get_weak_ref_released(),
        std::memory_order_release
    ))
    {
        if (rc_is_weak_ref_released(weak_counter))
        {
            // unexpected, another thread released the weak counter
            SKR_UNREACHABLE_CODE();
        }
        else
        {
            // another thread created a weak counter
            SKR_ASSERT(weak_counter != nullptr);
        }
    }

    // now release the weak counter
    if (weak_counter)
    {
        // lock for delete
        weak_counter->lock_for_delete();

        // release
        std::atomic_thread_fence(std::memory_order_acquire);
        weak_counter->notify_dead();
        weak_counter->release();

        // unlock for delete
        weak_counter->unlock_for_delete();
    }
}

// concept
template <typename T>
concept ObjectWithRC = requires(const T* const_obj, T* obj) {
    { const_obj->skr_rc_count() } -> std::same_as<skr::RCCounterType>;
    { const_obj->skr_rc_add_ref() } -> std::same_as<skr::RCCounterType>;
    { const_obj->skr_rc_add_ref_unique() } -> std::same_as<skr::RCCounterType>;
    { const_obj->skr_rc_release_unique() } -> std::same_as<skr::RCCounterType>;
    { const_obj->skr_rc_weak_lock() } -> std::same_as<skr::RCCounterType>;
    { const_obj->skr_rc_release() } -> std::same_as<skr::RCCounterType>;
    { const_obj->skr_rc_weak_ref_count() } -> std::same_as<skr::RCCounterType>;
    { const_obj->skr_rc_weak_ref_counter() } -> std::same_as<skr::RCWeakRefCounter*>;
};
template <typename T>
concept ObjectWithRCDeleter = requires(const T* const_obj, T* obj) {
    { obj->skr_rc_delete() } -> std::same_as<void>;
};
template <typename From, typename To>
concept RCConvertible = requires(From obj) {
    ObjectWithRC<From>;
    ObjectWithRC<To>;
    std::convertible_to<From*, To*>;
};

// deleter traits
template <typename T>
struct RCDeleterTraits {
    inline static void do_delete(T* obj)
    {
        SkrDelete(obj);
    }
};
template <ObjectWithRCDeleter T>
struct RCDeleterTraits<T> {
    inline static void do_delete(T* obj)
    {
        obj->skr_rc_delete();
    }
};

// release helper
template <typename T>
inline void rc_release_with_delete(T* p)
{
    SKR_ASSERT(p != nullptr);
    if (p->skr_rc_release() == 0)
    {
        p->skr_rc_weak_ref_counter_notify_dead();
        RCDeleterTraits<T>::do_delete(p);
    }
}

} // namespace skr

// interface macros
#define SKR_RC_INTEFACE()                                                           \
    virtual skr::RCCounterType     skr_rc_count() const                        = 0; \
    virtual skr::RCCounterType     skr_rc_add_ref() const                      = 0; \
    virtual skr::RCCounterType     skr_rc_add_ref_unique() const               = 0; \
    virtual skr::RCCounterType     skr_rc_release_unique() const               = 0; \
    virtual skr::RCCounterType     skr_rc_weak_lock() const                    = 0; \
    virtual skr::RCCounterType     skr_rc_release() const                      = 0; \
    virtual skr::RCCounterType     skr_rc_weak_ref_count() const               = 0; \
    virtual skr::RCWeakRefCounter* skr_rc_weak_ref_counter() const             = 0; \
    virtual void                   skr_rc_weak_ref_counter_notify_dead() const = 0;
#define SKR_RC_DELETER_INTERFACE() \
    virtual void skr_rc_delete() = 0;

// impl macros
#define SKR_RC_IMPL(__SUFFIX)                                                      \
private:                                                                           \
    mutable ::std::atomic<::skr::RCCounterType>     zz_skr_rc           = 0;       \
    mutable ::std::atomic<::skr::RCWeakRefCounter*> zz_skr_weak_counter = nullptr; \
                                                                                   \
public:                                                                            \
    inline skr::RCCounterType skr_rc_count() const __SUFFIX                        \
    {                                                                              \
        return skr::rc_ref_count(zz_skr_rc);                                       \
    }                                                                              \
    inline skr::RCCounterType skr_rc_add_ref() const __SUFFIX                      \
    {                                                                              \
        return skr::rc_add_ref(zz_skr_rc);                                         \
    }                                                                              \
    inline skr::RCCounterType skr_rc_add_ref_unique() const __SUFFIX               \
    {                                                                              \
        return skr::rc_add_ref_unique(zz_skr_rc);                                  \
    }                                                                              \
    inline skr::RCCounterType skr_rc_release_unique() const __SUFFIX               \
    {                                                                              \
        return skr::rc_release_unique(zz_skr_rc);                                  \
    }                                                                              \
    inline skr::RCCounterType skr_rc_weak_lock() const __SUFFIX                    \
    {                                                                              \
        return skr::rc_weak_lock(zz_skr_rc);                                       \
    }                                                                              \
    inline skr::RCCounterType skr_rc_release() const __SUFFIX                      \
    {                                                                              \
        return skr::rc_release(zz_skr_rc);                                         \
    }                                                                              \
    inline skr::RCCounterType skr_rc_weak_ref_count() const __SUFFIX               \
    {                                                                              \
        return skr::rc_weak_ref_count(zz_skr_weak_counter);                        \
    }                                                                              \
    inline skr::RCWeakRefCounter* skr_rc_weak_ref_counter() const __SUFFIX         \
    {                                                                              \
        return skr::rc_get_or_new_weak_ref_counter(zz_skr_weak_counter);           \
    }                                                                              \
    inline void skr_rc_weak_ref_counter_notify_dead() const __SUFFIX               \
    {                                                                              \
        skr::rc_notify_weak_ref_counter_dead(zz_skr_weak_counter);                 \
    }
#define SKR_RC_DELETER_IMPL_DEFAULT(__SUFFIX) \
    inline void skr_rc_delete() __SUFFIX      \
    {                                         \
        SkrDelete(this);                      \
    }

// TODO. remove it
namespace skr
{
struct SKR_CORE_API IRCAble {
    virtual ~IRCAble() SKR_NOEXCEPT = default;
    SKR_RC_INTEFACE();
    SKR_RC_DELETER_INTERFACE();
};
} // namespace skr