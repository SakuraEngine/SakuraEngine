/*
 * @This File is Part of Sakura by SaeruHikari:
 * @Description: Copyright SaeruHikari
 * @Version: 0.1.0
 * @Autor: SaeruHikari
 * @Date: 2020-02-13 22:58:31
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2020-06-08 02:46:16
 */
#pragma once
#include "configure.h"

#if defined(SKR_OS_UNIX)
    #include <dlfcn.h>
using NativeLibHandle = void*;
#elif defined(SKR_OS_WINDOWS)
    #include <windows.h>
    #include <ghc/filesystem.hpp>
using NativeLibHandle = HMODULE;
#endif
#include <EASTL/string.h>

namespace skr
{
/**
 * @description: Provides cross-platform low-level
 * access to shared library
 * @author: SaeruHikari
 */
class SharedLibrary
{
public:
    SharedLibrary() = default;
    /**
     * @description: Create & load a SharedLibrary on the disk.
     * @author: SaeruHikari
     */
    SharedLibrary(const char* path)
    {
        load(path);
    }
    ~SharedLibrary() = default;

    SharedLibrary(const SharedLibrary& rhs) = delete;
    const SharedLibrary& operator=(const SharedLibrary& rhs) = delete;

    /**
     * @description: Load a shared lib. Unload old lib if called
     * on a obj with loaded lib.
     * @param const char* path
     * @return: result of the operation
     * @author: SaeruHikari
     */
    bool load(const char* path)
    {
        if (isLoaded() && !unload())
            return false;
        return loadImpl(path);
    }

    /**
     * @description: checks if the library is loaded.
     * @author: SaeruHikari
     */
    bool isLoaded() const
    {
        return _handle != nullptr;
    }
    /**
     * @description: unload the shared library.
     * @author: SaeruHikari
     */
    bool unload()
    {
        return isLoaded() && unloadImpl();
    }

    /**
     * @description: Checks if the library has the symbol
     * specified by @a symbolName
     * @param symbolName
     * @author: SaeruHikari
     */
    bool hasSymbol(const char* symbolName)
    {
        eastl::string error = _lastError;
        getImpl(symbolName);
        bool has = _lastError.empty();
        _lastError = error;
        return has;
    }

    /**
     * @description: Returns the symbol specified by @a symbolName
     * need to ensuring the symbol type.
     * @param symbolName
     * @author: SaeruHikari
     */
    template <typename SymT>
    SymT& get(const char* symbolName)
    {
        return *(reinterpret_cast<SymT*>(reinterpret_cast<uintptr_t>(getImpl(symbolName))));
    }

    /**
     * @description:  Get the address of a symbol.
     *  Returns nullptr if the library doesn't have the symbol.
     * @param symbolName
     * @return The address
     * @author: SaeruHikari
     */
    void* getRawAddress(const char* symbolName)
    {
        return getImpl(symbolName);
    }

    /**
     * @description: Checks if the last call raise an error.
     * @author: SaeruHikari
     */
    bool hasError() const
    {
        return !_lastError.empty();
    }

    /**
     * @description: Get the last error string.
     * @author: SaeruHikari
     */
    eastl::string errorString() const
    {
        return _lastError;
    }

    /**
     * @description: Get the native handle of lib on the system.
     * @author: SaeruHikari
     */
    NativeLibHandle handle() const
    {
        return _handle;
    }

private:
    eastl::string _lastError;
    NativeLibHandle _handle = nullptr;
    // Linux implementation
#if defined(SKR_OS_UNIX)
    bool loadImpl(const char* path)
    {
        _lastError.clear();
        _handle = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
        if (!_handle)
        {
            _lastError = dlerror();
            return false;
        }
        return true;
    }

    bool unloadImpl()
    {
        _lastError.clear();
        // dlclose returns 0 on success
        if (dlclose(_handle) != 0)
        {
            _lastError = dlerror();
            return false;
        }
        _handle = nullptr;
        return true;
    }

    void* getImpl(const char* symbolName)
    {
        _lastError.clear();
        dlerror();
        void* symbol = dlsym(_handle, symbolName);
        const char* error = dlerror();
        if (error)
        {
            // An error occured
            _lastError = error;
            return nullptr;
        }
        return symbol;
    }
#elif defined(SKR_OS_WINDOWS) // Windows implementation
    void tchar_to_utf8(const TCHAR* str, char* str8)
    {
    #ifdef _UNICODE
        auto size = WideCharToMultiByte(CP_UTF8, 0, str, (int)wcslen(str), NULL, 0, NULL, NULL);
        WideCharToMultiByte(CP_UTF8, 0, str, (int)wcslen(str), str8, size, NULL, NULL);
        str8[size] = '\0';
    #else
        return strcpy(str8, str);
    #endif
    }

    // Return a string explaining the last error
    eastl::string getWindowsError()
    {
        DWORD lastError = GetLastError();
        TCHAR buffer[256];
        char8_t u8str[256];
        if (lastError != 0)
        {
            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
            nullptr,
            lastError,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            buffer,
            256 - 1,
            nullptr);
            tchar_to_utf8(buffer, u8str);
            return eastl::string(u8str);
        }
        return eastl::string();
    }

    bool loadImpl(const char* path)
    {
        _lastError.clear();
        if (path == nullptr)
        {
            _handle = GetModuleHandle(nullptr);
        }
        else
        {
            auto wpath = ghc::filesystem::path(path);
            _handle = LoadLibrary(wpath.c_str());
        }
        if (!_handle)
        {
            _lastError = getWindowsError();
            return false;
        }
        return true;
    }

    bool unloadImpl()
    {
        _lastError.clear();
        if (!FreeLibrary(_handle))
        {
            _lastError = getWindowsError();
            return false;
        }
        _handle = nullptr;
        return true;
    }

    void* getImpl(const char* symbolName)
    {
        _lastError.clear();
        void* addr = (void*)GetProcAddress(_handle, symbolName);
        if (!addr)
        {
            _lastError = getWindowsError();
            return nullptr;
        }
        return addr;
    }
#endif
};

} // namespace skr