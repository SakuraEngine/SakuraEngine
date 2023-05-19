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
#include "platform/configure.h"
#include <containers/string.hpp>

namespace skr
{
using NativeLibHandle = void*;

/**
 * @description: Provides cross-platform low-level
 * access to shared library
 * @author: SaeruHikari
 */
class RUNTIME_API SharedLibrary
{
public:
    SharedLibrary() = default;
    /**
     * @description: Create & load a SharedLibrary on the disk.
     * @author: SaeruHikari
     */
    SharedLibrary(const char8_t* path)
    {
        load(path);
    }
    ~SharedLibrary() = default;

    SharedLibrary(const SharedLibrary& rhs) = delete;
    const SharedLibrary& operator=(const SharedLibrary& rhs) = delete;

    static const char8_t* GetPlatformFilePrefixName();
    static const char8_t* GetPlatformFileExtensionName();

    /**
     * @description: Load a shared lib. Unload old lib if called
     * on a obj with loaded lib.
     * @param const char8_t* path
     * @return: result of the operation
     * @author: SaeruHikari
     */
    bool load(const char8_t* path);
    /**
     * @description: checks if the library is loaded.
     * @author: SaeruHikari
     */
    bool isLoaded() const;
    /**
     * @description: unload the shared library.
     * @author: SaeruHikari
     */
    bool unload();
    /**
     * @description: Checks if the library has the symbol
     * specified by @a symbolName
     * @param symbolName
     * @author: SaeruHikari
     */
    bool hasSymbol(const char8_t* symbolName);
    /**
     * @description: Returns the symbol specified by @a symbolName
     * need to ensuring the symbol type.
     * @param symbolName
     * @author: SaeruHikari
     */
    template <typename SymT>
    SymT& get(const char8_t* symbolName)
    {
        return *(reinterpret_cast<SymT*>(getImpl(symbolName)));
    }
    /**
     * @description:  Get the address of a symbol.
     *  Returns nullptr if the library doesn't have the symbol.
     * @param symbolName
     * @return The address
     * @author: SaeruHikari
     */
    void* getRawAddress(const char8_t* symbolName);
    /**
     * @description: Checks if the last call raise an error.
     * @author: SaeruHikari
     */
    bool hasError() const;
    /**
     * @description: Get the last error string.
     * @author: SaeruHikari
     */
    skr::string errorString() const;
    /**
     * @description: Get the native handle of lib on the system.
     * @author: SaeruHikari
     */
    NativeLibHandle handle() const;
private:
    skr::string _lastError;
    NativeLibHandle _handle = nullptr;
    // Linux implementation
#if defined(SKR_OS_UNIX)
    bool loadImpl(const char8_t* path);
    bool unloadImpl();
    void* getImpl(const char8_t* symbolName);
#elif defined(SKR_OS_WINDOWS) // Windows implementation
    // Return a string explaining the last error
    skr::string getWindowsError();
    bool loadImpl(const char8_t* path);
    bool unloadImpl();
    void* getImpl(const char8_t* symbolName);
#endif
};

} // namespace skr

#define SKR_SHARED_LIB_API_PFN(api) decltype(&api)
#define SKR_SHARED_LIB_LOAD_API(lib, api) (lib).hasSymbol(SKR_UTF8(#api)) ? &(lib).get<decltype(api)>(SKR_UTF8(#api)) : nullptr