/* Signal guard support
(C) 2018-2021 Niall Douglas <http://www.nedproductions.biz/> (4 commits)
File Created: June 2018


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License in the accompanying file
Licence.txt or at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.


Distributed under the Boost Software License, Version 1.0.
    (See accompanying file Licence.txt or copy at
          http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef QUICKCPPLIB_SIGNAL_GUARD_HPP
#define QUICKCPPLIB_SIGNAL_GUARD_HPP

#include "config.hpp"

#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>

#ifdef _WIN32
#ifndef SIGBUS
#define SIGBUS (7)
#endif
#ifndef SIGPIPE
#define SIGPIPE (13)
#endif
struct sigset_t
{
  unsigned mask;
};
#define sigismember(s, signo) (((s)->mask & (1ULL << signo)) != 0)

#ifdef _MSC_VER
extern "C" unsigned long __cdecl _exception_code(void);
extern "C" void *__cdecl _exception_info(void);
#endif
#endif

#if defined(__cplusplus)
#include "bitfield.hpp"

#include <atomic>
#include <cassert>
#include <exception>
#include <new>  // for placement new
#endif

#ifdef QUICKCPPLIB_EXPORTS
#define SIGNALGUARD_CLASS_DECL QUICKCPPLIB_SYMBOL_EXPORT
#define SIGNALGUARD_FUNC_DECL extern QUICKCPPLIB_SYMBOL_EXPORT
#define SIGNALGUARD_MEMFUNC_DECL
#else
#if defined(__cplusplus) && (!defined(QUICKCPPLIB_HEADERS_ONLY) || QUICKCPPLIB_HEADERS_ONLY == 1) && !defined(DOXYGEN_SHOULD_SKIP_THIS)
#define SIGNALGUARD_CLASS_DECL
#define SIGNALGUARD_FUNC_DECL extern inline
#define SIGNALGUARD_MEMFUNC_DECL inline
#elif defined(QUICKCPPLIB_DYN_LINK) && !defined(QUICKCPPLIB_STATIC_LINK)
#define SIGNALGUARD_CLASS_DECL QUICKCPPLIB_SYMBOL_IMPORT
#define SIGNALGUARD_FUNC_DECL extern QUICKCPPLIB_SYMBOL_IMPORT
#define SIGNALGUARD_MEMFUNC_DECL
#else
#define SIGNALGUARD_CLASS_DECL
#define SIGNALGUARD_FUNC_DECL extern
#define SIGNALGUARD_MEMFUNC_DECL
#endif
#endif

#if defined(__cplusplus)
extern "C"
{
#endif

  /*! \union raised_signal_info_value
  \brief User defined value.
  */
  union raised_signal_info_value
  {
    int int_value;
    void *ptr_value;
#if defined(__cplusplus)
    raised_signal_info_value() = default;
    raised_signal_info_value(int v)
        : int_value(v)
    {
    }
    raised_signal_info_value(void *v)
        : ptr_value(v)
    {
    }
#endif
  };
#if defined(__cplusplus)
  static_assert(std::is_trivial<raised_signal_info_value>::value, "raised_signal_info_value is not trivial!");
  static_assert(std::is_trivially_copyable<raised_signal_info_value>::value, "raised_signal_info_value is not trivially copyable!");
  static_assert(std::is_standard_layout<raised_signal_info_value>::value, "raised_signal_info_value does not have standard layout!");
#endif

  //! Typedef to a system specific error code type
#ifdef _WIN32
  typedef long raised_signal_error_code_t;
#else
typedef int raised_signal_error_code_t;
#endif

  /*! \struct raised_signal_info
  \brief A platform independent subset of `siginfo_t`.
  */
  struct raised_signal_info
  {
    jmp_buf buf;  //!< setjmp() buffer written on entry to guarded section
    int signo;    //!< The signal raised

    //! The system specific error code for this signal, the `si_errno` code (POSIX) or `NTSTATUS` code (Windows)
    raised_signal_error_code_t error_code;
    void *addr;                            //!< Memory location which caused fault, if appropriate
    union raised_signal_info_value value;  //!< A user-defined value

    //! The OS specific `siginfo_t *` (POSIX) or `PEXCEPTION_RECORD` (Windows)
    void *raw_info;
    //! The OS specific `ucontext_t` (POSIX) or `PCONTEXT` (Windows)
    void *raw_context;
  };

  //! \brief The type of the guarded function.
  typedef union raised_signal_info_value (*thrd_signal_guard_guarded_t)(union raised_signal_info_value);

  //! \brief The type of the function called to recover from a signal being raised in a guarded section.
  typedef union raised_signal_info_value (*thrd_signal_guard_recover_t)(const struct raised_signal_info *);

  //! \brief The type of the function called when a signal is raised. Returns true to continue guarded code, false to recover.
  typedef bool (*thrd_signal_guard_decide_t)(struct raised_signal_info *);

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4190)  // C-linkage with UDTs
#endif
  /*! \brief Installs a thread-local signal guard for the calling thread, and calls the guarded function `guarded`.
  \return The value returned by `guarded`, or `recovery`.
  \param signals The set of signals to guard against.
  \param guarded A function whose execution is to be guarded against signal raises.
  \param recovery A function to be called if a signal is raised.
  \param decider A function to be called to decide whether to recover from the signal and continue
  the execution of the guarded routine, or to abort and call the recovery routine.
  \param value A value to supply to the guarded routine.
   */
  SIGNALGUARD_FUNC_DECL union raised_signal_info_value thrd_signal_guard_call(const sigset_t *signals, thrd_signal_guard_guarded_t guarded,
                                                                              thrd_signal_guard_recover_t recovery, thrd_signal_guard_decide_t decider,
                                                                              union raised_signal_info_value value);
