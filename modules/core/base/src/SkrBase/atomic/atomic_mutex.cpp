#include "SkrBase/misc/debug.h"
#include "SkrBase/atomic/atomic_mutex.hpp"

namespace skr
{

shared_atomic_mutex::shared_atomic_mutex() SKR_NOEXCEPT
    : _bitfield(0)
{

}

// Acquire the unique lock, first waiting until all shared locks are unlocked.
void shared_atomic_mutex::lock() SKR_NOEXCEPT
{
    // Increment the unique lock count (which is the number of threads waiting for a unique lock).
    // Incrementing immediately, whether actively uniquely locked or not, blocks any new thread asking for a new shared lock.
    skr_atomic_fetch_add(&_bitfield, one_unique_thread);
    acquire_unique();
}

void shared_atomic_mutex::unlock() SKR_NOEXCEPT
{
    SKR_ASSERT(num_unique_locks() > 0 && is_unique_locked());
    skr_atomic_fetch_sub(&_bitfield, one_unique_flag + one_unique_thread);
}

void shared_atomic_mutex::upgrade() SKR_NOEXCEPT
{
    static_assert(one_unique_thread > one_shared_thread, "This section of code assumes one_unique_thread > one_shared_thread for subtraction");
    // increment the unique lock count and remove this thread from the shared lock thread count
    skr_atomic_fetch_add(&_bitfield, one_unique_thread - one_shared_thread);
    acquire_unique();
}

void shared_atomic_mutex::lock_shared() SKR_NOEXCEPT
{
    // Unlike the unique lock, don't immediately increment the shared count (wait until there are no unique lock requests before adding).
    bitfield_t oldval = skr_atomic_load(&_bitfield);
    bitfield_t newval = oldval;
    do
    {
        // Proceed if any number of shared locks and no unique locks are waiting or active.
        any_shared_no_unique(oldval);
        // New value is expected value with a new shared lock (increment the shared lock count).
        newval = oldval + one_shared_thread;

        // If _bitfield==oldval (there are no unique locks) then store newval in _bitfield (add a shared lock).
        // Otherwise update oldval with the latest value of _bitfield and run the test loop again.
    } while ((
        !skr_atomic_compare_exchange_weak_explicit(&_bitfield, &oldval, newval, skr_memory_order_relaxed, skr_memory_order_relaxed)
    ));
}

void shared_atomic_mutex::unlock_shared() SKR_NOEXCEPT
{
    assert(num_shared_locks() > 0);
    skr_atomic_fetch_sub(&_bitfield, one_shared_thread);
}

shared_atomic_mutex::val_t shared_atomic_mutex::num_shared_locks() SKR_NOEXCEPT
{
    const auto mask = skr_atomic_load(&_bitfield) & num_shared_mask;
    return val_t(mask >> num_shared_bitshift);
}

shared_atomic_mutex::val_t shared_atomic_mutex::num_unique_locks() SKR_NOEXCEPT
{
    const auto mask = skr_atomic_load(&_bitfield) & num_unique_mask;
    return val_t(mask >> num_unique_bitshift);
}

bool shared_atomic_mutex::is_unique_locked() SKR_NOEXCEPT
{
    const auto mask = skr_atomic_load(&_bitfield) & unique_flag_mask;
    return (mask >> unique_flag_bitshift) != 0;
}

void shared_atomic_mutex::acquire_unique() SKR_NOEXCEPT
{
    bitfield_t oldval = _bitfield;
    bitfield_t newval = oldval;
    do
    {
        // Proceed if there are no shared locks and the unique lock is available (unique lock flag is 0).
        no_shared_no_unique(oldval);
        // Set the unique lock flag to 1.
        newval = oldval + one_unique_flag;

        // If _bitfield==oldval (there are no active shared locks and no thread has a unique lock) then store newval in _bitfield (get the unique lock).
        // Otherwise update oldval with the latest value of _bitfield and run the test loop again.
    } while ((
        !skr_atomic_compare_exchange_weak_explicit(&_bitfield, &oldval, newval, skr_memory_order_relaxed, skr_memory_order_relaxed)
    ));
}


} // namespace skr