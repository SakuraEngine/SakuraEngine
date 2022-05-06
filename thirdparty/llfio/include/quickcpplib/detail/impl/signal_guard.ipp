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

#include "../include/quickcpplib/signal_guard.hpp"

#include "../include/quickcpplib/scope.hpp"
#include "../include/quickcpplib/spinlock.hpp"

#include <cstring>       // for memset etc
#include <system_error>  // for system_error

#ifndef _WIN32
#include <time.h>    // for timers
#include <unistd.h>  // for write()
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4190)  // C-linkage with UDTs
#endif
extern "C" union raised_signal_info_value thrd_signal_guard_call(const sigset_t *signals, thrd_signal_guard_guarded_t guarded,
                                                                 thrd_signal_guard_recover_t recovery, thrd_signal_guard_decide_t decider,
                                                                 union raised_signal_info_value value)
{
  using namespace QUICKCPPLIB_NAMESPACE::signal_guard;
  signalc_set mask;
  for(size_t n = 0; n < 31; n++)
  {
    if(sigismember(signals, n))
    {
      mask |= static_cast<signalc_set>(1ULL << n);
    }
  }
  return signal_guard(mask, guarded, recovery, decider, value);
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif

extern "C" bool thrd_raise_signal(int signo, void *raw_info, void *raw_context)
{
  using namespace QUICKCPPLIB_NAMESPACE::signal_guard;
  return thrd_raise_signal(static_cast<signalc>(1U << signo), raw_info, raw_context);
}

extern "C" void *signal_guard_create(const sigset_t *guarded)
{
  using namespace QUICKCPPLIB_NAMESPACE::signal_guard;
  signalc_set mask;
  for(size_t n = 0; n < 31; n++)
  {
    if(sigismember(guarded, n))
    {
      mask |= static_cast<signalc_set>(1ULL << n);
    }
  }
  return new(std::nothrow) QUICKCPPLIB_NAMESPACE::signal_guard::signal_guard_install(mask);
}

extern "C" bool signal_guard_destroy(void *i)
{
  auto *p = (QUICKCPPLIB_NAMESPACE::signal_guard::signal_guard_install *) i;
  delete p;
  return true;
}

QUICKCPPLIB_NAMESPACE_BEGIN

namespace signal_guard
{
  namespace detail
  {
    struct signal_guard_decider_callable
    {
      thrd_signal_guard_decide_t decider;
      raised_signal_info_value value;
      bool operator()(raised_signal_info *rsi) const noexcept
      {
        rsi->value = value;
        return decider(rsi);
      }
    };
  }  // namespace detail
}  // namespace signal_guard
QUICKCPPLIB_NAMESPACE_END

extern "C" void *signal_guard_decider_create(const sigset_t *guarded, bool callfirst, thrd_signal_guard_decide_t decider, union raised_signal_info_value value)
{
  using namespace QUICKCPPLIB_NAMESPACE::signal_guard;
  signalc_set mask;
  for(size_t n = 0; n < 31; n++)
  {
    if(sigismember(guarded, n))
    {
      mask |= static_cast<signalc_set>(1ULL << n);
    }
  }
  return new(std::nothrow)
  signal_guard_global_decider<detail::signal_guard_decider_callable>(mask, detail::signal_guard_decider_callable{decider, value}, callfirst);
}
extern "C" bool signal_guard_decider_destroy(void *decider)
{
  using namespace QUICKCPPLIB_NAMESPACE::signal_guard;
  auto *p = (signal_guard_global_decider<detail::signal_guard_decider_callable> *) decider;
  delete p;
  return true;
}


QUICKCPPLIB_NAMESPACE_BEGIN

namespace signal_guard
{
  namespace detail
  {
    struct global_signal_decider
    {
      global_signal_decider *next, *prev;
      signalc_set guarded;
      thrd_signal_guard_decide_t decider;
      raised_signal_info_value value;
    };
    extern QUICKCPPLIB_SYMBOL_VISIBLE inline thread_local_signal_guard *&_current_thread_local_signal_handler() noexcept
    {
      static thread_local thread_local_signal_guard *v;
      return v;
    }
    extern QUICKCPPLIB_SYMBOL_VISIBLE inline std::terminate_handler &terminate_handler_old() noexcept
    {
#ifdef _MSC_VER
      static thread_local std::terminate_handler v;
#else
      static std::terminate_handler v;
#endif
      return v;
    }
    struct _state_t
    {
      configurable_spinlock::spinlock<uintptr_t> lock;
      unsigned new_handler_count{0}, terminate_handler_count{0};
      std::new_handler new_handler_old{};
      global_signal_decider *global_signal_deciders_front{nullptr}, *global_signal_deciders_back{nullptr};
#ifdef _WIN32
      unsigned win32_console_ctrl_handler_count{0};
      void *win32_global_signal_decider1{nullptr};
      void *win32_global_signal_decider2{nullptr};
#endif
    };
    extern QUICKCPPLIB_SYMBOL_VISIBLE inline _state_t &_state() noexcept
    {
      static _state_t v;
      return v;
    }
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
    SIGNALGUARD_FUNC_DECL QUICKCPPLIB_NOINLINE void push_thread_local_signal_handler(thread_local_signal_guard *g) noexcept
    {
      g->previous = _current_thread_local_signal_handler();
      _current_thread_local_signal_handler() = g;
    }
    SIGNALGUARD_FUNC_DECL QUICKCPPLIB_NOINLINE void pop_thread_local_signal_handler(thread_local_signal_guard *g) noexcept
    {
      assert(_current_thread_local_signal_handler() == g);
      if(_current_thread_local_signal_handler() != g)
      {
        abort();
      }
      _current_thread_local_signal_handler() = g->previous;
    }
    SIGNALGUARD_FUNC_DECL QUICKCPPLIB_NOINLINE thread_local_signal_guard *current_thread_local_signal_handler() noexcept
    {
      return _current_thread_local_signal_handler();
    }
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

    SIGNALGUARD_FUNC_DECL const char *signalc_to_string(signalc code) noexcept
    {
      static constexpr const struct
      {
        signalc code;
        const char *string;
      } strings[] = {
      //
      {signalc::none, "none"},                                               //
      {signalc::abort_process, "Signal abort process"},                      //
      {signalc::undefined_memory_access, "Signal undefined memory access"},  //
      {signalc::illegal_instruction, "Signal illegal instruction"},          //
      {signalc::interrupt, "Signal interrupt"},                              //
      {signalc::broken_pipe, "Signal broken pipe"},                          //
      {signalc::segmentation_fault, "Signal segmentation fault"},            //
      {signalc::floating_point_error, "Signal floating point error"},        //
      {signalc::process_terminate, "Process termination requested"},         //

#ifndef _WIN32
      {signalc::timer_expire, "Timer has expired"},                 //
      {signalc::child_exit, "Child has exited"},                    //
      {signalc::process_continue, "Process is being continued"},    //
      {signalc::tty_hangup, "Controlling terminal has hung up"},    //
      {signalc::process_kill, "Process has received kill signal"},  //
#ifdef SIGPOLL
      {signalc::pollable_event, "i/o is now possible"},  //
#endif
      {signalc::profile_event, "Profiling timer expired"},              //
      {signalc::process_quit, "Process is being quit"},                 //
      {signalc::process_stop, "Process is being stopped"},              //
      {signalc::tty_stop, "Terminal requests stop"},                    //
      {signalc::bad_system_call, "Bad system call"},                    //
      {signalc::process_trap, "Process has reached a breakpoint"},      //
      {signalc::tty_input, "Terminal has input"},                       //
      {signalc::tty_output, "Terminal ready for output"},               //
      {signalc::urgent_condition, "Urgent condition"},                  //
      {signalc::user_defined1, "User defined 1"},                       //
      {signalc::user_defined2, "User defined 2"},                       //
      {signalc::virtual_alarm_clock, "Virtual alarm clock"},            //
      {signalc::cpu_time_limit_exceeded, "CPU time limit exceeded"},    //
      {signalc::file_size_limit_exceeded, "File size limit exceeded"},  //
#endif

      {signalc::cxx_out_of_memory, "C++ out of memory"},  //
      {signalc::cxx_termination, "C++ termination"}       //
      };
      for(auto &i : strings)
      {
        if(code == i.code)
          return i.string;
      }
      return "unknown";
    }