#ifdef _MSC_VER
#pragma warning(pop)
#endif

  /*! \brief Call the currently installed signal handler for a signal (POSIX), or raise a Win32 structured
  exception (Windows), returning false if no handler was called due to the currently
  installed handler being `SIG_IGN` (POSIX).

  Note that on POSIX, we fetch the currently installed signal handler and try to call it directly.
  This allows us to supply custom `raw_info` and `raw_context`, and we do all the things which the signal
  handler flags tell us to do beforehand [1]. If the current handler has been defaulted, we
  enable the signal and execute `pthread_kill(pthread_self(), signo)` in order to invoke the
  default handling.

  Note that on Windows, `raw_context` is ignored as there is no way to override the context thrown
  with a Win32 structured exception.

  [1]: We currently do not implement alternative stack switching. If a handler requests that, we
  simply abort the process. Code donations implementing support are welcome.
   */
  SIGNALGUARD_FUNC_DECL bool thrd_raise_signal(int signo, void *raw_info, void *raw_context);

  /*! \brief On platforms where it is necessary (POSIX), installs, and potentially enables,
  the global signal handlers for the signals specified by `guarded`. Each signal installed
  is threadsafe reference counted, so this is safe to call from multiple threads or instantiate
  multiple times.

  On platforms with better than POSIX global signal support, this function does nothing.

  ## POSIX only

  Any existing global signal handlers are replaced with a filtering signal handler, which
  checks if the current kernel thread has installed a signal guard, and if so executes the
  guard. If no signal guard has been installed for the current kernel thread, global signal
  continuation handlers are executed. If none claims the signal, the previously
  installed signal handler is called.

  After the new signal handlers have been installed, the guarded signals are globally enabled
  for all threads of execution. Be aware that the handlers are installed with `SA_NODEFER`
  to avoid the need to perform an expensive syscall when a signal is handled.
  However this may also produce surprise e.g. infinite loops.

  \warning This class is threadsafe with respect to other concurrent executions of itself,
  but is NOT threadsafe with respect to other code modifying the global signal handlers.
  */
  SIGNALGUARD_FUNC_DECL void *signal_guard_create(const sigset_t *guarded);
  /*! \brief Uninstall a previously installed signal guard.
   */
  SIGNALGUARD_FUNC_DECL bool signal_guard_destroy(void *i);

  /*! \brief Create a global signal continuation decider. Threadsafe with respect to
  other calls of this function, but not reentrant i.e. modifying the global signal continuation
  decider registry whilst inside a global signal continuation decider is racy. Called after
  all thread local handling is exhausted. Note that what you can safely do in the decider
  function is extremely limited, only async signal safe functions may be called.
  \return An opaque pointer to the registered decider. `NULL` if `malloc` failed.
  \param guarded The set of signals to be guarded against.
  \param callfirst True if this decider should be called before any other. Otherwise
  call order is in the order of addition.
  \param decider A decider function, which must return `true` if execution is to resume,
  `false` if the next decider function should be called.
  \param value A user supplied value to set in the `raised_signal_info` passed to the
  decider callback.
  */
  SIGNALGUARD_FUNC_DECL void *signal_guard_decider_create(const sigset_t *guarded, bool callfirst, thrd_signal_guard_decide_t decider,
                                                          union raised_signal_info_value value);
  /*! \brief Destroy a global signal continuation decider. Threadsafe with
  respect to other calls of this function, but not reentrant i.e. do not call
  whilst inside a global signal continuation decider.
  \return True if recognised and thus removed.
  */
  SIGNALGUARD_FUNC_DECL bool signal_guard_decider_destroy(void *decider);


#if defined(__cplusplus)
}
#endif


/**************************************** The C++ API ***************************************/

#if defined(__cplusplus)
static_assert(std::is_trivial<raised_signal_info>::value, "raised_signal_info is not trivial!");
static_assert(std::is_trivially_copyable<raised_signal_info>::value, "raised_signal_info is not trivially copyable!");
static_assert(std::is_standard_layout<raised_signal_info>::value, "raised_signal_info does not have standard layout!");

QUICKCPPLIB_NAMESPACE_BEGIN

//! \brief The namespace for signal_guard
namespace signal_guard
{
  using raised_signal_info = ::raised_signal_info;

  //! \brief The signals which are supported
  enum class signalc
  {
    none = 0,

