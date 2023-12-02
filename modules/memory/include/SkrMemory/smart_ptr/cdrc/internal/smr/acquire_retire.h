
#ifndef CDRC_SMR_ACQUIRE_RETIRE_H
#define CDRC_SMR_ACQUIRE_RETIRE_H

#include <cassert>
#include <cstddef>

#include <algorithm>
#include <array>
#include <atomic>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../counted_object.h"
#include "../memory_manager_base.h"
#include "../utils.h"

namespace cdrc {

namespace internal {

// An interface for safe memory reclamation that protects reference-counted
// resources by deferring their reference count decrements until no thread
// is still reading them.
//
// Unlike hazard pointers, acquire-retire allows multiple concurrent retires
// of the same handle, which is what makes it suitable for managing reference
// counted pointers, since multiple copies of the same reference counted pointer
// may need to be destructed (i.e., have their counter decremented) concurrently.
//
// T =              The underlying type of the object being protected
// snapshot_slots = The number of additional announcement slots available for
//                  snapshot pointers. More allows more snapshots to be alive
//                  at a time, but makes reclamation slower
// eject_delay =    The maximum number of deferred ejects that will be held by
//                  any one worker thread is at most eject_delay * #threads.
//
template<typename T, size_t snapshot_slots = 7, size_t eject_delay = 2>
struct acquire_retire : public memory_manager_base<T, acquire_retire<T, snapshot_slots, eject_delay>> {

  using base = memory_manager_base<T, acquire_retire<T, snapshot_slots, eject_delay>>;

  using base::increment_allocations;
  using base::decrement_allocations;
  using base::increment_ref_cnt;
  using base::decrement_weak_cnt;
  using base::eject;

 private:

  using counted_object_t = counted_object<T>;
  using counted_ptr_t = std::add_pointer_t<counted_object_t>;

  // Align to cache line boundary to avoid false sharing
  struct alignas(128) LocalSlot {
    std::atomic<counted_ptr_t> announcement;
    std::array<std::atomic<counted_ptr_t>, snapshot_slots> snapshot_announcements{};
    alignas(128) size_t last_free{0};

    LocalSlot() : announcement(nullptr) {
      for (auto &a : snapshot_announcements) {
        std::atomic_init(&a, nullptr);
      }
    }
  };

 public:

  static acquire_retire& instance() {
    static acquire_retire ar{utils::num_threads()};
    return ar;
  }

  template<typename... Args>
  counted_ptr_t create_object(Args &&... args) {
    increment_allocations();
    return new counted_object_t(std::forward<Args>(args)...);
  }

  void delete_object(counted_ptr_t p) {
    delete p;
    decrement_allocations();
  }

  // An RAII wrapper around an acquired handle. Automatically
  // releases the handle when the wrapper goes out of scope.
  template<typename U>
  struct acquired_pointer {
   public:
    friend struct acquire_retire;

    acquired_pointer() : value(nullptr), slot(nullptr) {}

    acquired_pointer(U value_, std::atomic<counted_ptr_t>* slot_) : value(value_), slot(slot_) {}

    acquired_pointer(acquired_pointer&& other) noexcept : value(other.value), slot(other.slot) {
      other.value = nullptr;
      other.slot = nullptr;
    }

    ~acquired_pointer() { clear_protection(); }

    acquired_pointer& operator=(acquired_pointer&& other) noexcept {
      value = other.value;
      slot = other.slot;
      other.value = nullptr;
      other.slot = nullptr;
    }

    void swap(acquired_pointer &other) {
      std::swap(value, other.value);
      std::swap(slot, other.slot);
    }

    U& get() {
      return value;
    }

    U get() const {
      return value;
    }

    bool is_protected() const {
      return slot != nullptr && value != nullptr;
    }

    void clear_protection() {
      if (value != nullptr && slot != nullptr) {
        slot->store(nullptr, std::memory_order_release);
      }
    }

    void clear() {
      clear_protection();
      value = nullptr;
      slot = nullptr;
    }

