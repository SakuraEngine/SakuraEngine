/* Implements backtrace() et al from glibc on win64
(C) 2016-2017 Niall Douglas <http://www.nedproductions.biz/> (14 commits)
File Created: Mar 2016


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

#include "../include/quickcpplib/execinfo_win64.h"

#include <atomic>
#include <stdlib.h>  // for abort
#include <string.h>

// To avoid including windows.h, this source has been macro expanded and win32 function shimmed for C++ only
#if defined(__cplusplus) && !defined(__clang__)
namespace win32
{
  extern _Ret_maybenull_ void *__stdcall LoadLibraryA(_In_ const char *lpLibFileName);
  typedef int(__stdcall *GetProcAddress_returntype)();
  extern GetProcAddress_returntype __stdcall GetProcAddress(_In_ void *hModule, _In_ const char *lpProcName);
  extern _Success_(return != 0) unsigned short __stdcall RtlCaptureStackBackTrace(_In_ unsigned long FramesToSkip, _In_ unsigned long FramesToCapture,
                                                                                  _Out_writes_to_(FramesToCapture, return ) void **BackTrace,
                                                                                  _Out_opt_ unsigned long *BackTraceHash);
  extern _Success_(return != 0)
  _When_((cchWideChar == -1) && (cbMultiByte != 0),
         _Post_equal_to_(_String_length_(lpMultiByteStr) +
                         1)) int __stdcall WideCharToMultiByte(_In_ unsigned int CodePage, _In_ unsigned long dwFlags, const wchar_t *lpWideCharStr,
                                                               _In_ int cchWideChar, _Out_writes_bytes_to_opt_(cbMultiByte, return ) char *lpMultiByteStr,
                                                               _In_ int cbMultiByte, _In_opt_ const char *lpDefaultChar, _Out_opt_ int *lpUsedDefaultChar);
#pragma comment(lib, "kernel32.lib")
#if(defined(__x86_64__) || defined(_M_X64)) || (defined(__aarch64__) || defined(_M_ARM64))
#pragma comment(linker, "/alternatename:?LoadLibraryA@win32@@YAPEAXPEBD@Z=LoadLibraryA")
#pragma comment(linker, "/alternatename:?GetProcAddress@win32@@YAP6AHXZPEAXPEBD@Z=GetProcAddress")
#pragma comment(linker, "/alternatename:?RtlCaptureStackBackTrace@win32@@YAGKKPEAPEAXPEAK@Z=RtlCaptureStackBackTrace")
#pragma comment(linker, "/alternatename:?WideCharToMultiByte@win32@@YAHIKPEB_WHPEADHPEBDPEAH@Z=WideCharToMultiByte")
#elif defined(__x86__) || defined(_M_IX86) || defined(__i386__)
#pragma comment(linker, "/alternatename:?LoadLibraryA@win32@@YGPAXPBD@Z=__imp__LoadLibraryA@4")
#pragma comment(linker, "/alternatename:?GetProcAddress@win32@@YGP6GHXZPAXPBD@Z=__imp__GetProcAddress@8")
#pragma comment(linker, "/alternatename:?RtlCaptureStackBackTrace@win32@@YGGKKPAPAXPAK@Z=__imp__RtlCaptureStackBackTrace@16")
#pragma comment(linker, "/alternatename:?WideCharToMultiByte@win32@@YGHIKPB_WHPADHPBDPAH@Z=__imp__WideCharToMultiByte@32")
#elif defined(__arm__) || defined(_M_ARM)
#pragma comment(linker, "/alternatename:?LoadLibraryA@win32@@YAPAXPBD@Z=LoadLibraryA")
#pragma comment(linker, "/alternatename:?GetProcAddress@win32@@YAP6AHXZPAXPBD@Z=GetProcAddress")
#pragma comment(linker, "/alternatename:?RtlCaptureStackBackTrace@win32@@YAGKKPAPAXPAK@Z=RtlCaptureStackBackTrace")
#pragma comment(linker, "/alternatename:?WideCharToMultiByte@win32@@YAHIKPB_WHPADHPBDPAH@Z=WideCharToMultiByte")
#else
#error Unknown architecture
#endif
}  // namespace win32
#else
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif

#ifdef __cplusplus
namespace
{
#endif

  typedef struct _IMAGEHLP_LINE64
  {
    unsigned long SizeOfStruct;
    void *Key;
    unsigned long LineNumber;
    wchar_t *FileName;
    unsigned long long int Address;
  } IMAGEHLP_LINE64, *PIMAGEHLP_LINE64;

  typedef int(__stdcall *SymInitialize_t)(_In_ void *hProcess, _In_opt_ const wchar_t *UserSearchPath, _In_ int fInvadeProcess);

  typedef int(__stdcall *SymGetLineFromAddr64_t)(_In_ void *hProcess, _In_ unsigned long long int dwAddr, _Out_ unsigned long *pdwDisplacement,
                                                 _Out_ PIMAGEHLP_LINE64 Line);

  static std::atomic<unsigned> dbghelp_init_lock;
#if defined(__cplusplus) && !defined(__clang__)
  static void *dbghelp;
#else
static HMODULE dbghelp;
#endif
  static SymInitialize_t SymInitialize;
  static SymGetLineFromAddr64_t SymGetLineFromAddr64;

  static void load_dbghelp()
  {
#if defined(__cplusplus) && !defined(__clang__)
    using win32::GetProcAddress;
    using win32::LoadLibraryA;
#endif
    while(dbghelp_init_lock.exchange(1, std::memory_order_acq_rel))
      ;
    if(dbghelp)
    {
      dbghelp_init_lock.store(0, std::memory_order_release);
      return;
    }
    dbghelp = LoadLibraryA("DBGHELP.DLL");
    if(dbghelp)
    {
      SymInitialize = (SymInitialize_t) GetProcAddress(dbghelp, "SymInitializeW");
      if(!SymInitialize)
        abort();
      if(!SymInitialize((void *) (size_t) -1 /*GetCurrentProcess()*/, NULL, 1))
        abort();
      SymGetLineFromAddr64 = (SymGetLineFromAddr64_t) GetProcAddress(dbghelp, "SymGetLineFromAddrW64");
      if(!SymGetLineFromAddr64)
        abort();
    }
    dbghelp_init_lock.store(0, std::memory_order_release);
  }

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C"
{
#endif

  _Check_return_ size_t backtrace(_Out_writes_(len) void **bt, _In_ size_t len)
  {
#if defined(__cplusplus) && !defined(__clang__)
    using win32::RtlCaptureStackBackTrace;
#endif
    return RtlCaptureStackBackTrace(1, (unsigned long) len, bt, NULL);
  }

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 6385 6386)  // MSVC static analyser can't grok this function. clang's analyser gives it thumbs up.
#endif
  _Check_return_ _Ret_writes_maybenull_(len) char **backtrace_symbols(_In_reads_(len) void *const *bt, _In_ size_t len)
  {
#if defined(__cplusplus) && !defined(__clang__)
    using win32::WideCharToMultiByte;
#endif
    size_t bytes = (len + 1) * sizeof(void *) + 256, n;
    if(!len)
      return NULL;
    else
    {
      char **ret = (char **) malloc(bytes);
      char *p = (char *) (ret + len + 1), *end = (char *) ret + bytes;
      if(!ret)
        return NULL;
      for(n = 0; n < len + 1; n++)
        ret[n] = NULL;
      load_dbghelp();
      for(n = 0; n < len; n++)
      {
        unsigned long displ;
        IMAGEHLP_LINE64 ihl;
        memset(&ihl, 0, sizeof(ihl));
        ihl.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
        int please_realloc = 0;
        if(!bt[n])
        {
          ret[n] = NULL;
        }
        else
        {
          // Keep offset till later
          ret[n] = (char *) ((char *) p - (char *) ret);
          {
            static std::atomic<unsigned> symlock(0);
            while(symlock.exchange(1, std::memory_order_acq_rel))
              ;
            if(!SymGetLineFromAddr64 || !SymGetLineFromAddr64((void *) (size_t) -1 /*GetCurrentProcess()*/, (size_t) bt[n], &displ, &ihl))
            {
              symlock.store(0, std::memory_order_release);
              if(n == 0)
              {
                free(ret);
                return NULL;
              }
              ihl.FileName = (wchar_t *) L"unknown";
              ihl.LineNumber = 0;
            }
            else
            {
              symlock.store(0, std::memory_order_release);
            }
          }
        retry:
          if(please_realloc)
          {
            char **temp = (char **) realloc(ret, bytes + 256);
            if(!temp)
            {
              free(ret);
              return NULL;
            }
            p = (char *) temp + (p - (char *) ret);
            ret = temp;
            bytes += 256;
            end = (char *) ret + bytes;
          }
          if(ihl.FileName && ihl.FileName[0])
          {
            int plen = WideCharToMultiByte(65001 /*CP_UTF8*/, 0, ihl.FileName, -1, p, (int) (end - p), NULL, NULL);
            if(!plen)
            {
              please_realloc = 1;
              goto retry;
            }
            p[plen - 1] = 0;
            p += plen - 1;
          }
          else
          {
            if(end - p < 16)
            {
              please_realloc = 1;
              goto retry;
            }
            _ui64toa_s((size_t) bt[n], p, end - p, 16);
            p = strchr(p, 0);
          }
          if(end - p < 16)
          {
            please_realloc = 1;
            goto retry;
          }
          *p++ = ':';
          _itoa_s(ihl.LineNumber, p, end - p, 10);
          p = strchr(p, 0) + 1;
        }
      }
      for(n = 0; n < len; n++)
      {
        if(ret[n])
          ret[n] = (char *) ret + (size_t) ret[n];
      }
      return ret;
    }
  }
#ifdef _MSC_VER
#pragma warning(pop)
#endif

  // extern void backtrace_symbols_fd(void *const *bt, size_t len, int fd);

#ifdef __cplusplus
}
#endif