    // Overload for when a C++ signal is being raised
    inline bool set_siginfo(thread_local_signal_guard *g, signalc _cppsignal)
    {
      auto cppsignal = static_cast<signalc_set>(1ULL << static_cast<int>(_cppsignal));
      if(g->guarded & cppsignal)
      {
        g->info.signo = static_cast<int>(_cppsignal);
        g->info.error_code = 0;  // no system error code for C++ signals
        g->info.addr = 0;        // no address for OOM nor terminate
        g->info.value.ptr_value = nullptr;
        g->info.raw_info = nullptr;
        g->info.raw_context = nullptr;
        return true;
      }
      return false;
    }

#ifdef _WIN32
    // WARNING: Always called from a separate thread!
    inline int __stdcall win32_console_ctrl_handler(unsigned long dwCtrlType)
    {
      signalc signo;
      switch(dwCtrlType)
      {
      case 0:  // CTRL_C_EVENT
      case 1:  // CTRL_BREAK_EVENT
      {
        signo = signalc::interrupt;
        break;
      }
      case 2:  // CTRL_CLOSE_EVENT
      {
        signo = signalc::process_terminate;
        break;
      }
      default:
        return 0;  // not handled
      }
      for(auto *shi = current_thread_local_signal_handler(); shi != nullptr; shi = shi->previous)
      {
        if(set_siginfo(shi, signo))
        {
          shi->call_continuer();
        }
      }
      _state().lock.lock();
      if(_state().global_signal_deciders_front != nullptr)
      {
        raised_signal_info rsi;
        memset(&rsi, 0, sizeof(rsi));
        rsi.signo = static_cast<int>(signo);
        auto *d = _state().global_signal_deciders_front;
        for(size_t n = 0; d != nullptr; n++)
        {
          size_t i = 0;
          for(d = _state().global_signal_deciders_front; d != nullptr; d = d->next)
          {
            if(i++ == n)
            {
              if(d->guarded & (1ULL << static_cast<int>(signo)))
              {
                rsi.value = d->value;
                _state().lock.unlock();
                d->decider(&rsi);
                _state().lock.lock();
                break;
              }
              n++;
            }
          }
        }
      }
      return 0;  // call other handlers
    }
#endif

    inline void new_handler()
    {
      for(auto *shi = current_thread_local_signal_handler(); shi != nullptr; shi = shi->previous)
      {
        if(set_siginfo(shi, signalc::cxx_out_of_memory))
        {
          if(!shi->call_continuer())
          {
            longjmp(shi->info.buf, 1);
          }
        }
      }
      _state().lock.lock();
      if(_state().global_signal_deciders_front != nullptr)
      {
        raised_signal_info rsi;
        memset(&rsi, 0, sizeof(rsi));
        rsi.signo = static_cast<int>(signalc::cxx_out_of_memory);
        auto *d = _state().global_signal_deciders_front;
        for(size_t n = 0; d != nullptr; n++)
        {
          size_t i = 0;
          for(d = _state().global_signal_deciders_front; d != nullptr; d = d->next)
          {
            if(i++ == n)
            {
              if(d->guarded & (1ULL << static_cast<int>(signalc::cxx_out_of_memory)))
              {
                rsi.value = d->value;
                _state().lock.unlock();
                if(d->decider(&rsi))
                {
                  // Cannot continue OOM
                  abort();
                }
                _state().lock.lock();
                break;
              }
              n++;
            }
          }
        }
      }
      if(_state().new_handler_old != nullptr)
      {
        auto *h = _state().new_handler_old;
        _state().lock.unlock();
        h();
      }
      else
      {
        _state().lock.unlock();
        throw std::bad_alloc();
      }
    }
    inline void terminate_handler()
    {
      for(auto *shi = current_thread_local_signal_handler(); shi != nullptr; shi = shi->previous)
      {
        if(set_siginfo(shi, signalc::cxx_termination))
        {
          if(!shi->call_continuer())
          {
            longjmp(shi->info.buf, 1);
          }
        }
      }
      _state().lock.lock();
      if(_state().global_signal_deciders_front != nullptr)
      {
        raised_signal_info rsi;
        memset(&rsi, 0, sizeof(rsi));
        rsi.signo = static_cast<int>(signalc::cxx_termination);
        auto *d = _state().global_signal_deciders_front;
        for(size_t n = 0; d != nullptr; n++)
        {
          size_t i = 0;
          for(d = _state().global_signal_deciders_front; d != nullptr; d = d->next)
          {
            if(i++ == n)
            {
              if(d->guarded & (1ULL << static_cast<int>(signalc::cxx_termination)))
              {
                rsi.value = d->value;
                _state().lock.unlock();
                if(d->decider(&rsi))
                {
                  // Cannot continue termination
                  abort();
                }
                _state().lock.lock();
                break;
              }
              n++;
            }
          }
        }
      }
      if(terminate_handler_old() != nullptr)
      {
        auto *h = terminate_handler_old();
        assert(h != terminate_handler);
        _state().lock.unlock();
        h();
      }
      else
      {
        _state().lock.unlock();
        std::abort();
      }
    }
#ifdef _MSC_VER
    static thread_local struct _win32_set_terminate_handler_per_thread_t
    {
      _win32_set_terminate_handler_per_thread_t()
      {
        // On MSVC, the terminate handler is thread local, so we have no choice but to always
        // set it for every thread
        if(std::get_terminate() != terminate_handler)
        {
          terminate_handler_old() = std::set_terminate(terminate_handler);
        }
      }
    } _win32_set_terminate_handler_per_thread;
#endif


#ifdef _WIN32
    namespace win32
    {
      typedef struct _EXCEPTION_RECORD
      {
        unsigned long ExceptionCode;
        unsigned long ExceptionFlags;
        struct _EXCEPTION_RECORD *ExceptionRecord;
        void *ExceptionAddress;
        unsigned long NumberParameters;
        unsigned long long ExceptionInformation[15];
      } EXCEPTION_RECORD;

      typedef EXCEPTION_RECORD *PEXCEPTION_RECORD;

      struct CONTEXT;
      typedef CONTEXT *PCONTEXT;

      typedef struct _EXCEPTION_POINTERS
      {
        PEXCEPTION_RECORD ExceptionRecord;
        PCONTEXT ContextRecord;
      } EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;

      typedef long(__stdcall *PVECTORED_EXCEPTION_HANDLER)(struct _EXCEPTION_POINTERS *ExceptionInfo);

      typedef int(__stdcall *PHANDLER_ROUTINE)(unsigned long dwCtrlType);

      typedef struct _OVERLAPPED OVERLAPPED, *LPOVERLAPPED;

      typedef struct _SECURITY_ATTRIBUTES SECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

      typedef unsigned long(__stdcall *LPTHREAD_START_ROUTINE)(void *lpParameter);

      typedef void(__stdcall *PAPCFUNC)(uintptr_t);

      extern void __stdcall RaiseException(unsigned long dwExceptionCode, unsigned long dwExceptionFlags, unsigned long nNumberOfArguments,
                                           const unsigned long long *lpArguments);
      extern PVECTORED_EXCEPTION_HANDLER __stdcall SetUnhandledExceptionFilter(PVECTORED_EXCEPTION_HANDLER Handler);
      extern void *__stdcall AddVectoredContinueHandler(unsigned long First, PVECTORED_EXCEPTION_HANDLER Handler);
      extern unsigned long __stdcall RemoveVectoredContinueHandler(void *Handle);
      extern unsigned long __stdcall GetLastError();
      extern int __stdcall SetConsoleCtrlHandler(PHANDLER_ROUTINE HandlerRoutine, int Add);
      extern int __stdcall TerminateProcess(void *hProcess, unsigned int uExitCode);
      extern void *__stdcall GetStdHandle(unsigned long);
      extern int __stdcall WriteFile(void *hFile, const void *lpBuffer, unsigned long nNumberOfBytesToWrite, unsigned long *lpNumberOfBytesWritten,
                                     OVERLAPPED *lpOverlapped);
      extern void *__stdcall CreateThread(SECURITY_ATTRIBUTES *lpThreadAttributes, size_t dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, void *lpParameter,
                                          unsigned long dwCreationFlags, unsigned long *lpThreadId);
      extern unsigned long __stdcall QueueUserAPC(PAPCFUNC pfnAPC, void *hThread, uintptr_t dwData);
      extern unsigned long __stdcall SleepEx(unsigned long dwMillseconds, int bAlertable);
      extern int __stdcall CloseHandle(void *hObject);
      extern unsigned long long __stdcall GetTickCount64();

#pragma comment(lib, "kernel32.lib")
#ifndef QUICKCPPLIB_DISABLE_ABI_PERMUTATION
#define QUICKCPPLIB_SIGNAL_GUARD_SYMBOL2(a, b, c) a #b c
#define QUICKCPPLIB_SIGNAL_GUARD_SYMBOL1(a, b, c) QUICKCPPLIB_SIGNAL_GUARD_SYMBOL2(a, b, c)
#define QUICKCPPLIB_SIGNAL_GUARD_SYMBOL(a, b) QUICKCPPLIB_SIGNAL_GUARD_SYMBOL1(a, QUICKCPPLIB_PREVIOUS_COMMIT_UNIQUE, b)
#if(defined(__x86_64__) || defined(_M_X64)) || (defined(__aarch64__) || defined(_M_ARM64))
#pragma comment(linker,                                                                                                                                        \
                QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?RaiseException@win32@detail@signal_guard@_", "@quickcpplib@@YAXKKKPEB_K@Z=RaiseException"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?SetUnhandledExceptionFilter@win32@detail@signal_guard@_",                             \
                                                        "@quickcpplib@@YAP6AJPEAU_EXCEPTION_POINTERS@12345@@ZP6AJ0@Z@Z=SetUnhandledExceptionFilter"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?AddVectoredContinueHandler@win32@detail@signal_guard@_",                              \
                                                        "@quickcpplib@@YAPEAXKP6AJPEAU_EXCEPTION_POINTERS@12345@@Z@Z=AddVectoredContinueHandler"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?RemoveVectoredContinueHandler@win32@detail@signal_guard@_",                           \
                                                        "@quickcpplib@@YAKPEAX@Z=RemoveVectoredContinueHandler"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?GetLastError@win32@detail@signal_guard@_", "@quickcpplib@@YAKXZ=GetLastError"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?SetConsoleCtrlHandler@win32@detail@signal_guard@_",                                   \
                                                        "@quickcpplib@@YAHP6AHK@ZH@Z=SetConsoleCtrlHandler"))
#pragma comment(linker,                                                                                                                                        \
                QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?TerminateProcess@win32@detail@signal_guard@_", "@quickcpplib@@YAHPEAXI@Z=TerminateProcess"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?GetStdHandle@win32@detail@signal_guard@_", "@quickcpplib@@YAPEAXK@Z=GetStdHandle"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?WriteFile@win32@detail@signal_guard@_",                                               \
                                                        "@quickcpplib@@YAHPEAXPEBXKPEAKPEAU_OVERLAPPED@12345@@Z=WriteFile"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?WriteFile@win32@detail@signal_guard@_",                                               \
                                                        "@quickcpplib@@YAHPEAXPEBXKPEAKPEAU_OVERLAPPED@@@Z=WriteFile"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?CreateThread@win32@detail@signal_guard@_",                                            \
                                                        "@quickcpplib@@YAPEAXPEAU_SECURITY_ATTRIBUTES@12345@_KP6AKPEAX@Z2KPEAK@Z=CreateThread"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?CreateThread@win32@detail@signal_guard@_",                                            \
                                                        "@quickcpplib@@YAPEAXPEAU_SECURITY_ATTRIBUTES@@_KP6AKPEAX@Z2KPEAK@Z=CreateThread"))
