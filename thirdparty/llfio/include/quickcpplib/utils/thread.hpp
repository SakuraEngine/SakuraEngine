/* Thread related functions
(C) 2016-2017 Niall Douglas <http://www.nedproductions.biz/> (3 commits)
File Created: Jun 2016


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

#ifndef QUICKCPPLIB_UTILS_THREAD_HPP
#define QUICKCPPLIB_UTILS_THREAD_HPP

#include "../config.hpp"

#ifdef __linux__
#include <sys/syscall.h>  // for SYS_gettid
#include <unistd.h>       // for syscall()
#endif

#if defined(__APPLE__)
#include <mach/mach_init.h>  // for mach_thread_self
#include <mach/mach_port.h>  // for mach_port_deallocate
#endif

#ifdef __FreeBSD__
#include <pthread_np.h>  // for pthread_getthreadid_np
#endif

QUICKCPPLIB_NAMESPACE_BEGIN

namespace utils
{
  namespace thread
  {
#ifdef _WIN32
    namespace win32
    {
      extern "C" __declspec(dllimport) unsigned long __stdcall GetCurrentThreadId(void);
    }
#endif
    //! The thread id of the calling thread
    inline unsigned this_thread_id() noexcept
    {
#ifdef _WIN32
      return (unsigned) win32::GetCurrentThreadId();
#elif defined(__linux__)
      return (unsigned) syscall(SYS_gettid);
#elif defined(__APPLE__)
      thread_port_t tid = mach_thread_self();
      mach_port_deallocate(mach_task_self(), tid);
      return (unsigned) tid;
#else
      return (unsigned) pthread_getthreadid_np();
#endif
    }
  }
}

QUICKCPPLIB_NAMESPACE_END


#endif
