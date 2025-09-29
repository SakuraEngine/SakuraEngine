#pragma once
#include <SkrBase/meta.h>
#include <SkrBase/types/expected.hpp>
#include <SkrBase/atomic/atomic_mutex.hpp>
#include <SkrCore/memory/memory.h>
#include <SkrBase/template/concepts.hpp>

namespace skr
{
// fwd
struct RCBlock;
struct RCWeakRefCounter;

// concept
namespace concepts
{
template <typename T>
concept ObjectWithRC = requires(const T* const_obj, T* obj) {
    { const_obj->skr_rc_get_block() } -> std::same_as<skr::RCBlock*>;
};
template <typename T>
concept ObjectWithRCDeleter = requires(const T* const_obj, T* obj) {
    { obj->skr_rc_delete() } -> std::same_as<void>;
};
template <typename T>
concept RCAble = !CompletedType<T> || ObjectWithRC<T>;
template <typename From, typename To>
concept RCConvertible =
    std::convertible_to<From*, To*> &&
    RCAble<From> &&
    RCAble<To>;
} // namespace concepts

// deleter traits
template <typename T>
struct RCDeleterTraits
{
    inline static void do_delete(T* obj)
    {
        SkrDelete(obj);
    }
};
template <concepts::ObjectWithRCDeleter T>
struct RCDeleterTraits<T>
{
    inline static void do_delete(T* obj)
    {
        obj->skr_rc_delete();
    }
};

// RC counter type
using RCCounterType = uint64_t;
inline static constexpr RCCounterType kRCCounterUniqueFlag = RCCounterType(1) << (std::numeric_limits<RCCounterType>::digits - 1);
inline static constexpr RCCounterType kRCCounterMax = kRCCounterUniqueFlag - 1;

// weak block
struct RCWeakRefCounter
{
    // ref count
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

    // object state
    inline void notify_object_dead()
    {
        SKR_ASSERT(_is_alive.load(std::memory_order_relaxed) == true);
        _is_alive.store(false, std::memory_order_relaxed);
    }
    inline bool is_alive() const
    {
        return _is_alive.load(std::memory_order_relaxed);
    }
    inline void lock_for_use_object()
    {
        _delete_mutex.lock_shared();
    }
    inline void unlock_for_use_object()
    {
        _delete_mutex.unlock_shared();
    }
    inline void lock_for_delete_object()
    {
        _delete_mutex.lock();
    }
    inline void unlock_for_delete_object()
    {
        _delete_mutex.unlock();
    }

private:
    std::atomic<RCCounterType> _ref_count = 0; // weak ref count
    shared_atomic_mutex _delete_mutex = {};    // mutex used to keep object alive until all weak locks released
    std::atomic<bool> _is_alive = true;
};

// rc block
struct RCBlock
{
    // ref count ops
    inline RCCounterType ref_count() const
    {
        return _ref_count.load(std::memory_order_relaxed) & ~kRCCounterUniqueFlag;
    }
    inline RCCounterType add_ref()
    {
        // load value
        RCCounterType old = _ref_count.load(std::memory_order_relaxed);
        SKR_ASSERT(!_is_unique(old) && "try to ref a unique object use shared way");

        // CAS
        while (!_ref_count.compare_exchange_weak(
            old,
            old + 1,
            std::memory_order_relaxed
        ))
        {
            SKR_ASSERT(!_is_unique(old) && "try to ref a unique object use shared way");
        }

        return old + 1;
    }
    inline RCCounterType unsafe_release()
    {
        RCCounterType old = _ref_count.fetch_sub(1, std::memory_order_release);
        if (_is_unique(old))
        {
            SKR_ASSERT(false && "try to release a unique object use shared way");
            return 0;
        }
        else
        {
            return old - 1;
        }
    }
    template <typename T>
    inline void release(const T* obj)
    {
        SKR_ASSERT(obj != nullptr);
        if (unsafe_release() == 0)
        {
            notify_weak_ref_counter_dead();
            RCDeleterTraits<T>::do_delete(const_cast<T*>(obj));
        }
    }
    inline RCCounterType add_ref_unique()
    {
        RCCounterType old = _ref_count.load(std::memory_order_relaxed);
        SKR_ASSERT(old == 0 && "try to ref a shared object use unique way");

        while (!_ref_count.compare_exchange_weak(
            old,
            kRCCounterUniqueFlag | 1,
            std::memory_order_relaxed
        ))
        {
            SKR_ASSERT(old == 0 && "try to ref a shared object use unique way");
        }
        return kRCCounterUniqueFlag;
    }
    inline RCCounterType unsafe_release_unique()
    {
        RCCounterType old = _ref_count.load(std::memory_order_relaxed);
        SKR_ASSERT(_is_unique(old) && "try to release a shared object use unique way");

        while (!_ref_count.compare_exchange_weak(
            old,
            0,
            std::memory_order_relaxed
        ))
        {
            SKR_ASSERT(_is_unique(old) && "try to release a shared object use unique way");
        }
        return 0;
    }
    template <typename T>
    inline void release_unique(const T* obj)
    {
        SKR_ASSERT(obj != nullptr);
        SKR_ASSERT(unsafe_release_unique() == 0);

        notify_weak_ref_counter_dead();
        RCDeleterTraits<T>::do_delete(const_cast<T*>(obj));
    }

