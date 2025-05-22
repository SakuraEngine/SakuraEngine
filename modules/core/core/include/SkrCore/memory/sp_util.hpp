#pragma once
#include <SkrBase/atomic/atomic_mutex.hpp>
#include <SkrCore/memory/memory.h>
#include <SkrBase/config.h>
#include <atomic>
#include <SkrBase/types/expected.hpp>
#include <concepts>

namespace skr
{
//========================SP ref counter========================
using SPCounterType = uint32_t;
template <typename T>
concept ObjectWithSPDeleter = requires(T* p) {
    { p->skr_sp_delete() } -> std::same_as<void>;
};
template <typename From, typename To>
concept SPConvertible = requires() {
    std::convertible_to<From*, To*>;
};
template <typename T>
struct SPDeleterTraits {
    inline static void do_delete(T* p)
    {
        SkrDelete(p);
    }
};
template <ObjectWithSPDeleter T>
struct SPDeleterTraits<T> {
    inline static void do_delete(T* p)
    {
        p->skr_sp_delete();
    }
};

struct SPRefCounter {
    inline SPCounterType ref_count() const
    {
        return _ref_count.load(std::memory_order_relaxed);
    }
    inline SPCounterType ref_count_weak() const
    {
        return _ref_count_weak.load(std::memory_order_relaxed);
    }
    inline void add_ref()
    {
        _ref_count.fetch_add(1, std::memory_order_relaxed);
        _ref_count_weak.fetch_add(1, std::memory_order_relaxed);
    }
    inline bool release()
    {
        SKR_ASSERT(_ref_count.load(std::memory_order_relaxed) > 0);
        if (_ref_count.fetch_sub(1, std::memory_order_release) == 1)
        {
            atomic_thread_fence(std::memory_order_acquire);
            return true;
        }

        release_weak();
        return false;
    }
    inline void add_ref_weak()
    {
        _ref_count_weak.fetch_add(1, std::memory_order_relaxed);
    }
    inline void release_weak()
    {
        SKR_ASSERT(_ref_count_weak.load(std::memory_order_relaxed) > 0);
        if (_ref_count_weak.fetch_sub(1, std::memory_order_release) == 1)
        {
            atomic_thread_fence(std::memory_order_acquire);
            SkrDelete(this);
        }
    }
    inline bool try_lock_weak()
    {
        for (SPCounterType old = _ref_count.load(std::memory_order_relaxed); old != 0;)
        {
            if (_ref_count.compare_exchange_weak(
                    old,
                    old + 1,
                    std::memory_order_relaxed
                ))
            {
                _ref_count_weak.fetch_add(1, std::memory_order_relaxed);
                return true;
            }
        }
        return false;
    }
    inline bool is_alive() const
    {
        return _ref_count.load(std::memory_order_relaxed) > 0;
    }

private:
    std::atomic<SPCounterType> _ref_count      = 0;
    std::atomic<SPCounterType> _ref_count_weak = 0;
};

} // namespace skr