   private:
    U value;
    std::atomic<counted_ptr_t> *slot;
  };

  explicit acquire_retire(size_t num_threads) :
      base(num_threads),
      announcement_slots(num_threads),
      in_progress(num_threads),
      deferred_destructs(num_threads),
      amortized_work(num_threads) {}

  template<typename U>
  [[nodiscard]] acquired_pointer<U> acquire(const std::atomic<U> *p) {
    auto id = utils::ThreadID::GetThreadID().getTID();
    U result;
    do {
      result = p->load(std::memory_order_seq_cst);
      announcement_slots[id].announcement.store(static_cast<counted_ptr_t>(result), std::memory_order_seq_cst);
    } while (p->load(std::memory_order_seq_cst) != result);
    return acquired_pointer<U>(result, &announcement_slots[id].announcement);
  }

  // Like acquire, but assuming that the caller already has a
  // copy of the handle and knows that it is protected
  template<typename U>
  [[nodiscard]] acquired_pointer<U> reserve(U p) {
    auto id = utils::ThreadID::GetThreadID().getTID();
    announcement_slots[id].announcement.store(static_cast<counted_ptr_t>(p),
                                              std::memory_order_seq_cst); // TODO: memory_order_release could be sufficient here
    return acquired_pointer<U>(p, &announcement_slots[id].announcement);
  }

  // Dummy function for when we need to conditionally reserve
  // something, but might need to reserve nothing
  template<typename U>
  [[nodiscard]] acquired_pointer<U> reserve_nothing() const {
    return {};
  }

  template<typename U>
  [[nodiscard]] acquired_pointer<U> protect_snapshot(const std::atomic<U> *p) {
    auto *slot = get_free_slot();

    // If no snapshot slot is available, just increment the reference count
    if (slot == nullptr) {
      while (true) {
        auto a = acquire(p);
        if (a.get() && increment_ref_cnt(a.get())) return acquired_pointer<U>(a.get(), nullptr);
        else if (a.get() == nullptr || p->load() == a.get()) return acquired_pointer<U>(nullptr, nullptr);
      }
    }

    U result;
    do {
      result = p->load(std::memory_order_seq_cst);
      PARLAY_PREFETCH(result, 0, 0);
      if (result == nullptr) {
        slot->store(nullptr, std::memory_order_release);
        return acquired_pointer<U>(result, nullptr);
      }
      slot->store(static_cast<counted_ptr_t>(result), std::memory_order_seq_cst);
    } while (p->load(std::memory_order_seq_cst) != result);
    return acquired_pointer<U>(result, slot);
  }

  [[nodiscard]] std::atomic<counted_ptr_t> *get_free_slot() {
    assert(snapshot_slots != 0);
    auto id = utils::ThreadID::GetThreadID().getTID();
    for (size_t i = 0; i < snapshot_slots; i++) {
      if (announcement_slots[id].snapshot_announcements[i].load(std::memory_order_acquire) == nullptr) {
        return std::addressof(announcement_slots[id].snapshot_announcements[i]);
      }
    }
    return nullptr;
  }

  void release() {
    auto id = utils::ThreadID::GetThreadID().getTID();
    auto &slot = announcement_slots[id].announcement;
    slot.store(nullptr, std::memory_order_release);
  }

  void retire(counted_ptr_t p, RetireType type) {
    auto id = utils::ThreadID::GetThreadID().getTID();
    deferred_destructs[id].emplace_back(p, type);
    work_toward_deferred_decrements(1);
  }

  // Perform any remaining deferred destruction. Need to be very careful
  // about additional objects being queued for deferred destruction by
  // an object that was just destructed.
  ~acquire_retire() {
    in_progress.assign(in_progress.size(), true);

    // Loop because the destruction of one object could trigger the deferred
    // destruction of another object (possibly even in another thread), and
    // so on recursively.
    while (std::any_of(deferred_destructs.begin(), deferred_destructs.end(),
                       [](const auto &v) { return !v.empty(); })) {

      // Move all of the contents from the deferred destruction lists
      // into a single local list. We don't want to just iterate the
      // deferred lists because a destruction may trigger another
      // deferred destruction to be added to one of the lists, which
      // would invalidate its iterators
      std::vector<std::pair<counted_ptr_t,RetireType>> destructs;
      for (auto &v : deferred_destructs) {
        destructs.insert(destructs.end(), v.begin(), v.end());
        v.clear();
      }

      // Perform all of the pending deferred destructions
      for (const auto& x : destructs) {
        eject(x.first, x.second);
      }
    }
  }