    abort_process = SIGABRT,           //!< The process is aborting (`SIGABRT`)
    undefined_memory_access = SIGBUS,  //!< Attempt to access a memory location which can't exist (`SIGBUS`)
    illegal_instruction = SIGILL,      //!< Execution of illegal instruction (`SIGILL`)
    interrupt =
    SIGINT,  //!< The process is interrupted (`SIGINT`). Note on Windows the continuation decider is ALWAYS called from a separate thread, and the process exits after you return (or take too long executing).
    broken_pipe = SIGPIPE,          //!< Reader on a pipe vanished (`SIGPIPE`). Note that Windows never raises this signal.
    segmentation_fault = SIGSEGV,   //!< Attempt to access a memory page whose permissions disallow (`SIGSEGV`)
    floating_point_error = SIGFPE,  //!< Floating point error (`SIGFPE`)
    process_terminate =
    SIGTERM,  //!< Process termination requested (`SIGTERM`). Note on Windows the handler is ALWAYS called from a separate thread, and the process exits after you return (or take too long executing).

#ifndef _WIN32
    timer_expire = SIGALRM,      //!< Timer has expired (`SIGALRM`). POSIX only.
    child_exit = SIGCHLD,        //!< Child has exited (`SIGCHLD`). POSIX only.
    process_continue = SIGCONT,  //!< Process is being continued (`SIGCONT`). POSIX only.
    tty_hangup = SIGHUP,         //!< Controlling terminal has hung up (`SIGHUP`). POSIX only.
    process_kill = SIGKILL,      //!< Process has received kill signal (`SIGKILL`). POSIX only.
#ifdef SIGPOLL
    pollable_event = SIGPOLL,  //!< i/o is now possible (`SIGPOLL`). POSIX only.
#endif
    profile_event = SIGPROF,             //!< Profiling timer expired (`SIGPROF`). POSIX only.
    process_quit = SIGQUIT,              //!< Process is being quit (`SIGQUIT`). POSIX only.
    process_stop = SIGSTOP,              //!< Process is being stopped (`SIGSTOP`). POSIX only.
    tty_stop = SIGTSTP,                  //!< Terminal requests stop (`SIGTSTP`). POSIX only.
    bad_system_call = SIGSYS,            //!< Bad system call (`SIGSYS`). POSIX only.
    process_trap = SIGTRAP,              //!< Process has reached a breakpoint (`SIGTRAP`). POSIX only.
    tty_input = SIGTTIN,                 //!< Terminal has input (`SIGTTIN`). POSIX only.
    tty_output = SIGTTOU,                //!< Terminal ready for output (`SIGTTOU`). POSIX only.
    urgent_condition = SIGURG,           //!< Urgent condition (`SIGURG`). POSIX only.
    user_defined1 = SIGUSR1,             //!< User defined 1 (`SIGUSR1`). POSIX only.
    user_defined2 = SIGUSR2,             //!< User defined 2 (`SIGUSR2`). POSIX only.
    virtual_alarm_clock = SIGVTALRM,     //!< Virtual alarm clock (`SIGVTALRM`). POSIX only.
    cpu_time_limit_exceeded = SIGXCPU,   //!< CPU time limit exceeded (`SIGXCPU`). POSIX only.
    file_size_limit_exceeded = SIGXFSZ,  //!< File size limit exceeded (`SIGXFSZ`). POSIX only.
#endif

    /* C++ handlers
    On all the systems I examined, all signal numbers are <= 30 in order to fit inside a sigset_t.
    Note that apparently IBM uses the full 32 bit range in its signal numbers.
    */
    cxx_out_of_memory = 32,  //!< A call to operator new failed, and a throw is about to occur
    cxx_termination = 33,    //!< A call to std::terminate() was made. NOT `SIGTERM`.

    _max_value
  };