    // weak api
    inline RCCounterType weak_lock()
    {
        for (RCCounterType old = _ref_count.load(std::memory_order_relaxed); old != 0;)
        {
            SKR_ASSERT(!_is_unique(old) && "try to lock a unique object");
            if (_ref_count.compare_exchange_weak(
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
    inline RCCounterType weak_ref_count() const
    {
        RCWeakRefCounter* weak_counter = _weak_counter.load(std::memory_order_relaxed);
        if (!weak_counter || _is_object_released(weak_counter))
        { // no weak counter or object has been released by other thread
            return 0;
        }
        else
        {
            std::atomic_thread_fence(std::memory_order_acquire);
            return weak_counter->ref_count();
        }
    }
    inline RCWeakRefCounter* get_or_new_weak_ref_counter()
    {
        RCWeakRefCounter* weak_counter = _weak_counter.load(std::memory_order_relaxed);
        if (weak_counter)
        {
            return _is_object_released(weak_counter) ? nullptr : weak_counter;
        }
        else
        {
            RCWeakRefCounter* new_counter = SkrNew<RCWeakRefCounter>();
            if (_weak_counter.compare_exchange_weak(
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
                if (_is_object_released(weak_counter))
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

    // notify weak counter the object is dead
    inline void notify_weak_ref_counter_dead()
    {
        RCWeakRefCounter* weak_counter = _weak_counter.load(std::memory_order_relaxed);

        // take release permissions
        while (!_weak_counter.compare_exchange_weak(
            weak_counter,
            _weak_ref_on_obj_released(),
            std::memory_order_release
        ))
        {
            if (_is_object_released(weak_counter))
            {
                // unexpected, another thread released the object
                // release race should handled in release() or release_unique()
                SKR_UNREACHABLE_CODE();
            }
            else
            {
                // another thread created a weak counter when we are deleting the object
                SKR_ASSERT(weak_counter != nullptr);
            }
        }

        // now release the weak counter
        if (weak_counter)
        {
            // lock for delete
            weak_counter->lock_for_delete_object();

            // release
            std::atomic_thread_fence(std::memory_order_acquire);
            weak_counter->notify_object_dead();
            weak_counter->release();

            // unlock for delete
            weak_counter->unlock_for_delete_object();
        }
    }

    //! Note: reset for pooling object,
    inline void unsafe_reset()
    {
        _ref_count.store(0, std::memory_order_relaxed);
        _weak_counter.store(nullptr, std::memory_order_relaxed);
    }

private:
    inline static bool _is_unique(RCCounterType counter)
    {
        return (counter & kRCCounterUniqueFlag) != 0;
    }
    // use 0xFFFF'FFFF'FFFF'FFFF to indicate the object has been released
    inline static RCWeakRefCounter* _weak_ref_on_obj_released()
    {
        return reinterpret_cast<RCWeakRefCounter*>(std::numeric_limits<std::uintptr_t>::max());
    }
    inline static bool _is_object_released(RCWeakRefCounter* weak_counter)
    {
        return reinterpret_cast<std::uintptr_t>(weak_counter) == std::numeric_limits<std::uintptr_t>::max();
    }

private:
    std::atomic<RCCounterType> _ref_count = 0;
    std::atomic<RCWeakRefCounter*> _weak_counter = nullptr;
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
#define SKR_RC_INTEFACE() \
    virtual skr::RCBlock* skr_rc_get_block() const = 0;

#define SKR_RC_DELETER_INTERFACE() \
    virtual void skr_rc_delete() = 0;

// impl macros
#define SKR_RC_IMPL(__SUFFIX)                                \
private:                                                     \
    [[sattr(serde = @disable)]]                                \
    mutable ::skr::RCBlock zz_skr_rc_block;                  \
                                                             \
public:                                                      \
    inline ::skr::RCBlock* skr_rc_get_block() const __SUFFIX \
    {                                                        \
        return &zz_skr_rc_block;                             \
    }

#define SKR_RC_DELETER_IMPL_DEFAULT(__SUFFIX) \
    inline void skr_rc_delete() __SUFFIX      \
    {                                         \
        SkrDelete(this);                      \
    }

// TODO. remove it
namespace skr
{
struct SKR_CORE_API IRCAble
{
    virtual ~IRCAble() SKR_NOEXCEPT = default;
    SKR_RC_INTEFACE();
    SKR_RC_DELETER_INTERFACE();
};
} // namespace skr