#pragma comment(linker,                                                                                                                                        \
                QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?QueueUserAPC@win32@detail@signal_guard@_", "@quickcpplib@@YAKP6AX_K@ZPEAX0@Z=QueueUserAPC"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?SleepEx@win32@detail@signal_guard@_", "@quickcpplib@@YAKKH@Z=SleepEx"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?CloseHandle@win32@detail@signal_guard@_", "@quickcpplib@@YAHPEAX@Z=CloseHandle"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?GetTickCount64@win32@detail@signal_guard@_", "@quickcpplib@@YA_KXZ=GetTickCount64"))
#elif defined(__x86__) || defined(_M_IX86) || defined(__i386__)
#pragma comment(                                                                                                                                               \
linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?RaiseException@win32@detail@signal_guard@_", "@quickcpplib@@YGXKKKPB_K@Z=__imp__RaiseException@16"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?SetUnhandledExceptionFilter@win32@detail@signal_guard@_",                             \
                                                        "@quickcpplib@@YGP6GJPAU_EXCEPTION_POINTERS@12345@@ZP6GJ0@Z@Z=__imp__SetUnhandledExceptionFilter@4"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?AddVectoredContinueHandler@win32@detail@signal_guard@_",                              \
                                                        "@quickcpplib@@YGPAXKP6GJPAU_EXCEPTION_POINTERS@12345@@Z@Z=__imp__AddVectoredContinueHandler@8"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?RemoveVectoredContinueHandler@win32@detail@signal_guard@_",                           \
                                                        "@quickcpplib@@YGKPAX@Z=__imp__RemoveVectoredContinueHandler@4"))
#pragma comment(linker,                                                                                                                                        \
                QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?GetLastError@win32@detail@signal_guard@_", "@quickcpplib@@YGKXZ=__imp__GetLastError@0"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?SetConsoleCtrlHandler@win32@detail@signal_guard@_",                                   \
                                                        "@quickcpplib@@YGHP6GHK@ZH@Z=__imp__SetConsoleCtrlHandler@8"))
#pragma comment(                                                                                                                                               \
linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?TerminateProcess@win32@detail@signal_guard@_", "@quickcpplib@@YGHPAXI@Z=__imp__TerminateProcess@8"))
#pragma comment(linker,                                                                                                                                        \
                QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?GetStdHandle@win32@detail@signal_guard@_", "@quickcpplib@@YGPAXK@Z=__imp__GetStdHandle@4"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?WriteFile@win32@detail@signal_guard@_",                                               \
                                                        "@quickcpplib@@YGHPAXPBXKPAKPAU_OVERLAPPED@12345@@Z=__imp__WriteFile@20"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?WriteFile@win32@detail@signal_guard@_",                                               \
                                                        "@quickcpplib@@YGHPAXPBXKPAKPAU_OVERLAPPED@@@Z=__imp__WriteFile@20"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?CreateThread@win32@detail@signal_guard@_",                                            \
                                                        "@quickcpplib@@YGPAXPAU_SECURITY_ATTRIBUTES@12345@IP6GKPAX@Z1KPAK@Z=__imp__CreateThread@24"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?CreateThread@win32@detail@signal_guard@_",                                            \
                                                        "@quickcpplib@@YGPAXPAU_SECURITY_ATTRIBUTES@@IP6GKPAX@Z1KPAK@Z=__imp__CreateThread@24"))
#pragma comment(                                                                                                                                               \
linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?QueueUserAPC@win32@detail@signal_guard@_", "@quickcpplib@@YGKP6GXI@ZPAXI@Z=__imp__QueueUserAPC@12"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?SleepEx@win32@detail@signal_guard@_", "@quickcpplib@@YGKKH@Z=__imp__SleepEx@8"))
#pragma comment(linker,                                                                                                                                        \
                QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?CloseHandle@win32@detail@signal_guard@_", "@quickcpplib@@YGHPAX@Z=__imp__CloseHandle@4"))
#pragma comment(linker,                                                                                                                                        \
                QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?GetTickCount64@win32@detail@signal_guard@_", "@quickcpplib@@YG_KXZ=__imp__GetTickCount64@0"))
#elif defined(__arm__) || defined(_M_ARM)
#pragma comment(linker,                                                                                                                                        \
                QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?RaiseException@win32@detail@signal_guard@_", "@quickcpplib@@YAXKKKPB_K@Z=RaiseException"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?SetUnhandledExceptionFilter@win32@detail@signal_guard@_",                             \
                                                        "@quickcpplib@@YAP6AJPAU_EXCEPTION_POINTERS@12345@@ZP6AJ0@Z@Z=SetUnhandledExceptionFilter"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?AddVectoredContinueHandler@win32@detail@signal_guard@_",                              \
                                                        "@quickcpplib@@YAPAXKP6AJPAU_EXCEPTION_POINTERS@12345@@Z@Z=AddVectoredContinueHandler"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?RemoveVectoredContinueHandler@win32@detail@signal_guard@_",                           \
                                                        "@quickcpplib@@YAKPAX@Z=RemoveVectoredContinueHandler"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?GetLastError@win32@detail@signal_guard@_", "@quickcpplib@@YAKXZ=GetLastError"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?SetConsoleCtrlHandler@win32@detail@signal_guard@_",                                   \
                                                        "@quickcpplib@@YAHP6AHK@ZH@Z=SetConsoleCtrlHandler"))
#pragma comment(linker,                                                                                                                                        \
                QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?TerminateProcess@win32@detail@signal_guard@_", "@quickcpplib@@YAHPAXI@Z=TerminateProcess"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?GetStdHandle@win32@detail@signal_guard@_", "@quickcpplib@@YAPAXK@Z=GetStdHandle"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?WriteFile@win32@detail@signal_guard@_",                                               \
                                                        "@quickcpplib@@YAHPAXPBXKPAKPAUOVERLAPPED@12345@@Z=WriteFile"))
#pragma comment(                                                                                                                                               \
linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?WriteFile@win32@detail@signal_guard@_", "@quickcpplib@@YAHPAXPBXKPAKPAUOVERLAPPED@@@Z=WriteFile"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?CreateThread@win32@detail@signal_guard@_",                                            \
                                                        "@quickcpplib@@YAPAXPAU_SECURITY_ATTRIBUTES@12345@IP6AKPAX@Z1KPAK@Z=CreateThread"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?CreateThread@win32@detail@signal_guard@_",                                            \
                                                        "@quickcpplib@@YAPAXPAU_SECURITY_ATTRIBUTES@@IP6AKPAX@Z1KPAK@Z=CreateThread"))
