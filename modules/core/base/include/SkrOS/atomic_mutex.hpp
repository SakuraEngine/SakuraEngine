#pragma once
#include <atomic>
#include <assert.h>

namespace atomic_mutex
{
	/* Provides similar capability as std::shared_mutex but does so with "lock-free programming" atomic operations.
	Also provides an atomic shared-to-unique-lock upgrade capability, which std::shared_mutex does not provide.

	This can be used with std::shared_lock, std::unique_lock, etc. but is best used with the atomic_mutex::upgradable_lock.

	This mutex works by keeping track of the number of threads with a shared lock and the number of threads waiting for a unique lock
	(one of which may be uniquely locked). Unique locks have priority: all threads asking for a shared lock are blocked until the
	number of threads waiting for a unique lock is zero. All threads asking for a unique lock are blocked until the number of threads
	with a shared lock is zero, and also until there is no active unique lock (each thread asking for a unique lock proceeds one-at-
	a-time).

	To guarantee atomicity of all shared/unique thread counting, updating, and checking, all data is stored in one atomic integer using
	bit masking to count the number of threads *with* shared locks, the number of the threads *waiting* for a unique lock, and a flag
	indicating whether there is an active unique lock.

	The order of thread unblocking is not guaranteed. All threads just spin in loops until unblocked.

	Example usage:
		shared_atomic_mutex m;
		m.lock_shared();
		*read stuff from a cache on this line*
		if (*need to write to the cache*)
		{
			m.upgrade();
			*write stuff to cache*
			m.unlock();
		}
		else
		{
			m.unlock_shared();
		}

	Dos and don'ts:
	- Do call unlock_shared() when a shared lock is no longer necessary. You must explicitly unlock.
	- Do call unlock() when a unique lock is no longer necessary. You must explicitly unlock.
	- Do not acquire a unique lock before unlocking any shared lock in the same thread, unless upgrading.
	- Do not acquire a shared lock before unlocking any unique lock in the same thread.

	Helpful for understanding the flexibility of atomic operations:
	http://preshing.com/20150402/you-can-do-any-kind-of-atomic-read-modify-write-operation/
	*/
	class shared_atomic_mutex
	{
	private:
		typedef uint32_t bitfield_t;
		typedef uint16_t val_t; //! only 2^16 shared and 2^15 unique threads may use this lock 

	public:
		shared_atomic_mutex() {}
		shared_atomic_mutex(const shared_atomic_mutex&) = delete;
		shared_atomic_mutex& operator=(const shared_atomic_mutex&) = delete;

		// Acquire the unique lock, first waiting until all shared locks are unlocked.
		void lock()
		{
			// Increment the unique lock count (which is the number of threads waiting for a unique lock).
			// Incrementing immediately, whether actively uniquely locked or not, blocks any new thread asking for a new shared lock.
			m_bitfield += one_unique_thread;
			acquire_unique();
		}

		// Unlock the unique lock.
		void unlock()
		{
			assert(num_unique_locks() > 0 && is_unique_locked());
			m_bitfield -= (one_unique_flag + one_unique_thread);
		}

		// Upgrade a previously-acquired shared lock to the unique lock.
		void upgrade()
		{
			static_assert(one_unique_thread > one_shared_thread, "This section of code assumes one_unique_thread > one_shared_thread for subtraction");
			// increment the unique lock count and remove this thread from the shared lock thread count
			m_bitfield += (one_unique_thread - one_shared_thread);
			acquire_unique();
		}

		// Acquire a shared lock, first waiting until all unique locks are unlocked.
		void lock_shared()
		{
			// Unlike the unique lock, don't immediately increment the shared count (wait until there are no unique lock requests before adding).
			bitfield_t oldval = m_bitfield;
			bitfield_t newval = oldval;
			do
			{
				// Proceed if any number of shared locks and no unique locks are waiting or active.
				any_shared_no_unique(oldval);
				// New value is expected value with a new shared lock (increment the shared lock count).
				newval = oldval + one_shared_thread;

				// If m_bitfield==oldval (there are no unique locks) then store newval in m_bitfield (add a shared lock).
				// Otherwise update oldval with the latest value of m_bitfield and run the test loop again.
			} while (!m_bitfield.compare_exchange_weak(oldval, newval, std::memory_order_relaxed));
		}

		// Unlock a shared lock.
		void unlock_shared()
		{
			assert(num_shared_locks() > 0);
			m_bitfield -= one_shared_thread;
		}

		// Returns the number of threads with an active shared lock.
		val_t num_shared_locks()
		{
			return val_t((m_bitfield & num_shared_mask) >> num_shared_bitshift);
		}

		// Returns the number of threads waiting for a unique lock, one of which may have an active unique lock.
		val_t num_unique_locks()
		{
			return val_t((m_bitfield & num_unique_mask) >> num_unique_bitshift);
		}

		// Returns whether the mutex is currently uniquely locked.
		bool is_unique_locked()
		{
			return ((m_bitfield & unique_flag_mask) >> unique_flag_bitshift) != 0;
		}

	private:
		void acquire_unique()
		{
			bitfield_t oldval = m_bitfield;
			bitfield_t newval = oldval;
			do
			{
				// Proceed if there are no shared locks and the unique lock is available (unique lock flag is 0).
				no_shared_no_unique(oldval);
				// Set the unique lock flag to 1.
				newval = oldval + one_unique_flag;

				// If m_bitfield==oldval (there are no active shared locks and no thread has a unique lock) then store newval in m_bitfield (get the unique lock).
				// Otherwise update oldval with the latest value of m_bitfield and run the test loop again.
			} while (!m_bitfield.compare_exchange_weak(oldval, newval, std::memory_order_relaxed));
		}

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

		std::atomic<bitfield_t> m_bitfield = 0; // consists of [ 1 bit unique lock flag, 15 bits # of unique locks, 16 bits # of shared locks]
	};
}