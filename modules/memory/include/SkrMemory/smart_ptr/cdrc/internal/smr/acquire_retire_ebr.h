
#ifndef CDRC_SMR_ACQUIRE_RETIRE_EBR_H
#define CDRC_SMR_ACQUIRE_RETIRE_EBR_H

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <atomic>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "../counted_object.h"
#include "../epoch_tracker.h"
#include "../memory_manager_base.h"
#include "../utils.h"

namespace cdrc {

namespace internal {

// An interface for safe memory reclamation that protects reference-counted
// resources by deferring their reference count decrements until no thread
// is still reading them.
//
// This implementation uses epoch-based reclamation, in which the user is
// responsible for acquiring a guard before performing any reads or writes
// to the shared pointer. The guard is acquired by holding a local instance
// of an "epoch_guard", like so
//
//   {
//     cdrc::epoch_guard g;
//     // critical code
//   }
//
// T =               The underlying type of the object being protected
// epoch_frequency = How often to update the global epoch. More often (lower value)
//                   will reduce performance but decrease memory usage.
// eject_delay =     The maximum number of deferred ejects that will be held by
//                   any one worker thread is at most eject_delay * #threads.
//
template<typename T, size_t epoch_frequency = 10, size_t eject_delay = 2>
struct acquire_retire_ebr : public memory_manager_base<T, acquire_retire_ebr<T>> {

  using base = memory_manager_base<T, acquire_retire_ebr<T>>;

  using base::increment_allocations;
  using base::decrement_allocations;
  using base::increment_ref_cnt;
  using base::eject;
  using base::decrement_weak_cnt;

private:
  using counted_object_t = counted_object<T>;
  using counted_ptr_t = std::add_pointer_t<counted_object_t>;

public:

  static acquire_retire_ebr& instance() {
    static acquire_retire_ebr ar{utils::num_threads()};
    return ar;
  }

  template<typename... Args>
  counted_ptr_t create_object(Args &&... args) {
    increment_allocations();
    work_toward_advancing_epoch(1);
    return new counted_object_t(std::forward<Args>(args)...);
  }

  void delete_object(counted_ptr_t p) {
    delete p;
    decrement_allocations();
  }

  struct RetiredObj {
    counted_ptr_t obj; uint64_t retireTS; RetireType type;
    RetiredObj(counted_ptr_t obj, uint64_t ts, RetireType type_) : obj(obj), retireTS(ts), type(type_) {}
  };

  template<typename U>
  using acquired_pointer = basic_acquired_pointer<U>;

  explicit acquire_retire_ebr(size_t num_threads_) :
    base(num_threads_),
    num_threads(num_threads_),
    in_progress(num_threads),
    deferred_destructs(num_threads),
    eject_work(num_threads),
    epoch_work(num_threads) {}

  template<typename U>
  [[nodiscard]] acquired_pointer<U> acquire(const std::atomic<U> *p) {
    return {p->load(std::memory_order_acquire)};
  }

  // Like acquire, but assuming that the caller already has a
  // copy of the handle and knows that it is protected
  template<typename U>
  [[nodiscard]] acquired_pointer<U> reserve(U p) {
    return {p};
  }

  // Dummy function for when we need to conditionally reserve
  // something, but might need to reserve nothing
  template<typename U>
  [[nodiscard]] acquired_pointer<U> reserve_nothing() const {
    return {};
  }

  template<typename U>
  [[nodiscard]] acquired_pointer<U> protect_snapshot(const std::atomic<U> *p) {
    auto ptr = p->load(std::memory_order_acquire);
    if (ptr != nullptr && ptr->get_use_count() == 0) ptr = nullptr;
    return {ptr};
  }

  void release() { }

  void retire(counted_ptr_t p, RetireType type) {
    auto id = utils::ThreadID::GetThreadID().getTID();
    deferred_destructs[id].emplace_back(p, epoch_tracker::instance().get_current_epoch(), type);
    work_toward_ejects(1);
  }

  // Perform any remaining deferred destruction. Need to be very careful
  // about additional objects being queued for deferred destruction by
  // an object that was just destructed.
  ~acquire_retire_ebr() {
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
        for (const auto& x : v) {
          destructs.emplace_back(x.obj, x.type);
        }
        v.clear();
      }

      // Perform all of the pending deferred ejects
      for (auto [x,type] : destructs) {
        eject(x,type);
      }
    }
  }

private:

  void work_toward_advancing_epoch(size_t work = 1) {
    auto id = utils::ThreadID::GetThreadID().getTID();
    epoch_work[id] = epoch_work[id] + work;
    if(epoch_work[id] >= epoch_frequency * num_threads) {
      epoch_work[id] = 0;
      epoch_tracker::instance().advance_global_epoch();
    }
  }

  void work_toward_ejects(size_t work = 1) {
    auto id = utils::ThreadID::GetThreadID().getTID();
    eject_work[id] = eject_work[id] + work;
    auto threshold = std::max<size_t>(30, eject_delay * num_threads);  // Always attempt at least 30 ejects
    while (!in_progress[id] && eject_work[id] >= threshold) {
      eject_work[id] = 0;
      if (deferred_destructs[id].size() == 0) break; // nothing to collect
      in_progress[id] = true;
      auto deferred = std::vector<RetiredObj>(std::move(deferred_destructs[id]));
      auto min_epoch = epoch_tracker::instance().get_min_announced_epoch();

      auto f = [this, min_epoch](const auto& x) {
        if (x.retireTS < min_epoch) {
          eject(x.obj, x.type);
          return true;
        }
        return false;
      };

      // Remove the deferred decrements that are successfully applied
      deferred.erase(remove_if(deferred.begin(), deferred.end(), f), deferred.end());
      deferred_destructs[id].insert(deferred_destructs[id].end(), deferred.begin(), deferred.end());
      in_progress[id] = false;
    }
  }

  size_t num_threads;
  std::vector<AlignedBool> in_progress;                         // Local flags to prevent reentrancy while destructing
  std::vector<AlignedVector<RetiredObj>> deferred_destructs;    // Thread-local lists of pending deferred destructs
  std::vector<AlignedInt> eject_work;                           // Amortized work to pay for ejecting deferred destructs
  std::vector<AlignedInt> epoch_work;                           // Amortized work to pay for incrementing the epoch
};


}  // namespace internal

}  // namespace cdrc

#endif // CDRC_SMR_ACQUIRE_RETIRE_EBR_H