#pragma comment(linker,                                                                                                                                        \
                QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?QueueUserAPC@win32@detail@signal_guard@_", "@quickcpplib@@YAKP6AXI@ZPAXI@Z=QueueUserAPC"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?SleepEx@win32@detail@signal_guard@_", "@quickcpplib@@YAKKH@Z=SleepEx"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?CloseHandle@win32@detail@signal_guard@_", "@quickcpplib@@YAHPAX@Z=CloseHandle"))
#pragma comment(linker, QUICKCPPLIB_SIGNAL_GUARD_SYMBOL("/alternatename:?GetTickCount64@win32@detail@signal_guard@_", "@quickcpplib@@YA_KXZ=GetTickCount64"))
#else
#error Unknown architecture
#endif
#else
#if(defined(__x86_64__) || defined(_M_X64)) || (defined(__aarch64__) || defined(_M_ARM64))
#pragma comment(linker, "/alternatename:?RaiseException@win32@detail@signal_guard@quickcpplib@@YAXKKKPEB_K@Z=RaiseException")
#pragma comment(                                                                                                                                               \
linker,                                                                                                                                                        \
"/alternatename:?SetUnhandledExceptionFilter@win32@detail@signal_guard@quickcpplib@@YAP6AJPEAU_EXCEPTION_POINTERS@1234@@ZP6AJ0@Z@Z=SetUnhandledExceptionFilter")
#pragma comment(                                                                                                                                               \
linker,                                                                                                                                                        \
"/alternatename:?AddVectoredContinueHandler@win32@detail@signal_guard@quickcpplib@@YAPEAXKP6AJPEAU_EXCEPTION_POINTERS@1234@@Z@Z=AddVectoredContinueHandler")
#pragma comment(linker, "/alternatename:?RemoveVectoredContinueHandler@win32@detail@signal_guard@quickcpplib@@YAKPEAX@Z=RemoveVectoredContinueHandler")
#pragma comment(linker, "/alternatename:?GetLastError@win32@detail@signal_guard@quickcpplib@@YAKXZ=GetLastError")
#pragma comment(linker, "/alternatename:?SetConsoleCtrlHandler@win32@detail@signal_guard@quickcpplib@@YAHP6AHK@ZH@Z=SetConsoleCtrlHandler")
#pragma comment(linker, "/alternatename:?TerminateProcess@win32@detail@signal_guard@quickcpplib@@YAHPEAXI@Z=TerminateProcess")
#pragma comment(linker, "/alternatename:?GetStdHandle@win32@detail@signal_guard@quickcpplib@@YAPEAXK@Z=GetStdHandle")
#pragma comment(linker, "/alternatename:?WriteFile@win32@detail@signal_guard@quickcpplib@@YAHPEAXPEBXKPEAKPEAU_OVERLAPPED@1234@@Z=WriteFile")
#pragma comment(linker, "/alternatename:?WriteFile@win32@detail@signal_guard@quickcpplib@@YAHPEAXPEBXKPEAKPEAU_OVERLAPPED@@@Z=WriteFile")
#pragma comment(linker,                                                                                                                                        \
                "/alternatename:?CreateThread@win32@detail@signal_guard@quickcpplib@@YAPEAXPEAU_SECURITY_ATTRIBUTES@1234@_KP6AKPEAX@Z2KPEAK@Z=CreateThread")
#pragma comment(linker, "/alternatename:?CreateThread@win32@detail@signal_guard@quickcpplib@@YAPEAXPEAU_SECURITY_ATTRIBUTES@@_KP6AKPEAX@Z2KPEAK@Z=CreateThread")
#pragma comment(linker, "/alternatename:?QueueUserAPC@win32@detail@signal_guard@quickcpplib@@YAKP6AX_K@ZPEAX0@Z=QueueUserAPC")
#pragma comment(linker, "/alternatename:?SleepEx@win32@detail@signal_guard@quickcpplib@@YAKKH@Z=SleepEx")
#pragma comment(linker, "/alternatename:?CloseHandle@win32@detail@signal_guard@quickcpplib@@YAHPEAX@Z=CloseHandle")
#pragma comment(linker, "/alternatename:?GetTickCount64@win32@detail@signal_guard@quickcpplib@@YA_KXZ=GetTickCount64")
#elif defined(__x86__) || defined(_M_IX86) || defined(__i386__)
#pragma comment(linker, "/alternatename:?RaiseException@win32@detail@signal_guard@quickcpplib@@YGXKKKPB_K@Z=__imp__RaiseException@16")
#pragma comment(                                                                                                                                               \
linker,                                                                                                                                                        \
"/alternatename:?SetUnhandledExceptionFilter@win32@detail@signal_guard@quickcpplib@@YGP6GJPAU_EXCEPTION_POINTERS@@@ZP6GJ0@Z@Z=__imp__SetUnhandledExceptionFilter@4")
#pragma comment(                                                                                                                                               \
linker,                                                                                                                                                        \
"/alternatename:?AddVectoredContinueHandler@win32@detail@signal_guard@quickcpplib@@YGPAXKP6GJPAU_EXCEPTION_POINTERS@@@Z@Z=__imp__AddVectoredContinueHandler@8")
#pragma comment(linker, "/alternatename:?RemoveVectoredContinueHandler@win32@detail@signal_guard@quickcpplib@@YGKPAX@Z=__imp__RemoveVectoredContinueHandler@4")
#pragma comment(linker, "/alternatename:?GetLastError@win32@detail@signal_guard@quickcpplib@@YGKXZ=__imp__GetLastError@0")
#pragma comment(linker, "/alternatename:?SetConsoleCtrlHandler@win32@detail@signal_guard@quickcpplib@@YGHP6GHK@ZH@Z=__imp__SetConsoleCtrlHandler@8")
#pragma comment(linker, "/alternatename:?TerminateProcess@win32@detail@signal_guard@quickcpplib@@YGHPAXI@Z=__imp__TerminateProcess@8")
#pragma comment(linker, "/alternatename:?GetStdHandle@win32@detail@signal_guard@quickcpplib@@YGPAXK@Z=__imp__GetStdHandle@4")
#pragma comment(linker, "/alternatename:?WriteFile@win32@detail@signal_guard@quickcpplib@@YGHPAXPBXKPAKPAU_OVERLAPPED@1234@@Z=__imp__WriteFile@20")
#pragma comment(linker, "/alternatename:?WriteFile@win32@detail@signal_guard@quickcpplib@@YGHPAXPBXKPAKPAU_OVERLAPPED@@@Z=__imp__WriteFile@20")
#pragma comment(                                                                                                                                               \
linker, "/alternatename:?CreateThread@win32@detail@signal_guard@quickcpplib@@YGPAXPAU_SECURITY_ATTRIBUTES@1234@IP6GKPAX@Z1KPAK@Z=__imp__CreateThread@24")
#pragma comment(linker,                                                                                                                                        \
                "/alternatename:?CreateThread@win32@detail@signal_guard@quickcpplib@@YGPAXPAU_SECURITY_ATTRIBUTES@@IP6GKPAX@Z1KPAK@Z=__imp__CreateThread@24")