  //! \brief Bitfield for the signals which are supported
  QUICKCPPLIB_BITFIELD_BEGIN_T(signalc_set, uint64_t){
  none = 0,

  abort_process = (1ULL << static_cast<int>(signalc::abort_process)),                      //!< The process is aborting (`SIGABRT`)
  undefined_memory_access = (1ULL << static_cast<int>(signalc::undefined_memory_access)),  //!< Attempt to access a memory location which can't exist (`SIGBUS`)
  illegal_instruction = (1ULL << static_cast<int>(signalc::illegal_instruction)),          //!< Execution of illegal instruction (`SIGILL`)
  interrupt =
  (1ULL << static_cast<int>(
   signalc::
   interrupt)),  //!< The process is interrupted (`SIGINT`). Note on Windows the handler is ALWAYS called from a separate thread, and the process exits after you return (or take too long executing).
  broken_pipe = (1ULL << static_cast<int>(signalc::broken_pipe)),  //!< Reader on a pipe vanished (`SIGPIPE`). Note that Windows never raises this signal.
  segmentation_fault = (1ULL << static_cast<int>(signalc::segmentation_fault)),      //!< Attempt to access a memory page whose permissions disallow (`SIGSEGV`)
  floating_point_error = (1ULL << static_cast<int>(signalc::floating_point_error)),  //!< Floating point error (`SIGFPE`)
  process_terminate =
  (1ULL << static_cast<int>(
   signalc::
   process_terminate)),  //!< Process termination requested (`SIGTERM`). Note on Windows the handler is ALWAYS called from a separate thread, and the process exits after you return (or take too long executing).


#ifndef _WIN32
  timer_expire = (1ULL << static_cast<int>(signalc::timer_expire)),          //!< Timer has expired (`SIGALRM`). POSIX only.
  child_exit = (1ULL << static_cast<int>(signalc::child_exit)),              //!< Child has exited (`SIGCHLD`). POSIX only.
  process_continue = (1ULL << static_cast<int>(signalc::process_continue)),  //!< Process is being continued (`SIGCONT`). POSIX only.
  tty_hangup = (1ULL << static_cast<int>(signalc::tty_hangup)),              //!< Controlling terminal has hung up (`SIGHUP`). POSIX only.
  process_kill = (1ULL << static_cast<int>(signalc::process_kill)),          //!< Process has received kill signal (`SIGKILL`). POSIX only.
#ifdef SIGPOLL
  pollable_event = (1ULL << static_cast<int>(signalc::pollable_event)),  //!< i/o is now possible (`SIGPOLL`). POSIX only.
#endif
  profile_event = (1ULL << static_cast<int>(signalc::profile_event)),                        //!< Profiling timer expired (`SIGPROF`). POSIX only.
  process_quit = (1ULL << static_cast<int>(signalc::process_quit)),                          //!< Process is being quit (`SIGQUIT`). POSIX only.
  process_stop = (1ULL << static_cast<int>(signalc::process_stop)),                          //!< Process is being stopped (`SIGSTOP`). POSIX only.
  tty_stop = (1ULL << static_cast<int>(signalc::tty_stop)),                                  //!< Terminal requests stop (`SIGTSTP`). POSIX only.
  bad_system_call = (1ULL << static_cast<int>(signalc::bad_system_call)),                    //!< Bad system call (`SIGSYS`). POSIX only.
  process_trap = (1ULL << static_cast<int>(signalc::process_trap)),                          //!< Process has reached a breakpoint (`SIGTRAP`). POSIX only.
  tty_input = (1ULL << static_cast<int>(signalc::tty_input)),                                //!< Terminal has input (`SIGTTIN`). POSIX only.
  tty_output = (1ULL << static_cast<int>(signalc::tty_output)),                              //!< Terminal ready for output (`SIGTTOU`). POSIX only.
  urgent_condition = (1ULL << static_cast<int>(signalc::urgent_condition)),                  //!< Urgent condition (`SIGURG`). POSIX only.
  user_defined1 = (1ULL << static_cast<int>(signalc::user_defined1)),                        //!< User defined 1 (`SIGUSR1`). POSIX only.
  user_defined2 = (1ULL << static_cast<int>(signalc::user_defined2)),                        //!< User defined 2 (`SIGUSR2`). POSIX only.
  virtual_alarm_clock = (1ULL << static_cast<int>(signalc::virtual_alarm_clock)),            //!< Virtual alarm clock (`SIGVTALRM`). POSIX only.
  cpu_time_limit_exceeded = (1ULL << static_cast<int>(signalc::cpu_time_limit_exceeded)),    //!< CPU time limit exceeded (`SIGXCPU`). POSIX only.
  file_size_limit_exceeded = (1ULL << static_cast<int>(signalc::file_size_limit_exceeded)),  //!< File size limit exceeded (`SIGXFSZ`). POSIX only.
#endif

  // C++ handlers
  cxx_out_of_memory = (1ULL << static_cast<int>(signalc::cxx_out_of_memory)),  //!< A call to operator new failed, and a throw is about to occur
  cxx_termination = (1ULL << static_cast<int>(signalc::cxx_termination))       //!< A call to std::terminate() was made

  } QUICKCPPLIB_BITFIELD_END(signalc_set)

  namespace detail
  {
    SIGNALGUARD_FUNC_DECL const char *signalc_to_string(signalc code) noexcept;
    extern inline std::atomic<signalc_set> &signal_guards_installed()
    {
      static std::atomic<signalc_set> v;
      return v;
    }
  }

  /*! \brief On platforms where it is necessary (POSIX), installs, and potentially enables,
  the global signal handlers for the signals specified by `guarded`. Each signal installed
  is threadsafe reference counted, so this is safe to call from multiple threads or instantiate
  multiple times. It is also guaranteed safe to call from within static data init or deinit,
  so a very common use case is simply to place an instance into global static data. This
  ensures that dynamically loaded and unloaded shared objects compose signal guards appropriately.

  On platforms with better than POSIX global signal support, this class does nothing.

  ## POSIX only

  Any existing global signal handlers are replaced with a filtering signal handler, which
  checks if the current kernel thread has installed a signal guard, and if so executes the
  guard. If no signal guard has been installed for the current kernel thread, global signal
  continuation handlers are executed. If none claims the signal, the previously
  installed signal handler is called.

  After the new signal handlers have been installed, the guarded signals are globally enabled
  for all threads of execution. Be aware that the handlers are installed with `SA_NODEFER`
  to avoid the need to perform an expensive syscall when a signal is handled.
  However this may also produce surprise e.g. infinite loops.

  \warning This class is threadsafe with respect to other concurrent executions of itself,
  but is NOT threadsafe with respect to other code modifying the global signal handlers. It
  is NOT async signal safe.
  */
  class SIGNALGUARD_CLASS_DECL signal_guard_install
  {
    signalc_set _guarded;

