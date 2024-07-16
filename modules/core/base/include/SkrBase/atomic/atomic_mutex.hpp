#pragma once
#include "SkrBase/config.h"
#include "SkrBase/atomic/atomic.h"

namespace skr
{
struct SKR_STATIC_API shared_atomic_mutex
{
public:
	using bitfield_t = uint32_t;
	using val_t = uint16_t;

	shared_atomic_mutex() SKR_NOEXCEPT;
	shared_atomic_mutex(const shared_atomic_mutex&) = delete;
	shared_atomic_mutex& operator=(const shared_atomic_mutex&) = delete;

	// Acquire the unique lock, first waiting until all shared locks are unlocked.
	void lock() SKR_NOEXCEPT;

	// Unlock the unique lock.
	void unlock() SKR_NOEXCEPT;

	// Upgrade a previously-acquired shared lock to the unique lock.
	void upgrade() SKR_NOEXCEPT;

	// Acquire a shared lock, first waiting until all unique locks are unlocked.
	void lock_shared() SKR_NOEXCEPT;

	// Unlock a shared lock.
	void unlock_shared() SKR_NOEXCEPT;

	// Returns the number of threads with an active shared lock.
	val_t num_shared_locks() SKR_NOEXCEPT;

	// Returns the number of threads waiting for a unique lock, one of which may have an active unique lock.
	val_t num_unique_locks() SKR_NOEXCEPT;

	// Returns whether the mutex is currently uniquely locked.
	bool is_unique_locked() SKR_NOEXCEPT;

private:
	void acquire_unique() SKR_NOEXCEPT;

	static const uint8_t num_shared_bitshift = 0;
	static const uint8_t num_unique_bitshift = 16;
	static const uint8_t unique_flag_bitshift = 31;

	static const bitfield_t num_shared_mask = bitfield_t(65535) << num_shared_bitshift;
	static const bitfield_t num_unique_mask = bitfield_t(32767) << num_unique_bitshift;
	static const bitfield_t unique_flag_mask = bitfield_t(1) << unique_flag_bitshift;

	static const bitfield_t one_shared_thread = bitfield_t(1) << num_shared_bitshift;
	static const bitfield_t one_unique_thread = bitfield_t(1) << num_unique_bitshift;
	static const bitfield_t one_unique_flag = bitfield_t(1) << unique_flag_bitshift;

	static void any_shared_no_unique(bitfield_t& bits) { bits &= num_shared_mask; }
	static void no_shared_no_unique(bitfield_t& bits) { bits &= num_unique_mask; }

	_SAtomic(bitfield_t) _bitfield; // consists of [ 1 bit unique lock flag, 15 bits # of unique locks, 16 bits # of shared locks]
};
}