#pragma comment(linker, "/alternatename:?QueueUserAPC@win32@detail@signal_guard@quickcpplib@@YGKP6GXI@ZPAXI@Z=__imp__QueueUserAPC@12")
#pragma comment(linker, "/alternatename:?SleepEx@win32@detail@signal_guard@quickcpplib@@YGKKH@Z=__imp__SleepEx@8")
#pragma comment(linker, "/alternatename:?CloseHandle@win32@detail@signal_guard@quickcpplib@@YGHPAX@Z=__imp__CloseHandle@4")
#pragma comment(linker, "/alternatename:?GetTickCount64@win32@detail@signal_guard@quickcpplib@@YG_KXZ=__imp__GetTickCount64@0")
#elif defined(__arm__) || defined(_M_ARM)
#pragma comment(linker, "/alternatename:?RaiseException@win32@detail@signal_guard@quickcpplib@@YAXKKKPB_K@Z=RaiseException")
#pragma comment(                                                                                                                                               \
linker,                                                                                                                                                        \
"/alternatename:?SetUnhandledExceptionFilter@win32@detail@signal_guard@quickcpplib@@YAP6AJPAU_EXCEPTION_POINTERS@1234@@ZP6AJ0@Z@Z=SetUnhandledExceptionFilter")
#pragma comment(                                                                                                                                               \
linker,                                                                                                                                                        \
"/alternatename:?AddVectoredContinueHandler@win32@detail@signal_guard@quickcpplib@@YAPAXKP6AJPAU_EXCEPTION_POINTERS@1234@@Z@Z=AddVectoredContinueHandler")
#pragma comment(linker, "/alternatename:?RemoveVectoredContinueHandler@win32@detail@signal_guard@quickcpplib@@YAKPAX@Z=RemoveVectoredContinueHandler")
#pragma comment(linker, "/alternatename:?GetLastError@win32@detail@signal_guard@quickcpplib@@YAKXZ=GetLastError")
#pragma comment(linker, "/alternatename:?SetConsoleCtrlHandler@win32@detail@signal_guard@quickcpplib@@YAHP6AHK@ZH@Z=SetConsoleCtrlHandler")
#pragma comment(linker, "/alternatename:?TerminateProcess@win32@detail@signal_guard@quickcpplib@@YAHPAXI@Z=TerminateProcess")
#pragma comment(linker, "/alternatename:?GetStdHandle@win32@detail@signal_guard@quickcpplib@@YAPAXK@Z=GetStdHandle")
#pragma comment(linker, "/alternatename:?WriteFile@win32@detail@signal_guard@quickcpplib@@YAHPAXPBXKPAKPAUOVERLAPPED@1234@@Z=WriteFile")
#pragma comment(linker, "/alternatename:?WriteFile@win32@detail@signal_guard@quickcpplib@@YAHPAXPBXKPAKPAUOVERLAPPED@@@Z=WriteFile")
#pragma comment(linker, "/alternatename:?CreateThread@win32@detail@signal_guard@quickcpplib@@YAPAXPAU_SECURITY_ATTRIBUTES@1234@IP6AKPAX@Z1KPAK@Z=CreateThread")
#pragma comment(linker, "/alternatename:?CreateThread@win32@detail@signal_guard@quickcpplib@@YAPAXPAU_SECURITY_ATTRIBUTES@@IP6AKPAX@Z1KPAK@Z=CreateThread")
#pragma comment(linker, "/alternatename:?QueueUserAPC@win32@detail@signal_guard@quickcpplib@@YAKP6AXI@ZPAXI@Z=QueueUserAPC")
#pragma comment(linker, "/alternatename:?SleepEx@win32@detail@signal_guard@quickcpplib@@YAKKH@Z=SleepEx")
#pragma comment(linker, "/alternatename:?CloseHandle@win32@detail@signal_guard@quickcpplib@@YAHPAX@Z=CloseHandle")
#pragma comment(linker, "/alternatename:?GetTickCount64@win32@detail@signal_guard@quickcpplib@@YA_KXZ=GetTickCount64")
#else
#error Unknown architecture
#endif
#endif
    }  // namespace win32
    inline unsigned long win32_exception_code_from_signalc(signalc c)
    {
      switch(c)
      {
      default:
        abort();
      case signalc::abort_process:
        return ((unsigned long) 0xC0000025L) /*EXCEPTION_NONCONTINUABLE_EXCEPTION*/;
      case signalc::undefined_memory_access:
        return ((unsigned long) 0xC0000006L) /*EXCEPTION_IN_PAGE_ERROR*/;
      case signalc::illegal_instruction:
        return ((unsigned long) 0xC000001DL) /*EXCEPTION_ILLEGAL_INSTRUCTION*/;
      // case signalc::interrupt:
      //  return SIGINT;
      // case signalc::broken_pipe:
      //  return SIGPIPE;
      case signalc::segmentation_fault:
        return ((unsigned long) 0xC0000005L) /*EXCEPTION_ACCESS_VIOLATION*/;
      case signalc::floating_point_error:
        return ((unsigned long) 0xC0000090L) /*EXCEPTION_FLT_INVALID_OPERATION*/;
      }
    }
    inline signalc signalc_from_win32_exception_code(unsigned long c)
    {
      switch(c)
      {
      case((unsigned long) 0xC0000025L) /*EXCEPTION_NONCONTINUABLE_EXCEPTION*/:
        return signalc::abort_process;
      case((unsigned long) 0xC0000006L) /*EXCEPTION_IN_PAGE_ERROR*/:
        return signalc::undefined_memory_access;
      case((unsigned long) 0xC000001DL) /*EXCEPTION_ILLEGAL_INSTRUCTION*/:
        return signalc::illegal_instruction;
      // case SIGINT:
      //  return signalc::interrupt;
      // case SIGPIPE:
      //  return signalc::broken_pipe;
      case((unsigned long) 0xC0000005L) /*EXCEPTION_ACCESS_VIOLATION*/:
        return signalc::segmentation_fault;
      case((unsigned long) 0xC000008DL) /*EXCEPTION_FLT_DENORMAL_OPERAND*/:
      case((unsigned long) 0xC000008EL) /*EXCEPTION_FLT_DIVIDE_BY_ZERO*/:
      case((unsigned long) 0xC000008FL) /*EXCEPTION_FLT_INEXACT_RESULT*/:
      case((unsigned long) 0xC0000090L) /*EXCEPTION_FLT_INVALID_OPERATION*/:
      case((unsigned long) 0xC0000091L) /*EXCEPTION_FLT_OVERFLOW*/:
      case((unsigned long) 0xC0000092L) /*EXCEPTION_FLT_STACK_CHECK*/:
      case((unsigned long) 0xC0000093L) /*EXCEPTION_FLT_UNDERFLOW*/:
        return signalc::floating_point_error;
      case((unsigned long) 0xC00000FDL) /*EXCEPTION_STACK_OVERFLOW*/:
        return signalc::cxx_out_of_memory;
      default:
        return signalc::none;
      }
    }
    // Overload for when a Win32 exception is being raised
    inline bool set_siginfo(thread_local_signal_guard *g, unsigned long code, win32::PEXCEPTION_RECORD raw_info, win32::PCONTEXT raw_context)
    {
      auto signo = signalc_from_win32_exception_code(code);
      auto signalset = static_cast<signalc_set>(1ULL << static_cast<int>(signo));
      if(g->guarded & signalset)
      {
        g->info.signo = static_cast<int>(signo);
        g->info.error_code = (long) raw_info->ExceptionInformation[2];
        g->info.addr = (void *) raw_info->ExceptionInformation[1];
        g->info.value.ptr_value = nullptr;
        g->info.raw_info = raw_info;
        g->info.raw_context = raw_context;
        return true;
      }
      return false;
    }
    SIGNALGUARD_FUNC_DECL long win32_exception_filter_function(unsigned long code, win32::_EXCEPTION_POINTERS *ptrs) noexcept
    {
      for(auto *shi = current_thread_local_signal_handler(); shi != nullptr; shi = shi->previous)
      {
        if(set_siginfo(shi, code, ptrs->ExceptionRecord, ptrs->ContextRecord))
        {
          if(shi->call_continuer())
          {
            // continue execution
            return (long) -1 /*EXCEPTION_CONTINUE_EXECUTION*/;
          }
          else
          {
            // invoke longjmp
            return 1 /*EXCEPTION_EXECUTE_HANDLER*/;
          }
        }
      }
      return 0 /*EXCEPTION_CONTINUE_SEARCH*/;
    }
    SIGNALGUARD_FUNC_DECL long __stdcall win32_vectored_exception_function(win32::_EXCEPTION_POINTERS *ptrs) noexcept
    {
      _state().lock.lock();
      if(_state().global_signal_deciders_front != nullptr)
      {
        auto *raw_info = ptrs->ExceptionRecord;
        auto *raw_context = ptrs->ContextRecord;
        const auto signo = signalc_from_win32_exception_code(raw_info->ExceptionCode);
        const signalc_set signo_set = 1ULL << static_cast<int>(signo);
        raised_signal_info rsi;
        memset(&rsi, 0, sizeof(rsi));
        rsi.signo = static_cast<int>(signo);
        rsi.error_code = (long) raw_info->ExceptionInformation[2];
        rsi.addr = (void *) raw_info->ExceptionInformation[1];
        rsi.raw_info = raw_info;
        rsi.raw_context = raw_context;
        auto *d = _state().global_signal_deciders_front;
        for(size_t n = 0; d != nullptr; n++)
        {
          size_t i = 0;
          for(d = _state().global_signal_deciders_front; d != nullptr; d = d->next)
          {
            if(i++ == n)
            {
              if(d->guarded & signo_set)
              {
                rsi.value = d->value;
                _state().lock.unlock();
                if(d->decider(&rsi))
                {
                  return (long) -1 /*EXCEPTION_CONTINUE_EXECUTION*/;
                }
                _state().lock.lock();
                break;
              }
              n++;
            }
          }
        }
      }
      _state().lock.unlock();
      return 0 /*EXCEPTION_CONTINUE_SEARCH*/;
    }
