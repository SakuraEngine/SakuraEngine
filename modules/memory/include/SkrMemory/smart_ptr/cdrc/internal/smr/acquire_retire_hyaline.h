
#ifndef CDRC_SMR_ACQUIRE_RETIRE_HYALINE_H
#define CDRC_SMR_ACQUIRE_RETIRE_HYALINE_H

#include <cassert>
#include <cstdint>

#include <atomic>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "../counted_object.h"
#include "../memory_manager_base.h"
#include "../utils.h"

namespace cdrc {

namespace internal {

struct hyaline_tracker {

  struct alignas(128) Reservation;

  struct Node { // TODO: unclear if this should be aligned
    void* obj;
    union {      // Each node takes 3 memory words
      std::atomic<int64_t> refc;  // REFS: Refer. counter
      Node* bnext;    // SLOT: Next node
    };
    union {
      Node* next;           // SLOT: After retiring
      std::function<void(void*)>* decrement;
    };
    Node* blink;    // REFS: First SLOT node,

    Node(void* obj) : obj(obj), bnext(nullptr), next(nullptr), blink(nullptr) {}
  };

  inline static Node* invptr = reinterpret_cast<Node*>(0x1);
  inline static const int64_t REFC_PROTECT = (1ull << 62);

  struct alignas(128) Batch {
    Node* first;
    Node* refs;
    size_t counter;
    Batch() : first(nullptr), refs(nullptr), counter(0) {}
  };

  struct alignas(128) Reservation {
    std::atomic<Node*> list;
    Reservation() : list(invptr) {}
  };

  static hyaline_tracker &instance() {
    static hyaline_tracker tracker;
    return tracker;
  }

  bool begin_critical_section() {
    auto id = utils::ThreadID::GetThreadID().getTID();
    if (critical_section[id]) {
      return false;
    } else {
      rsrv[id].list.exchange(nullptr);
      critical_section[id] = true;
      return true;
    }
  }

  void end_critical_section() {
    auto id = utils::ThreadID::GetThreadID().getTID();
    assert(critical_section[id]);
    Node* p = rsrv[id].list.exchange(invptr);
    assert(p != invptr);
    traverse(p);
    critical_section[id] = false;
  }

  bool in_critical_section() {
    auto id = utils::ThreadID::GetThreadID().getTID();
    return critical_section[id];
  }

  void add_batch(const Batch& batch) {
    batch.refs->blink = batch.first;
    Node* curr = batch.first;
    int64_t cnt = -REFC_PROTECT;
    for(size_t i = 0; i < utils::num_threads(); i++) {
      while(true) {
        Node* prev = rsrv[i].list.load();
        if(prev == invptr) break;
        curr->next = prev;
        if(rsrv[i].list.compare_exchange_strong(prev, curr)) {
          cnt++;
          break;
        }
      }
      curr = curr->bnext;
    }
    if(batch.refs->refc.fetch_add(cnt) == -cnt) // Finish
      free_batch(batch.refs); // retiring: change refc // TODO: fix recursion problem
  }

private:
  void traverse(Node* next) {
    while(next != nullptr) {
      Node* curr = next;
      assert(curr != invptr);
      next = curr->next;
      Node* refs = curr->blink;
      if(refs->refc.fetch_add(-1) == 1) free_batch(refs);
    }
  }

  void free_batch(Node* refs) {
    std::function<void(void*)>& decrement = *(refs->decrement);
    Node* n = refs->blink;
    do {
      Node* node = n;
      // refc and bnext overlap and are 0
      // (nullptr) for the last REFS node
      assert(n != invptr);
      n = n->bnext;
      decrement(node->obj);
      delete node;
    } while(n != nullptr);
  }

public:
  hyaline_tracker() : critical_section(utils::num_threads()),
                      rsrv(utils::num_threads()) {}

  std::vector<utils::Padded<bool>> critical_section;
  std::vector<Reservation> rsrv;
};

}  // namespace internal

struct hyaline_guard {
  hyaline_guard() : engaged(internal::hyaline_tracker::instance().begin_critical_section()) {}

  ~hyaline_guard() {
    if (engaged) { internal::hyaline_tracker::instance().end_critical_section(); }
  }

  hyaline_guard(const hyaline_guard &) = delete;

  hyaline_guard(hyaline_guard &&) = delete;

  hyaline_guard &operator=(const hyaline_guard &) = delete;

  hyaline_guard &operator=(hyaline_guard &&) = delete;

private:
  bool engaged;
};

template<typename F>
std::invoke_result_t<F> with_hyaline_guard(F&& f) {
  hyaline_guard g;
  return std::invoke(std::forward<F>(f));
}

namespace internal {

// An interface for safe memory reclamation that protects reference-counted
// resources by deferring their reference count decrements until no thread
// is still reading them.
//
// This implementation uses Hyaline, in which the user is
// responsible for acquiring a guard before performing any reads or writes
// to the shared pointer. The guard is acquired by holding a local instance
// of a "hyaline_guard", like so
//
//   {
//     cdrc::hyaline_guard g;
//     // critical code
//   }
//
// T =              The underlying type of the object being protected
// batch_size       accumulate (batch_size*num_threads)+1 nodes before announcing batch
//
// NOTE: handling recursion was tricky
// Note: Hyaline doesn't suffer as much from the recursive destruct problem. No maybe it
// still does because batching. But it might be easier to fix.
template<typename T, size_t batch_size = 2>
struct acquire_retire_hyaline : public memory_manager_base<T, acquire_retire_hyaline<T>> {

