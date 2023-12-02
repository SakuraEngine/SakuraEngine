#ifndef CDRC_SMR_ACQUIRE_RETIRE_IBR_H
#define CDRC_SMR_ACQUIRE_RETIRE_IBR_H

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
// This implementation uses interval-based reclamation, in which the user is
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
template<typename T, size_t epoch_frequency = 40, size_t eject_delay = 2>
struct acquire_retire_ibr : public memory_manager_base<T, acquire_retire_ibr<T, eject_delay>> {

  using base = memory_manager_base<T, acquire_retire_ibr<T, eject_delay>>;

  using base::increment_allocations;
  using base::decrement_allocations;
  using base::increment_ref_cnt;
  using base::eject;

  inline static const uint64_t INVALID_TS = 0;

 private:
  using counted_object_t = counted_object<T>;
  using counted_ptr_t = std::add_pointer_t<counted_object_t>;

  // Align to cache line boundary to avoid false sharing
  struct alignas(64) LocalSlot {
    std::atomic<uint64_t> endTS_ann;

    LocalSlot() : endTS_ann(INVALID_TS) {}
  };

 public:

  static acquire_retire_ibr& instance() {
    static acquire_retire_ibr ar{utils::num_threads()};
    return ar;
  }

  // Augments a reference-counted object with a birth timestamp field that is
  // initialized with the value of the current epoch when the object is created
  struct stamped_counted_object : public counted_object<T> {
    template<typename... Args>
    explicit stamped_counted_object(uint64_t t, Args&&... args)
      : counted_object<T>(std::forward<Args>(args)...), birthTS(t) {}
    uint64_t birthTS;
  };

  uint64_t get_birth_timestamp(counted_ptr_t p) {
    return static_cast<stamped_counted_object*>(p)->birthTS;
  }

  template<typename... Args>
  counted_ptr_t create_object(Args &&... args) {
    increment_allocations();
    work_toward_advancing_epoch(1);
    return new stamped_counted_object(epoch_tracker::instance().get_current_epoch(), std::forward<Args>(args)...);
  }

  void delete_object(counted_ptr_t p) {
    delete static_cast<stamped_counted_object*>(p);
    decrement_allocations();
  }

  struct RetiredObj {
    counted_ptr_t obj; uint64_t birthTS; uint64_t retireTS; RetireType type;
    RetiredObj(counted_ptr_t obj, uint64_t birthTS, uint64_t retireTS, RetireType type) :
        obj(obj), birthTS(birthTS), retireTS(retireTS), type(type) {}
  };

  template<typename U>
  using acquired_pointer = basic_acquired_pointer<U>;


  explicit acquire_retire_ibr(size_t num_threads_) :
      base(num_threads_),
      num_threads(num_threads_),
      announcement_slots(num_threads),
      in_progress(num_threads),
      deferred_destructs(num_threads),
      eject_work(num_threads),
      epoch_work(num_threads) {}

  template<typename U>
  [[nodiscard]] acquired_pointer<U> acquire(const std::atomic<U> *p) {
    return protect_snapshot(p);
  }

  // Like acquire, but assuming that the caller already has a
  // copy of the handle and knows that it is protected
  template<typename U>
  [[nodiscard]] acquired_pointer<U> reserve(U p) {
    //if(p == nullptr) return {p, nullptr};
    auto id = utils::ThreadID::GetThreadID().getTID();
    LocalSlot& slot = announcement_slots[id];
    uint64_t curTS = epoch_tracker::instance().get_current_epoch();
    if(slot.endTS_ann.load() != curTS)
      slot.endTS_ann.store(curTS, std::memory_order_release);
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
    auto id = utils::ThreadID::GetThreadID().tid;
    auto p_epoch = announcement_slots[id].endTS_ann.load();
    while(true) {
      U result = p->load(std::memory_order_seq_cst);
      uint64_t curTS = epoch_tracker::instance().get_current_epoch();
      if(p_epoch == curTS) {
        if (result != nullptr && result->get_use_count() == 0) return {nullptr};
        return {result};
      }
      else {
        announcement_slots[id].endTS_ann.exchange(curTS);
        p_epoch = curTS;
      }
    }
  }

  void release() {}

  void retire(counted_ptr_t p, RetireType type) {
    auto id = utils::ThreadID::GetThreadID().getTID();
    deferred_destructs[id].push_back(RetiredObj(p, get_birth_timestamp(p), epoch_tracker::instance().get_current_epoch(), type));
    work_toward_ejects(1);
  }

  // Perform any remaining deferred destruction. Need to be very careful
  // about additional objects being queued for deferred destruction by
  // an object that was just destructed.
  ~acquire_retire_ibr() {
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
    auto threshold = std::max<size_t>(30, eject_delay * eject_work.size());  // Always attempt at least 30 ejects
    while (!in_progress[id] && eject_work[id] >= threshold) {
      eject_work[id] = 0;
      if (deferred_destructs[id].size() == 0) break; // nothing to collect
      in_progress[id] = true;
      auto deferred = AlignedVector<RetiredObj>(std::move(deferred_destructs[id]));

      std::vector<std::pair<uint64_t, uint64_t>> announced;
      epoch_tracker::instance().scan_announced_epochs([&](auto index, auto startTS) { 
        uint64_t endTS = announcement_slots[index].endTS_ann.load();
        if(startTS <= endTS) announced.push_back(std::make_pair(startTS, endTS)); 
      });

      auto f = [&, this](auto x) {
        bool reserved = false;
        for(auto& ann : announced) {
          if(x.retireTS < ann.first || x.birthTS > ann.second) continue;
          reserved = true;
          break;
        }

        if (!reserved) {
          eject(x.obj, x.type);
          return true;
        } else {
          return false;
        }
      };

      // Remove the deferred decrements that are successfully applied
      deferred.erase(remove_if(deferred.begin(), deferred.end(), f), deferred.end());
      deferred_destructs[id].insert(deferred_destructs[id].end(), deferred.begin(), deferred.end());
      in_progress[id] = false;
    }
  }

  size_t num_threads;
  std::vector<LocalSlot> announcement_slots;                      // Announcement array slots
  std::vector<AlignedBool> in_progress;                           // Local flags to prevent reentrancy while destructing
  std::vector<AlignedVector<RetiredObj>> deferred_destructs;      // Thread-local lists of pending deferred destructs
  std::vector<AlignedInt> eject_work;                             // Amortized work to pay for ejecting deferred destructs
  std::vector<AlignedInt> epoch_work;                             // Amortized work to pay for incrementing the epoch
};


}  // namespace internal

}  // namespace cdrc

#endif  // CDRC_SMR_ACQUIRE_RETIRE_IBR_H