#else
    struct installed_signal_handler
    {
      unsigned count;  // number of signal_install instances for this signal
      struct sigaction former;
    };
    static installed_signal_handler handler_counts[32];  // indexed by signal number

    // Simulate the raising of a signal
    inline bool do_raise_signal(int signo, struct sigaction &sa, siginfo_t *_info, void *_context)
    {
      void (*h1)(int signo, siginfo_t *info, void *context) = nullptr;
      void (*h2)(int signo) = nullptr;
      siginfo_t info;
      ucontext_t context;
      // std::cout << "do_raise_signal(" << signo << ", " << (void *) sa.sa_handler << ", " << info << ", " << context << ")" << std::endl;
      if(sa.sa_handler == SIG_IGN)
      {
        // std::cout << "ignore" << std::endl;
        return false;
      }
      else if(sa.sa_handler == SIG_DFL)
      {
        // std::cout << "default" << std::endl;
        // Default action for these is to ignore
        if(signo == SIGCHLD || signo == SIGURG
#ifdef SIGWINCH
           || signo == SIGWINCH
#endif
#if !defined(__linux__) && defined(SIGIO)
           || signo == SIGIO
#endif
#ifdef SIGINFO
           || signo == SIGINFO
#endif
        )
          return false;
#if 1
        // Simulate invoke the default handler. We preserve semantics - ish.
        // Default ignored signals already are handled above, so just disambiguate between core dumping, terminate, and immediate exit signals
        switch(signo)
        {
        // Core dump signals
        case SIGABRT:
        case SIGBUS:
        case SIGFPE:
        case SIGILL:
        case SIGQUIT:
        case SIGSEGV:
        case SIGSYS:
        case SIGTRAP:
#ifdef __linux__
        case SIGXCPU:
        case SIGXFSZ:
#endif
          // glibc's abort() implementation does a ton of stuff to ensure it always works no matter from where it is called :)
          abort();

        // Termination signals
        case SIGSTOP:
        case SIGTSTP:
        case SIGTTIN:
        case SIGTTOU:
        case SIGPIPE:
        case SIGALRM:
        case SIGTERM:
#ifdef __linux__
        case SIGIO:
#endif
#if defined(__FreeBSD__) || defined(__APPLE__)
        case SIGXCPU:
        case SIGXFSZ:
#endif
#ifdef SIGVTALRM
        case SIGVTALRM:
#endif
#ifdef SIGPROF
        case SIGPROF:
#endif
        case SIGUSR1:
        case SIGUSR2:
#ifdef SIGTHR
        case SIGTHR:
#endif
#ifdef SIGLIBRT
        case SIGLIBRT:
#endif
#ifdef SIGEMT
        case SIGEMT:
#endif
#ifdef SIGPWR
        case SIGPWR:
#endif
          // Immediate exit without running cleanup
          _exit(127);  // compatibility code with glibc's default signal handler

        // Immediate exit running cleanup
        default:
          ::exit(EXIT_SUCCESS);
        }
#else
        // This is the original implementation which appears to sometimes hang in pthread_kill() for no obvious reason
        // Ok, need to invoke the default handler. NOTE: The following code is racy wrt other threads manipulating the global signal handlers
        struct sigaction dfl, myformer;
        memset(&dfl, 0, sizeof(dfl));
        dfl.sa_handler = SIG_DFL;
        sigaction(signo, &dfl, &myformer);  // restore action to default
        // Unblock this signal for this thread
        sigset_t myformer2;
        sigset_t def2;
        sigemptyset(&def2);
        sigaddset(&def2, signo);
        pthread_sigmask(SIG_UNBLOCK, &def2, &myformer2);
        // Raise this signal for this thread, invoking the default action
        pthread_kill(pthread_self(), signo);   // Very likely never returns
        sigaction(signo, &myformer, nullptr);  // but if it does, restore the previous action
#endif
        return true;  // and return true to indicate that we executed some signal handler
      }
      else if(sa.sa_flags & SA_SIGINFO)
      {
        h1 = sa.sa_sigaction;
        if(nullptr != _info)
        {
          info = *_info;
        }
        if(nullptr != _context)
        {
          context = *(const ucontext_t *) _context;
        }
      }
      else
      {
        h2 = sa.sa_handler;
      }
      // std::cout << "handler" << std::endl;
      if(sa.sa_flags & SA_ONSTACK)
      {
        // Not implemented yet
        abort();
      }
      if(sa.sa_flags & SA_RESETHAND)
      {
        struct sigaction dfl;
        memset(&dfl, 0, sizeof(dfl));
        dfl.sa_handler = SIG_DFL;
        sigaction(signo, &dfl, nullptr);
      }
      if(!(sa.sa_flags & (SA_RESETHAND | SA_NODEFER)))
      {
        sigaddset(&sa.sa_mask, signo);
      }
      sigset_t oldset;
      pthread_sigmask(SIG_BLOCK, &sa.sa_mask, &oldset);
      if(h1 != nullptr)
        h1(signo, (nullptr != _info) ? &info : nullptr, (nullptr != _context) ? &context : nullptr);
      else
        h2(signo);
      pthread_sigmask(SIG_SETMASK, &oldset, nullptr);
      return true;
    }

    // Overload for when a POSIX signal is being raised
    inline bool set_siginfo(thread_local_signal_guard *g, int signo, siginfo_t *raw_info, void *raw_context)
    {
      auto signalset = static_cast<signalc_set>(1ULL << signo);
      if(g->guarded & signalset)
      {
        g->info.signo = signo;
        g->info.error_code = (nullptr != raw_info) ? raw_info->si_errno : 0;
        g->info.addr = (nullptr != raw_info) ? raw_info->si_addr : nullptr;
        g->info.value.ptr_value = nullptr;
        g->info.raw_info = raw_info;
        g->info.raw_context = raw_context;
        return true;
      }
      return false;
    }
    // Called by POSIX to handle a raised signal
    inline void raw_signal_handler(int signo, siginfo_t *info, void *context)
    {
      // std::cout << "raw_signal_handler(" << signo << ", " << info << ", " << context << ") shi=" << shi << std::endl;
      for(auto *shi = current_thread_local_signal_handler(); shi != nullptr; shi = shi->previous)
      {
        if(set_siginfo(shi, signo, info, context))
        {
          if(!shi->call_continuer())
          {
            longjmp(shi->info.buf, 1);
          }
        }
      }
      _state().lock.lock();
      if(_state().global_signal_deciders_front != nullptr)
      {
        raised_signal_info rsi;
        memset(&rsi, 0, sizeof(rsi));
        rsi.signo = signo;
        rsi.error_code = (nullptr != info) ? info->si_errno : 0;
        rsi.addr = (nullptr != info) ? info->si_addr : nullptr;
        rsi.raw_info = info;
        rsi.raw_context = context;
        auto *d = _state().global_signal_deciders_front;
        for(size_t n = 0; d != nullptr; n++)
        {
          size_t i = 0;
          for(d = _state().global_signal_deciders_front; d != nullptr; d = d->next)
          {
            if(i++ == n)
            {
              if(d->guarded & (1ULL << signo))
              {
                rsi.value = d->value;
                _state().lock.unlock();
                if(d->decider(&rsi))
                {
                  return;  // resume execution
                }
                _state().lock.lock();
                break;
              }
              n++;
            }
          }
        }
      }
      // Otherwise, call the previous signal handler
      if(handler_counts[signo].count > 0)
      {
        struct sigaction sa;
        sa = handler_counts[signo].former;
        _state().lock.unlock();
        do_raise_signal(signo, sa, info, context);
      }
      else
      {
        _state().lock.unlock();
      }
    }