  public:
    SIGNALGUARD_MEMFUNC_DECL explicit signal_guard_install(signalc_set guarded);
    SIGNALGUARD_MEMFUNC_DECL ~signal_guard_install();
    signal_guard_install(const signal_guard_install &) = delete;
    signal_guard_install(signal_guard_install &&o) noexcept
        : _guarded(o._guarded)
    {
      o._guarded = signalc_set::none;
    }
    signal_guard_install &operator=(const signal_guard_install &) = delete;
    signal_guard_install &operator=(signal_guard_install &&o) noexcept
    {
      this->~signal_guard_install();
      new(this) signal_guard_install(static_cast<signal_guard_install &&>(o));
      return *this;
    }
  };

  namespace detail
  {
    static inline bool signal_guard_global_decider_impl_indirect(raised_signal_info *rsi);
    class SIGNALGUARD_CLASS_DECL signal_guard_global_decider_impl
    {
      friend inline bool signal_guard_global_decider_impl_indirect(raised_signal_info *rsi);
      signal_guard_install _install;
      void *_p{nullptr};
      virtual bool _call(raised_signal_info *) const noexcept = 0;

    public:
      SIGNALGUARD_MEMFUNC_DECL signal_guard_global_decider_impl(signalc_set guarded, bool callfirst);
      virtual SIGNALGUARD_MEMFUNC_DECL ~signal_guard_global_decider_impl();
      signal_guard_global_decider_impl(const signal_guard_global_decider_impl &) = delete;
      SIGNALGUARD_MEMFUNC_DECL signal_guard_global_decider_impl(signal_guard_global_decider_impl &&o) noexcept;
      signal_guard_global_decider_impl &operator=(const signal_guard_global_decider_impl &) = delete;
      signal_guard_global_decider_impl &operator=(signal_guard_global_decider_impl &&) = delete;
    };
  }  // namespace detail

