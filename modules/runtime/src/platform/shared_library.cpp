#include "platform/shared_library.hpp"
#if defined(SKR_OS_UNIX)
    #include <dlfcn.h>
#elif defined(SKR_OS_WINDOWS)
    #ifndef WIN32_MEAN_AND_LEAN
        #define WIN32_MEAN_AND_LEAN
    #endif
    #include <windows.h>
    #include <platform/filesystem.hpp>
#endif

#if defined(SKR_OS_MACOSX)
    static const char8_t* ___dl_prefix_name___ = u8"lib";
#elif defined(SKR_OS_PROSPERO)
    static const char8_t* ___dl_prefix_name___ = u8"";
#elif defined(SKR_OS_UNIX)
    const char8_t* ___dl_prefix_name___ = u8"lib";
#elif defined(SKR_OS_WINDOWS)
    static const char8_t* ___dl_prefix_name___ = u8"";
#endif    

const char8_t* skr::SharedLibrary::GetPlatformFilePrefixName()
{
    return ___dl_prefix_name___;
}

#if defined(SKR_OS_MACOSX)
    static const char8_t* ___dl_ext_name___ = u8".dylib";
#elif defined(SKR_OS_PROSPERO)
    static const char8_t* ___dl_ext_name___ = u8".elf";
#elif defined(SKR_OS_UNIX)
    static const char8_t* ___dl_ext_name___ = u8".so";
#elif defined(SKR_OS_WINDOWS)
    static const char8_t* ___dl_ext_name___ = u8".dll";
#endif    

const char8_t* skr::SharedLibrary::GetPlatformFileExtensionName()
{
    return ___dl_ext_name___;
}

bool skr::SharedLibrary::load(const char8_t* path)
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

bool skr::SharedLibrary::hasSymbol(const char8_t* symbolName)
{
    skr::string error = _lastError;
    getImpl(symbolName);
    bool has = _lastError.is_empty();
    _lastError = error;
    return has;
}

void* skr::SharedLibrary::getRawAddress(const char8_t* symbolName)
{
    return getImpl(symbolName);
}

bool skr::SharedLibrary::hasError() const
{
    return !_lastError.is_empty();
}

skr::string skr::SharedLibrary::errorString() const
{
    return _lastError;
}

skr::NativeLibHandle skr::SharedLibrary::handle() const
{
    return _handle;
}

#if defined(SKR_OS_UNIX)
    bool skr::SharedLibrary::loadImpl(const char8_t* path)
    {
        _lastError.empty();
        _handle = dlopen((const char*)path, RTLD_LAZY | RTLD_LOCAL);
        if (!_handle)
        {
            _lastError =(const char8_t*) dlerror();
            return false;
        }
        return true;
    }

    bool skr::SharedLibrary::unloadImpl()
    {
        _lastError.empty();
        // dlclose returns 0 on success
        if (_handle != nullptr && dlclose(_handle) != 0)
        {
            _lastError = (const char8_t*)dlerror();
            return false;
        }
        _handle = nullptr;
        return true;
    }

    void* skr::SharedLibrary::getImpl(const char8_t* symbolName)
    {
        _lastError.empty();
        dlerror();
        if (!_handle) _handle = dlopen(NULL, RTLD_LAZY | RTLD_LOCAL);
        void* symbol = dlsym(_handle, (const char*)symbolName);
        const char8_t* error = (const char8_t*)dlerror();
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
    skr::string skr::SharedLibrary::getWindowsError()
    {
        auto tchar_to_utf8 = +[](const TCHAR* str, char8_t* str8)
        {
        #ifdef _UNICODE
            auto size = WideCharToMultiByte(CP_UTF8, 0, str, (int)wcslen(str), NULL, 0, NULL, NULL);
            WideCharToMultiByte(CP_UTF8, 0, str, (int)wcslen(str), (char*)str8, size, NULL, NULL);
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
            return skr::string(u8str);
        }
        return skr::string();
    }

    bool skr::SharedLibrary::loadImpl(const char8_t* path)
    {
        _lastError.empty();
        if (path == nullptr)
        {
            _handle = GetModuleHandle(nullptr);
        }
        else
        {
            auto wpath = skr::filesystem::path(path);
            _handle = GetModuleHandle(wpath.c_str());
            if (_handle == NULL)
            {
                _handle = LoadLibrary(wpath.c_str());
            }
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
        _lastError.empty();
        if (!FreeLibrary((HMODULE)_handle))
        {
            _lastError = getWindowsError();
            return false;
        }
        _handle = nullptr;
        return true;
    }

    void* skr::SharedLibrary::getImpl(const char8_t* symbolName)
    {
        _lastError.empty();
        void* addr = (void*)GetProcAddress((HMODULE)_handle, (const char*)symbolName);
        if (!addr)
        {
            _lastError = getWindowsError();
            return nullptr;
        }
        return addr;
    }
#endif