#endif
  }  // namespace detail

  SIGNALGUARD_MEMFUNC_DECL signal_guard_install::signal_guard_install(signalc_set guarded)
      : _guarded(guarded)
  {
#ifndef _WIN32
    sigset_t set;
    sigemptyset(&set);
    detail::_state().lock.lock();
    for(int signo = 0; signo < 32; signo++)
    {
      if((static_cast<uint64_t>(guarded) & (1 << signo)) != 0)
      {
        if(!detail::handler_counts[signo].count++)
        {
          struct sigaction sa;
          memset(&sa, 0, sizeof(sa));
          sa.sa_sigaction = detail::raw_signal_handler;
          sa.sa_flags = SA_SIGINFO | SA_NODEFER;
          if(-1 == sigaction(signo, &sa, &detail::handler_counts[signo].former))
          {
            detail::handler_counts[signo].count--;
            detail::_state().lock.unlock();
            throw std::system_error(errno, std::system_category());
          }
        }
      }
      if(detail::handler_counts[signo].count > 0)
      {
        sigaddset(&set, signo);
      }
    }
    // Globally enable all signals we have installed handlers for
    if(-1 == sigprocmask(SIG_UNBLOCK, &set, nullptr))
    {
      detail::_state().lock.unlock();
      throw std::system_error(errno, std::system_category());
    }
    detail::signal_guards_installed().store(detail::signal_guards_installed().load(std::memory_order_relaxed) | guarded, std::memory_order_relaxed);
    detail::_state().lock.unlock();
#endif
    if((guarded & signalc_set::cxx_out_of_memory) || (guarded & signalc_set::cxx_termination)
#ifdef _WIN32
       || (guarded & signalc_set::interrupt) || (guarded & signalc_set::process_terminate)
#endif
    )
    {
      detail::_state().lock.lock();
      if(guarded & signalc_set::cxx_out_of_memory)
      {
        if(!detail::_state().new_handler_count++)
        {
          detail::_state().new_handler_old = std::set_new_handler(detail::new_handler);
        }
      }
      if(guarded & signalc_set::cxx_termination)
      {
        if(!detail::_state().terminate_handler_count++)
        {
#ifndef _MSC_VER
          if(std::get_terminate() != detail::terminate_handler)
          {
            detail::terminate_handler_old() = std::set_terminate(detail::terminate_handler);
          }
#endif
        }
      }
#ifdef _WIN32
      if((guarded & signalc_set::interrupt) || (guarded & signalc_set::process_terminate))
      {
        if(!detail::_state().win32_console_ctrl_handler_count++)
        {
          if(!detail::win32::SetConsoleCtrlHandler(detail::win32_console_ctrl_handler, true))
          {
            throw std::system_error(detail::win32::GetLastError(), std::system_category());
          }
        }
      }
#endif
      detail::_state().lock.unlock();
    }
  }

  SIGNALGUARD_MEMFUNC_DECL signal_guard_install::~signal_guard_install()
  {
    if((_guarded & signalc_set::cxx_out_of_memory) || (_guarded & signalc_set::cxx_termination)
#ifdef _WIN32
       || (_guarded & signalc_set::interrupt) || (_guarded & signalc_set::process_terminate)
#endif
    )
    {
      detail::_state().lock.lock();
      if(_guarded & signalc_set::cxx_out_of_memory)
      {
        if(!--detail::_state().new_handler_count)
        {
          std::set_new_handler(detail::_state().new_handler_old);
        }
      }
      if(_guarded & signalc_set::cxx_termination)
      {
        if(!--detail::_state().terminate_handler_count)
        {
#ifndef _MSC_VER
          std::set_terminate(detail::terminate_handler_old());
#endif
        }
      }
#ifdef _WIN32
      if((_guarded & signalc_set::interrupt) || (_guarded & signalc_set::process_terminate))
      {
        if(!--detail::_state().win32_console_ctrl_handler_count)
        {
          detail::win32::SetConsoleCtrlHandler(detail::win32_console_ctrl_handler, false);
        }
      }
#endif
      detail::_state().lock.unlock();
    }
#ifndef _WIN32
    uint64_t handlers_installed = 0;
    sigset_t set;
    sigemptyset(&set);
    bool setsigprocmask = false;
    detail::_state().lock.lock();
    for(int signo = 0; signo < 32; signo++)
    {
      if((static_cast<uint64_t>(_guarded) & (1 << signo)) != 0)
      {
        int ret = 0;
        if(!--detail::handler_counts[signo].count)
        {
          ret = sigaction(signo, &detail::handler_counts[signo].former, nullptr);
          if(ret == -1)
          {
            detail::_state().lock.unlock();
            abort();
            detail::_state().lock.lock();
          }
          sigaddset(&set, signo);
          setsigprocmask = true;
        }
      }
      if(detail::handler_counts[signo].count > 0)
      {
        handlers_installed |= (1ULL << signo);
      }
    }
    if(detail::_state().new_handler_count > 0)
    {
      handlers_installed |= signalc_set::cxx_out_of_memory;
    }
    if(detail::_state().terminate_handler_count > 0)
    {
      handlers_installed |= signalc_set::cxx_termination;
    }
    detail::signal_guards_installed().store(static_cast<signalc_set>(handlers_installed), std::memory_order_relaxed);
    detail::_state().lock.unlock();
    if(setsigprocmask)
    {
      sigprocmask(SIG_BLOCK, &set, nullptr);
    }
#endif
  }

  namespace detail
  {
    static inline bool signal_guard_global_decider_impl_indirect(raised_signal_info *rsi)
    {
      auto *p = (signal_guard_global_decider_impl *) rsi->value.ptr_value;
      return p->_call(rsi);
    }
    SIGNALGUARD_MEMFUNC_DECL signal_guard_global_decider_impl::signal_guard_global_decider_impl(signalc_set guarded, bool callfirst)
        : _install(guarded)
    {
      auto *p = new global_signal_decider;
      _p = p;
      p->guarded = guarded;
      p->decider = signal_guard_global_decider_impl_indirect;
      p->value.ptr_value = this;
      _state().lock.lock();
      global_signal_decider **a, **b;
      if(_state().global_signal_deciders_front == nullptr)
      {
        a = &_state().global_signal_deciders_front;
        b = &_state().global_signal_deciders_back;
      }
      else if(callfirst)
      {
        a = &_state().global_signal_deciders_front;
        b = &_state().global_signal_deciders_front->prev;
      }
      else
      {
        a = &_state().global_signal_deciders_back->next;
        b = &_state().global_signal_deciders_back;
      }
      p->next = *a;
      p->prev = *b;
      *a = p;
      *b = p;
#ifdef _WIN32
      if(nullptr == _state().win32_global_signal_decider2)
      {
        /* The interaction between AddVectoredContinueHandler, AddVectoredExceptionHandler,
        UnhandledExceptionFilter, and frame-based EH is completely undocumented in Microsoft
        documentation. The following is the truth, as determined by empirical testing:

        1. Vectored exception handlers get called first, before anything else, including
        frame-based EH. This is not what the MSDN documentation hints at.

        2. Frame-based EH filters are now run.

        3. UnhandledExceptionFilter() is now called. On older Windows, this invokes the
        debugger if being run under the debugger, otherwise continues search. But as of
        at least Windows 7 onwards, if no debugger is attached, it invokes Windows
        Error Reporting to send a core dump to Microsoft.

        4. Vectored continue handlers now get called, AFTER the frame-based EH. Again,
        not what MSDN hints at.


        The change in the default non-debugger behaviour of UnhandledExceptionFilter()
        effectively makes vectored continue handlers useless. I suspect whomever made
        the change at Microsoft didn't realise that vectored continue handlers are
        invoked AFTER the unhandled exception filter, because that's really non-obvious
        from the documentation.

        Anyway this is why we install for both the continue handler and the unhandled
        exception filters. The unhandled exception filter will be called when not running
        under a debugger. The vectored continue handler will be called when running
        under a debugger, as the UnhandledExceptionFilter() function never calls the
        installed unhandled exception filter function if under a debugger.
        */
        _state().win32_global_signal_decider1 = (void *) win32::SetUnhandledExceptionFilter(win32_vectored_exception_function);
        _state().win32_global_signal_decider2 = (void *) win32::AddVectoredContinueHandler(1, win32_vectored_exception_function);
      }
#endif
      _state().lock.unlock();
    }

    SIGNALGUARD_MEMFUNC_DECL signal_guard_global_decider_impl::signal_guard_global_decider_impl(signal_guard_global_decider_impl &&o) noexcept
        : _install(static_cast<signal_guard_global_decider_impl &&>(o)._install)
        , _p(o._p)
    {
      _state().lock.lock();
      auto *p = (global_signal_decider *) _p;
      p->value.ptr_value = this;
      _state().lock.unlock();
      o._p = nullptr;
    }

    SIGNALGUARD_MEMFUNC_DECL signal_guard_global_decider_impl::~signal_guard_global_decider_impl()
    {
      if(_p != nullptr)
      {
        auto *p = (global_signal_decider *) _p;
        _state().lock.lock();
        global_signal_decider **a = (nullptr == p->prev) ? &_state().global_signal_deciders_front : &p->prev->next;
        global_signal_decider **b = (nullptr == p->next) ? &_state().global_signal_deciders_back : &p->next->prev;
        *a = p->next;
        *b = p->prev;
#ifdef _WIN32
        if(nullptr == _state().global_signal_deciders_front)
        {
          win32::RemoveVectoredContinueHandler(_state().win32_global_signal_decider2);
          win32::SetUnhandledExceptionFilter((win32::PVECTORED_EXCEPTION_HANDLER) _state().win32_global_signal_decider1);
          _state().win32_global_signal_decider1 = nullptr;
          _state().win32_global_signal_decider2 = nullptr;
        }
#endif
        _state().lock.unlock();
        delete p;
        _p = nullptr;
      }
    }
  }  // namespace detail

  SIGNALGUARD_FUNC_DECL bool thrd_raise_signal(signalc signo, void *_info, void *_context)
  {
    if(signo == signalc::cxx_out_of_memory)
    {
      if(std::get_new_handler() == nullptr)
      {
        std::terminate();
      }
      std::get_new_handler()();
      return true;
    }
    else if(signo == signalc::cxx_termination)
    {
      std::terminate();
    }
#ifdef _WIN32
    using detail::win32::_EXCEPTION_RECORD;
    using detail::win32::RaiseException;
    auto win32sehcode = detail::win32_exception_code_from_signalc(signo);
    if((unsigned long) -1 == win32sehcode)
      throw std::runtime_error("Unknown signal");
    (void) _context;
    const auto *info = (const _EXCEPTION_RECORD *) _info;
    // info->ExceptionInformation[0] = 0=read 1=write 8=DEP
    // info->ExceptionInformation[1] = causing address
    // info->ExceptionInformation[2] = NTSTATUS causing exception
    if(info != nullptr)
    {
      RaiseException(win32sehcode, info->ExceptionFlags, info->NumberParameters, info->ExceptionInformation);
    }
    else
    {
      RaiseException(win32sehcode, 0, 0, nullptr);
    }
    return false;
#else
    auto *info = (siginfo_t *) _info;
    // Fetch the current handler, and simulate the raise
    struct sigaction sa;
    sigaction(static_cast<int>(signo), nullptr, &sa);
    return detail::do_raise_signal(static_cast<int>(signo), sa, info, _context);
#endif
  }

  SIGNALGUARD_FUNC_DECL void terminate_process_immediately(const char *msg) noexcept
  {
    static const char s[] = "\nProcess fail fast terminated\n";
    if(msg == nullptr)
    {
      msg = s;
    }
#ifdef _WIN32
    if(msg != nullptr && msg[0] != 0)
    {
      unsigned long written = 0;
      (void) detail::win32::WriteFile(detail::win32::GetStdHandle((unsigned long) -12 /*STD_ERROR_HANDLE*/), msg, (unsigned long) strlen(msg), &written,
                                      nullptr);
    }
    detail::win32::TerminateProcess((void *) -1, 1);
#else
    if(msg != nullptr && msg[0] != 0)
    {
      if(-1 == ::write(2, msg, strlen(msg)))
      {
        volatile int a = 1;  // shut up GCC
        (void) a;
      }
    }
    _exit(1);
#endif
    // We promised we would never, ever, return.
    for(;;)
    {
    }
  }

  namespace detail
  {
    static struct watchdog_decider_t
    {
      configurable_spinlock::spinlock<uintptr_t> lock;
      signal_guard_watchdog_impl *next{nullptr};
      bool check() noexcept
      {
        bool ret = false;
#ifdef _WIN32
        auto now = detail::win32::GetTickCount64();
#else
        struct timespec ts;
        memset(&ts, 0, sizeof(ts));
        (void) ::clock_gettime(CLOCK_MONOTONIC, &ts);
        auto now = ((uint64_t) ts.tv_sec * 1000ULL + (uint64_t) ts.tv_nsec / 1000000ULL);
#endif
        lock.lock();
        auto *d = next;
        for(size_t n = 0; d != nullptr; n++)
        {
          size_t i = 0;
          for(d = next; d != nullptr; d = d->_next)
          {
            if(i++ == n)
            {
              if(now >= d->_deadline_ms && !d->_alreadycalled)
              {
                d->_alreadycalled = true;
                lock.unlock();
                d->_call();
                ret = true;
                lock.lock();
                break;
              }
              n++;
            }
          }
        }
        lock.unlock();
        return ret;
      }
    } watchdog_decider;
#ifdef _WIN32
    static thread_local bool _win32_signal_guard_watchdog_impl_apcfunc_called = false;
    static inline void __stdcall _win32_signal_guard_watchdog_impl_apcfunc(uintptr_t /*unused*/) { _win32_signal_guard_watchdog_impl_apcfunc_called = true; }
    static inline unsigned long __stdcall _win32_signal_guard_watchdog_impl_thread(void *lpParameter)
    {
      detail::win32::SleepEx((unsigned long) (uintptr_t) lpParameter, true);
      if(!_win32_signal_guard_watchdog_impl_apcfunc_called)
      {
        watchdog_decider.check();
      }
      return 0;
    }
#else
    static auto posix_signal_guard_watchdog_decider_install =
    make_signal_guard_global_decider(signalc_set::timer_expire, [](raised_signal_info * /*unused*/) -> bool { return watchdog_decider.check(); });
#endif
    SIGNALGUARD_MEMFUNC_DECL void signal_guard_watchdog_impl::_detach() noexcept
    {
      watchdog_decider.lock.lock();
      if(_prev == nullptr)
      {
        assert(watchdog_decider.next == this);
        watchdog_decider.next = _next;
      }
      else
      {
        assert(_prev->_next == this);
        _prev->_next = _next;
        _prev = nullptr;
      }
      if(_next != nullptr)
      {
        assert(_next->_prev == this);
        _next->_prev = _prev;
        _next = nullptr;
      }
      watchdog_decider.lock.unlock();
    }
    SIGNALGUARD_MEMFUNC_DECL signal_guard_watchdog_impl::signal_guard_watchdog_impl(unsigned ms)
    {
      watchdog_decider.lock.lock();
      if(watchdog_decider.next != nullptr)
      {
        assert(watchdog_decider.next->_prev == nullptr);
        watchdog_decider.next->_prev = this;
        _next = watchdog_decider.next;
        _prev = nullptr;
        watchdog_decider.next = this;
      }
      else
      {
        _next = _prev = nullptr;
        watchdog_decider.next = this;
      }
      watchdog_decider.lock.unlock();
      auto uninstall = QUICKCPPLIB_NAMESPACE::scope::make_scope_fail([this]() noexcept { _detach(); });
#ifdef _WIN32
      unsigned long threadid;
      _deadline_ms = detail::win32::GetTickCount64() + ms;
      _threadh = detail::win32::CreateThread(nullptr, 0, _win32_signal_guard_watchdog_impl_thread, (void *) (uintptr_t) ms, 0, &threadid);
      if(_threadh == nullptr)
      {
        throw std::system_error(detail::win32::GetLastError(), std::system_category());
      }
      _inuse = true;
#else
      struct timespec ts;
      memset(&ts, 0, sizeof(ts));
      if(-1 == ::clock_gettime(CLOCK_MONOTONIC, &ts))
      {
        throw std::system_error(errno, std::system_category());
      }
#ifdef __APPLE__
      throw std::runtime_error("signal_guard_watchdog not implemented on Mac OS due to lack of POSIX timers");
#else
      timer_t timerid = nullptr;
      if(-1 == ::timer_create(CLOCK_MONOTONIC, nullptr, &timerid))
      {
        throw std::system_error(errno, std::system_category());
      }
      _deadline_ms = ((uint64_t) ts.tv_sec * 1000ULL + (uint64_t) ts.tv_nsec / 1000000ULL) + ms;
      _timerid = timerid;
      _inuse = true;
      struct itimerspec newtimer;
      memset(&newtimer, 0, sizeof(newtimer));
      newtimer.it_value.tv_sec = ms / 1000;
      newtimer.it_value.tv_nsec = (ms % 1000) * 1000000LL;
      if(-1 == ::timer_settime(timerid, 0, &newtimer, nullptr))
      {
        throw std::system_error(errno, std::system_category());
      }
#endif
#endif
    }
    SIGNALGUARD_MEMFUNC_DECL signal_guard_watchdog_impl::signal_guard_watchdog_impl(signal_guard_watchdog_impl &&o) noexcept
        :
#ifdef _WIN32
        _threadh(o._threadh)
#else
        _timerid(o._timerid)
#endif
        , _prev(o._prev)
        , _next(o._next)
        , _deadline_ms(o._deadline_ms)
        , _inuse(o._inuse)
        , _alreadycalled(o._alreadycalled)
    {
      if(_inuse)
      {
        watchdog_decider.lock.lock();
        if(_prev == nullptr)
        {
          assert(watchdog_decider.next == &o);
          watchdog_decider.next = this;
        }
        else
        {
          assert(_prev->_next == &o);
          _prev->_next = this;
        }
        if(_next != nullptr)
        {
          assert(_next->_prev == &o);
          _next->_prev = this;
        }
        o._prev = o._next = nullptr;
        o._inuse = false;
#ifdef _WIN32
        o._threadh = nullptr;
#else
        o._timerid = nullptr;
#endif
        watchdog_decider.lock.unlock();
      }
    }

    SIGNALGUARD_MEMFUNC_DECL void signal_guard_watchdog_impl::release()
    {
      if(_inuse)
      {
#ifdef _WIN32
        if(!detail::win32::QueueUserAPC(_win32_signal_guard_watchdog_impl_apcfunc, _threadh, 0))
        {
          throw std::system_error(detail::win32::GetLastError(), std::system_category());
        }
        if(!detail::win32::CloseHandle(_threadh))
        {
          throw std::system_error(detail::win32::GetLastError(), std::system_category());
        }
        _threadh = nullptr;
#else
#ifndef __APPLE__
        if(-1 == ::timer_delete(_timerid))
        {
          throw std::system_error(errno, std::system_category());
        }
#endif
        _timerid = nullptr;
#endif
        _inuse = false;
        _detach();
      }
    }
  }  // namespace detail

}  // namespace signal_guard

QUICKCPPLIB_NAMESPACE_END