  /*! \brief Install a global signal continuation decider.

  This is threadsafe with respect to concurrent instantiations of this type, but not reentrant
  i.e. modifying the global signal continuation decider registry whilst inside a global signal
  continuation decider is racy. Callable is called after
  all thread local handling is exhausted. Note that what you can safely do in the decider
  callable is extremely limited, only async signal safe functions may be called.

  A `signal_guard_install` is always instanced for every global decider, and creating and destroying
  these is NOT async signal safe i.e. create and destroy these ONLY outside a signal handler.

  On POSIX only, a global signal continuation decider is ALWAYS installed for `signalc::timer_expire`
  (`SIGALRM`) in order to implement `signal_guard_watchdog`. Be aware that if any `signal_guard_watchdog`
  exist and their expiry times indicate that they have expired, the `SIGALRM` will be consumed. You
  may wish to install further global signal continuation deciders using `callfirst = true` if you wish
  to be called before any `signal_guard_watchdog` processing. Note also that if no `signal_guard_watchdog`
  expiry times have expired, then the default action for `SIGALRM` is taken.
  */
  template <class T> class signal_guard_global_decider : public detail::signal_guard_global_decider_impl
  {
    T _f;
    virtual bool _call(raised_signal_info *rsi) const noexcept override final { return _f(rsi); }

  public:
    /*! \brief Constructs an instance.
    \param guarded The signal set for which this decider ought to be called.
    \param f A callable with prototype `bool(raised_signal_info *)`, which must return
    `true` if execution is to resume, `false` if the next decider function should be called.
    Note that on Windows only, `signalc::interrupt` and `signalc::process_terminate` call `f` from some other
    kernel thread, and the return value is always treated as `false`.
    \param callfirst True if this decider should be called before any other. Otherwise
    call order is in the order of addition.
    */
    QUICKCPPLIB_TEMPLATE(class U)
    QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(std::is_constructible<T, U>::value), QUICKCPPLIB_TEXPR(std::declval<U>()((raised_signal_info *) 0)))
    signal_guard_global_decider(signalc_set guarded, U &&f, bool callfirst)
        : detail::signal_guard_global_decider_impl(guarded, callfirst)
        , _f(static_cast<U &&>(f))
    {
    }
    ~signal_guard_global_decider() = default;
    signal_guard_global_decider(const signal_guard_global_decider &) = delete;
    signal_guard_global_decider(signal_guard_global_decider &&o) noexcept = default;
    signal_guard_global_decider &operator=(const signal_guard_global_decider &) = delete;
    signal_guard_global_decider &operator=(signal_guard_global_decider &&o) noexcept
    {
      this->~signal_guard_global_decider();
      new(this) signal_guard_global_decider(static_cast<signal_guard_global_decider &&>(o));
      return *this;
    }
  };
  //! \brief Convenience instantiator of `signal_guard_global_decider`.
  QUICKCPPLIB_TEMPLATE(class U)
  QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TEXPR(std::declval<U>()((raised_signal_info *) 0)))
  inline signal_guard_global_decider<std::decay_t<U>> make_signal_guard_global_decider(signalc_set guarded, U &&f, bool callfirst = false)
  {
    return signal_guard_global_decider<std::decay_t<U>>(guarded, static_cast<U &&>(f), callfirst);
  }

  /*! \brief Call the currently installed signal handler for a signal (POSIX), or raise a Win32 structured
  exception (Windows), returning false if no handler was called due to the currently
  installed handler being `SIG_IGN` (POSIX).

  Note that on POSIX, we fetch the currently installed signal handler and try to call it directly.
  This allows us to supply custom `raw_info` and `raw_context`, and we do all the things which the signal
  handler flags tell us to do beforehand [1]. If the current handler has been defaulted, we
  enable the signal and execute `pthread_kill(pthread_self(), signo)` in order to invoke the
  default handling.

  Note that on Windows, `raw_context` is ignored as there is no way to override the context thrown
  with a Win32 structured exception.

  [1]: We currently do not implement alternative stack switching. If a handler requests that, we
  simply abort the process. Code donations implementing support are welcome.
  */
  SIGNALGUARD_FUNC_DECL bool thrd_raise_signal(signalc signo, void *raw_info = nullptr, void *raw_context = nullptr);

  /*! \brief This initiates a fast fail process termination which immediately exits the process
  without calling any handlers: on POSIX, this is `SIGKILL`, on Windows this is `TerminateProcess()`.

  If `msg` is specified, it async signal safely prints that message before termination. If you don't
  specify a message (`nullptr`), a default message is printed. A message of `""` prints nothing.
  */
  QUICKCPPLIB_NORETURN SIGNALGUARD_FUNC_DECL void terminate_process_immediately(const char *msg = nullptr) noexcept;

  namespace detail
  {
    struct invoke_terminate_process_immediately
    {
      void operator()() const noexcept { terminate_process_immediately("signal_guard_watchdog expired"); }
    };
    class SIGNALGUARD_CLASS_DECL signal_guard_watchdog_impl
    {
      struct _signal_guard_watchdog_decider;
      friend struct watchdog_decider_t;
#ifdef _WIN32
      void *_threadh{nullptr};
#else
      void *_timerid{nullptr};
#endif
      signal_guard_watchdog_impl *_prev{nullptr}, *_next{nullptr};
      uint64_t _deadline_ms{0};
      bool _inuse{false}, _alreadycalled{false};
      virtual void _call() const noexcept = 0;

      SIGNALGUARD_MEMFUNC_DECL void _detach() noexcept;

    public:
      SIGNALGUARD_MEMFUNC_DECL signal_guard_watchdog_impl(unsigned ms);
      virtual ~signal_guard_watchdog_impl()
      {
        if(_inuse)
        {
          try
          {
            release();
          }
          catch(...)
          {
          }
        }
      }
      signal_guard_watchdog_impl(const signal_guard_watchdog_impl &) = delete;
      SIGNALGUARD_MEMFUNC_DECL signal_guard_watchdog_impl(signal_guard_watchdog_impl &&o) noexcept;
      signal_guard_watchdog_impl &operator=(const signal_guard_watchdog_impl &) = delete;
      signal_guard_watchdog_impl &operator=(signal_guard_watchdog_impl &&) = delete;
      SIGNALGUARD_MEMFUNC_DECL void release();
    };
  }  // namespace detail

  /*! \brief Call an optional specified routine after a period of time, possibly on another thread. Async signal safe.

  In your signal handler, you may have no choice but to execute async signal unsafe code e.g. you
  absolutely must call `malloc()` because third party code does so and there is no way to not
  call that third party code. This can very often lead to hangs, whether due to infinite loops
  or getting deadlocked by locking an already locked mutex.

  This facility provides the ability to set a watchdog timer which will either call your specified
  routine or the default routine after a period of time after construction, unless it is released
  or destructed first. Your routine will be called via an async signal (`SIGALRM`) if on POSIX, or
  from a separate thread if on Windows. Therefore, if on Windows, a kernel thread will be launched
  on construction, and killed on destruction. On POSIX, a global signal continuation decider is
  ALWAYS installed at process init for `signalc::timer_expire` (`SIGALRM`) in order to implement
  `signal_guard_watchdog`, this is a filtering decider which only matches `signal_guard_watchdog`
  whose timers have expired, otherwise it passes on the signal to other handlers.

  If you don't specify a routine, the default routine is `terminate_process_immediately()`
  which performs a fast fail process termination.
  */
  template <class T> class signal_guard_watchdog : public detail::signal_guard_watchdog_impl
  {
    T _f;
    virtual void _call() const noexcept override final { _f(); }

  public:
    /*! \brief Constructs an instance.
    \param f A callable with prototype `bool(raised_signal_info *)`, which must return
    `true` if execution is to resume, `false` if the next decider function should be called.
    Note that on Windows only, `signalc::interrupt` and `signalc::process_terminate` call `f` from some other
    kernel thread, and the return value is always treated as `false`.
    \param callfirst True if this decider should be called before any other. Otherwise
    call order is in the order of addition.
    */
    QUICKCPPLIB_TEMPLATE(class U)
    QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(std::is_constructible<T, U>::value), QUICKCPPLIB_TEXPR(std::declval<U>()()))
    signal_guard_watchdog(U &&f, unsigned ms)
        : detail::signal_guard_watchdog_impl(ms)
        , _f(static_cast<U &&>(f))
    {
    }
    ~signal_guard_watchdog() = default;
    signal_guard_watchdog(const signal_guard_watchdog &) = delete;
    signal_guard_watchdog(signal_guard_watchdog &&o) noexcept = default;
    signal_guard_watchdog &operator=(const signal_guard_watchdog &) = delete;
    signal_guard_watchdog &operator=(signal_guard_watchdog &&o) noexcept
    {
      this->~signal_guard_watchdog();
      new(this) signal_guard_watchdog(static_cast<signal_guard_watchdog &&>(o));
      return *this;
    }
  };
  //! \brief Convenience instantiator of `signal_guard_watchdog`.
  QUICKCPPLIB_TEMPLATE(class U)
  QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TEXPR(std::declval<U>()()))
  inline signal_guard_watchdog<std::decay_t<U>> make_signal_guard_watchdog(U &&f, unsigned ms = 3000)
  {
    return signal_guard_watchdog<std::decay_t<U>>(static_cast<U &&>(f), ms);
  }
  //! \brief Convenience instantiator of `signal_guard_watchdog`.
  inline signal_guard_watchdog<detail::invoke_terminate_process_immediately> make_signal_guard_watchdog(unsigned ms = 3000)
  {
    return signal_guard_watchdog<detail::invoke_terminate_process_immediately>(detail::invoke_terminate_process_immediately(), ms);
  }


  //! \brief Thrown by the default signal handler to abort the current operation
  class signal_raised : public std::exception
  {
    signalc _code;

  public:
    //! Constructor
    signal_raised(signalc code)
        : _code(code)
    {
    }
    virtual const char *what() const noexcept override { return detail::signalc_to_string(_code); }
  };

  namespace detail
  {
    struct thread_local_signal_guard;
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
    SIGNALGUARD_FUNC_DECL void push_thread_local_signal_handler(thread_local_signal_guard *) noexcept;
    SIGNALGUARD_FUNC_DECL void pop_thread_local_signal_handler(thread_local_signal_guard *) noexcept;
    SIGNALGUARD_FUNC_DECL thread_local_signal_guard *current_thread_local_signal_handler() noexcept;
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
    struct thread_local_signal_guard
    {
      signalc_set guarded;
      raised_signal_info info;
      thread_local_signal_guard *previous{nullptr};

      thread_local_signal_guard(signalc_set _guarded)
          : guarded(_guarded)
      {
        push_thread_local_signal_handler(this);
      }
      virtual ~thread_local_signal_guard() { pop_thread_local_signal_handler(this); }
      virtual bool call_continuer() = 0;
    };

#ifdef _WIN32
    namespace win32
    {
      struct _EXCEPTION_POINTERS;
    }
    SIGNALGUARD_FUNC_DECL long win32_exception_filter_function(unsigned long code, win32::_EXCEPTION_POINTERS *pts) noexcept;
#endif
    template <class R> inline R throw_signal_raised(const raised_signal_info *i) { throw signal_raised(signalc(1ULL << i->signo)); }
    inline bool continue_or_handle(const raised_signal_info * /*unused*/) noexcept { return false; }
    template <class R, class V> struct is_constructible_or_void : std::is_constructible<R, V>
    {
    };
    template <> struct is_constructible_or_void<void, void> : std::true_type
    {
    };
  }  // namespace detail