  using base = memory_manager_base<T, acquire_retire_hyaline<T>>;

  using base::increment_allocations;
  using base::decrement_allocations;

  using Node = hyaline_tracker::Node;
  using Batch = hyaline_tracker::Batch;

private:
  using counted_object_t = counted_object<T>;
  using counted_ptr_t = std::add_pointer_t<counted_object_t>;

public:

  static acquire_retire_hyaline& instance() {
    static acquire_retire_hyaline ar{utils::num_threads()};
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

  template<typename U>
  using acquired_pointer = basic_acquired_pointer<U>;

  explicit acquire_retire_hyaline(size_t num_threads_) :
    base(num_threads_),
    num_threads(num_threads_),
    local_batch(num_threads),
    in_progress(num_threads, false)
    {
      hyaline_tracker::instance();  // touch the tracker to force it to initialize before this object,
                                    // otherwise, destruction order may be wrong since acquire_retire_hyaline
                                    // refers to hyaline_tracker in its destructor, and hence hyaline tracker
                                    // MUST be destructed after (and hence constructed before!)

      strong_eject = [this](void* obj) {
        this->eject(reinterpret_cast<counted_ptr_t>(obj), RetireType::decrement_strong_count);
      };
      weak_eject = [this](void* obj) {
        this->eject(reinterpret_cast<counted_ptr_t>(obj), RetireType::decrement_weak_count);
      };
      dispose_eject = [this](void* obj) {
        this->eject(reinterpret_cast<counted_ptr_t>(obj), RetireType::dispose);
      };
    }

  template<typename U>
  [[nodiscard]] acquired_pointer<U> acquire(const std::atomic<U> *p) {
    return acquired_pointer<U>(p->load(std::memory_order_acquire));
  }

  // Like acquire, but assuming that the caller already has a
  // copy of the handle and knows that it is protected
  template<typename U>
  [[nodiscard]] acquired_pointer<U> reserve(U p) {
    return acquired_pointer<U>(p);
  }

  // Dummy function for when we need to conditionally reserve
  // something, but might need to reserve nothing
  template<typename U>
  [[nodiscard]] acquired_pointer<U> reserve_nothing() const {
    return acquired_pointer<U>();
  }

  template<typename U>
  [[nodiscard]] acquired_pointer<U> protect_snapshot(const std::atomic<U> *p) {
    auto ptr = p->load(std::memory_order_acquire);
    if (ptr != nullptr && ptr->get_use_count() == 0) ptr = nullptr;
    return {ptr};
  }

  void release() {}

  void retire(counted_ptr_t p, RetireType type) {
    auto id = utils::ThreadID::GetThreadID().getTID();
    if(p == nullptr) {return;}

    Batch& batch = local_batch[id];
    Node* node = new Node(reinterpret_cast<void*>(p));
    if(!batch.first) { // the REFS node
      batch.refs = node;
      node->refc.store(hyaline_tracker::REFC_PROTECT, std::memory_order_release);
      if (type == RetireType::decrement_strong_count) node->decrement = &strong_eject;
      else if (type == RetireType::decrement_weak_count) node->decrement = &weak_eject;
      else node->decrement = &dispose_eject;
    } else { // SLOT nodes
      node->blink = batch.refs; // points to REFS
      node->bnext = batch.first;
    }
    batch.first = node;
    batch.counter++;
    // Must have MAX_THREADS+1 nodes to insert to
    // MAX_THREADS lists, exit if do not have enough
    while(!in_progress[id] && batch.counter > batch_size*num_threads) {
      const Batch batch_copy = batch;
      batch.first = nullptr;
      batch.counter = 0;
      // if(in_progress) std::cout << "recursive call to retire" << std::endl;
      in_progress[id] = true;
      hyaline_tracker::instance().add_batch(batch_copy);
      in_progress[id] = false;
    }
  }

  // Perform any remaining deferred destruction. Need to be very careful
  // about additional objects being queued for deferred destruction by
  // an object that was just destructed.
  ~acquire_retire_hyaline() {
    auto id = utils::ThreadID::GetThreadID().getTID();
    do {
      for (size_t i = 0; i < num_threads; i++) {
        assert(hyaline_tracker::instance().rsrv[i].list.load() == hyaline_tracker::invptr);
        while(local_batch[i].first != nullptr) {
          Batch& batch = local_batch[i];
          const Batch batch_copy = batch;
          batch.first = nullptr;
          batch.counter = 0;
          Node* node = batch_copy.first;
          while(node != nullptr) {
            Node* next = node->bnext;
            void* obj = node->obj;
            in_progress[id] = true;
            (*batch_copy.refs->decrement)(obj);
            in_progress[id] = false;
            delete node;
            if(node == batch_copy.refs) break;
            node = next;
          }
        }
      }
    } while(local_batch[id].first != nullptr);
  }

private:
  alignas(128) size_t num_threads;
  alignas(128) std::vector<Batch> local_batch;
  alignas(128) std::function<void(void*)> strong_eject, weak_eject, dispose_eject;
  std::vector<AlignedBool> in_progress;                         // Local flags to prevent reentrancy while destructing
};


}  // namespace internal

}  // namespace cdrc

#endif // CDRC_SMR_ACQUIRE_RETIRE_HYALINE_H
