#include "platform/shared_library.h"
#if defined(SKR_OS_UNIX)
    #include <dlfcn.h>
#elif defined(SKR_OS_WINDOWS)
    #ifndef WIN32_MEAN_AND_LEAN
        #define WIN32_MEAN_AND_LEAN
    #endif
    #include <windows.h>
    #include <ghc/filesystem.hpp>
#endif


bool skr::SharedLibrary::load(const char* path)
{
    if (isLoaded() && !unload())
        return false;
    return loadImpl(path);
}

bool skr::SharedLibrary::isLoaded() const
{
    return _handle != nullptr;
}

bool skr::SharedLibrary::unload()
{
    return isLoaded() && unloadImpl();
}

bool skr::SharedLibrary::hasSymbol(const char* symbolName)
{
    eastl::string error = _lastError;
    getImpl(symbolName);
    bool has = _lastError.empty();
    _lastError = error;
    return has;
}

void* skr::SharedLibrary::getRawAddress(const char* symbolName)
{
    return getImpl(symbolName);
}

bool skr::SharedLibrary::hasError() const
{
    return !_lastError.empty();
}

eastl::string skr::SharedLibrary::errorString() const
{
    return _lastError;
}

skr::NativeLibHandle skr::SharedLibrary::handle() const
{
    return _handle;
}

#if defined(SKR_OS_UNIX)
    bool skr::SharedLibrary::loadImpl(const char* path)
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

    bool skr::SharedLibrary::unloadImpl()
    {
        _lastError.clear();
        // dlclose returns 0 on success
        if (_handle != nullptr && dlclose(_handle) != 0)
        {
            _lastError = dlerror();
            return false;
        }
        _handle = nullptr;
        return true;
    }

    void* skr::SharedLibrary::getImpl(const char* symbolName)
    {
        _lastError.clear();
        dlerror();
        if (!_handle) _handle = dlopen(NULL, RTLD_LAZY | RTLD_LOCAL);
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
    // Return a string explaining the last error
    eastl::string skr::SharedLibrary::getWindowsError()
    {
        auto tchar_to_utf8 = +[](const TCHAR* str, char* str8)
        {
        #ifdef _UNICODE
            auto size = WideCharToMultiByte(CP_UTF8, 0, str, (int)wcslen(str), NULL, 0, NULL, NULL);
            WideCharToMultiByte(CP_UTF8, 0, str, (int)wcslen(str), str8, size, NULL, NULL);
            str8[size] = '\0';
        #else
            return strcpy(str8, str);
        #endif
        };
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

    bool skr::SharedLibrary::loadImpl(const char* path)
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

    bool skr::SharedLibrary::unloadImpl()
    {
        _lastError.clear();
        if (!FreeLibrary((HMODULE)_handle))
        {
            _lastError = getWindowsError();
            return false;
        }
        _handle = nullptr;
        return true;
    }

    void* skr::SharedLibrary::getImpl(const char* symbolName)
    {
        _lastError.clear();
        void* addr = (void*)GetProcAddress((HMODULE)_handle, symbolName);
        if (!addr)
        {
            _lastError = getWindowsError();
            return nullptr;
        }
        return addr;
    }
#endif