#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ >= 6
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclobbered"
#if __GNUC__ >= 7
#pragma GCC diagnostic ignored "-Wnoexcept-type"
#endif
#endif
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4611)  // interaction between setjmp() and C++ object destruction is non-portable
#endif

  /*! Call a callable `f` with signals `guarded` protected for this thread only, returning whatever `f` or `h` returns.

  Firstly, how to restore execution to this context is saved, and `f(Args...)` is executed, returning whatever
  `f(Args...)` returns if `f` completes execution successfully. This is usually inlined code, so it will be
  quite fast. No memory allocation is performed if a `signal_guard_install` for the guarded signal set is already
  instanced. Approximate best case overhead:

  - Linux: 28 CPU cycles (Intel CPU), 53 CPU cycles (AMD CPU)
  - Windows: 36 CPU cycles (Intel CPU), 68 CPU cycles (AMD CPU)

  If during the execution of `f`, any one of the signals `guarded` is raised:

  1. `c`, which must have the prototype `bool(raised_signal_info *)`, is called with the signal which
  was raised. You can fix the cause of the signal and return `true` to continue execution, or else return `false`
  to halt execution. Note that the variety of code you can call in `c` is extremely limited, the same restrictions
  as for signal handlers apply. Note that on Windows only, `signalc::interrupt` and `signalc::process_terminate` call `c` from
  some other kernel thread, and the return value is always ignored (the process is always exited).

  2. If `c` returned `false`, the execution of `f` is halted **immediately** without stack unwind, the thread is returned
  to the state just before the calling of `f`, and the callable `g` is called with the specific signal
  which occurred. `g` must have the prototype `R(const raised_signal_info *)` where `R` is the return type of `f`.
  `g` is called with this signal guard removed, though a signal guard higher in the call chain may instead be active.

  Obviously all state which `f` may have been in the process of doing will be thrown away, in particular
  any stack allocated variables not marked `volatile` will have unspecified values. You
  should therefore make sure that `f` never causes side effects, including the interruption in the middle
  of some operation, which cannot be fixed by the calling of `h`. The default `h` simply throws a `signal_raised`
  C++ exception.

  \note On POSIX, if a `signal_guard_install` is not already instanced for the guarded set,
  one is temporarily installed, which is not quick. You are therefore very strongly recommended, when on POSIX,
  to call this function with a `signal_guard_install` already installed for all the signals you will ever guard.
  `signal_guard_install` is guaranteed to be composable and be safe to use within static data init, so a common
  use pattern is simply to place a guard install into your static data init.

  \note On MSVC because `std::set_terminate()` is thread local, we ALWAYS install our termination handler for
  every thread on creation and we never uninstall it.

  \note On MSVC, you cannot pass a type with a non-trivial destructor through `__try`, so this will limit the
  complexity of types that can be returned directly by this function. You can usually work around this via an
  intermediate `std::optional`.
  */
  QUICKCPPLIB_TEMPLATE(class F, class H, class C, class... Args, class R = decltype(std::declval<F>()(std::declval<Args>()...)))
  QUICKCPPLIB_TREQUIRES(
  QUICKCPPLIB_TPRED(detail::is_constructible_or_void<R, decltype(std::declval<H>()(std::declval<const raised_signal_info *>()))>::value),  //
  QUICKCPPLIB_TPRED(detail::is_constructible_or_void<bool, decltype(std::declval<C>()(std::declval<raised_signal_info *>()))>::value))
  inline R signal_guard(signalc_set guarded, F &&f, H &&h, C &&c, Args &&...args)
  {
    if(guarded == signalc_set::none)
    {
      return f(static_cast<Args &&>(args)...);
    }
    struct signal_guard_installer final : detail::thread_local_signal_guard
    {
      char _storage[sizeof(signal_guard_install)];
      signal_guard_install *sgi{nullptr};
      C &continuer;

      signal_guard_installer(signalc_set guarded, C &c)
          : detail::thread_local_signal_guard(guarded)
          , continuer(c)
      {
#ifndef _WIN32
        uint64_t oldinstalled(detail::signal_guards_installed().load(std::memory_order_relaxed));
        uint64_t newinstalled = oldinstalled | uint64_t(guarded);
        if(newinstalled != oldinstalled)
        {
          // Need a temporary signal guard install
          sgi = new(_storage) signal_guard_install(guarded);
        }
#endif
      }
      ~signal_guard_installer() { reset(); }
      void reset()
      {
#ifndef _WIN32
        if(nullptr != sgi)
        {
          sgi->~signal_guard_install();
        }
#endif
      }
      bool call_continuer() override { return continuer(&this->info); }
    } sgi(guarded, c);
    // Nothing in the C++ standard says that the compiler cannot reorder reads and writes
    // around a setjmp(), so let's prevent that. This is the weak form affecting the
    // compiler reordering only.
    std::atomic_signal_fence(std::memory_order_seq_cst);
    if(setjmp(sgi.info.buf))
    {
      // returning from longjmp, so unset the TLS and call failure handler
      sgi.reset();
      return h(&sgi.info);
    }
    std::atomic_signal_fence(std::memory_order_seq_cst);
#ifdef _WIN32
    // Cannot use __try in a function with non-trivial destructors
    return [&] {
      __try
      {
        static_assert(std::is_trivially_destructible<R>::value || std::is_void<R>::value,
                      "On MSVC, the return type of F must be trivially destructible, otherwise the compiler will refuse to compile the code.");
        return f(static_cast<Args &&>(args)...);
      }
      __except(detail::win32_exception_filter_function(_exception_code(), (struct detail::win32::_EXCEPTION_POINTERS *) _exception_info()))
      {
        longjmp(sgi.info.buf, 1);
      }
    }();
#else
    return f(static_cast<Args &&>(args)...);
#endif
  }
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ >= 6
#pragma GCC diagnostic pop
#endif
  //! \overload
  template <class F, class R = decltype(std::declval<F>()())> inline R signal_guard(signalc_set guarded, F &&f)
  {
    return signal_guard(guarded, static_cast<F &&>(f), detail::throw_signal_raised<R>, detail::continue_or_handle);
  }
  //! \overload
  QUICKCPPLIB_TEMPLATE(class F, class H, class R = decltype(std::declval<F>()()))
  QUICKCPPLIB_TREQUIRES(QUICKCPPLIB_TPRED(detail::is_constructible_or_void<R, decltype(std::declval<H>()(std::declval<const raised_signal_info *>()))>::value))
  inline auto signal_guard(signalc_set guarded, F &&f, H &&h)
  {
    return signal_guard(guarded, static_cast<F &&>(f), static_cast<H &&>(h), detail::continue_or_handle);
  }
}  // namespace signal_guard

QUICKCPPLIB_NAMESPACE_END

#if(!defined(QUICKCPPLIB_HEADERS_ONLY) || QUICKCPPLIB_HEADERS_ONLY == 1) && !defined(DOXYGEN_SHOULD_SKIP_THIS)
#define QUICKCPPLIB_INCLUDED_BY_HEADER 1
#include "detail/impl/signal_guard.ipp"
#undef QUICKCPPLIB_INCLUDED_BY_HEADER
#endif

#endif


#endif