 private:
  // Apply the function f to every currently announced handle
  template<typename F>
  void scan_slots(F &&f) {
    std::atomic_thread_fence(std::memory_order_seq_cst);
    for (const auto &announcement_slot : announcement_slots) {
      auto x = announcement_slot.announcement.load(std::memory_order_seq_cst);
      if (x != nullptr) f(x);
      for (const auto &free_slot : announcement_slot.snapshot_announcements) {
        auto y = free_slot.load(std::memory_order_seq_cst);
        if (y != nullptr) f(y);
      }
    }
  }

  void work_toward_deferred_decrements(size_t work = 1) {
    auto id = utils::ThreadID::GetThreadID().getTID();
    amortized_work[id] = amortized_work[id] + work;
    auto threshold = std::max<size_t>(30, eject_delay * amortized_work.size());  // Always attempt at least 30 ejects
    while (!in_progress[id] && amortized_work[id] >= threshold) {
      amortized_work[id] = 0;
      if (deferred_destructs[id].size() == 0) break; // nothing to collect
      in_progress[id] = true;
      auto deferred = AlignedVector<std::pair<counted_ptr_t,RetireType>>(std::move(deferred_destructs[id]));

      // We need a custom hash because the standard doesn't know how to hash enum types...
      struct Hash {
        std::size_t operator()(std::pair<counted_ptr_t, RetireType> t) const {
          return std::hash<counted_ptr_t>{}(t.first) ^ std::hash<std::size_t>{}(static_cast<std::size_t>(t.second));
        }
      };

      // Since there can be multiple kinds of deferred actions (delayed ejects), each announcement
      // needs to be able to protect each kind of action, since announcements do not specify which
      // actions they wish to protect against.
      std::unordered_map<std::pair<counted_ptr_t, RetireType>, unsigned int, Hash> announced;
      scan_slots([&](auto reserved) {
        for (size_t i = 0; i < num_retire_types; i++) {
          // The first announcement needs to protect up to two actions
          auto& cnt = announced[std::make_pair(reserved, static_cast<RetireType>(i))];
          if (cnt) cnt++;
          else cnt = 2;
        }
      });

      // For a given deferred decrement, we first check if it is announced, and, if so,
      // we defer it again. If it is not announced, it can be safely applied. If an
      // object is deferred / announced multiple times, each announcement only protects
      // against one of the deferred decrements, so for each object, the amount of
      // decrements applied in total will be #deferred - #announced
      auto f = [this, &announced](const auto& x) {
        auto it = announced.find(x);
        if (it == announced.end()) {
          eject(x.first, x.second);
          return true;
        } else {
          if (--(it->second) == 0) announced.erase(it);
          return false;
        }
      };

      // Remove the deferred decrements that are successfully applied
      deferred.erase(remove_if(deferred.begin(), deferred.end(), f), deferred.end());
      deferred_destructs[id].insert(deferred_destructs[id].end(), deferred.begin(), deferred.end());
      in_progress[id] = false;
    }
  }

  std::vector<LocalSlot> announcement_slots;          // Announcement array slots
  std::vector<AlignedBool> in_progress;               // Local flags to prevent reentrancy while destructing
  std::vector<AlignedVector<std::pair<counted_ptr_t, RetireType>>> deferred_destructs;   // Thread-local lists of pending deferred destructs
  std::vector<AlignedInt> amortized_work;             // Amortized work to pay for ejecting deferred destructs
};

}  // namespace internal

}  // namespace cdrc

#endif  // CDRC_SMR_ACQUIRE_RETIRE_H
