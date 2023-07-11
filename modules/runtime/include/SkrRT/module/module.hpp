/*
 * @CopyRight: MIT License
 * Copyright (c) 2020 SaeruHikari
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THESOFTWARE.
 *
 *
 * @Description:
 * @Version: 0.1.0
 * @Autor: SaeruHikari
 * @Date: 2020-03-01 19:46:34
 * @LastEditTime: 2020-04-15 18:17:34
 */
#pragma once
#include "SkrRT/misc/types.h"
#include "SkrRT/platform/configure.h"
#ifdef __cplusplus
#include "SkrRT/platform/shared_library.hpp"
#include <containers/string.hpp>
#include <EASTL/vector.h>
#include <EASTL/unique_ptr.h>

namespace skr
{
class ModuleManager;
}

namespace skr
{
/**
 * @description: Represents a dependency as specified in the
 * plugin.json file. A list of dependencies is stored in each
 * PluginInfo Object.
 * @author: SaeruHikari
 */
struct ModuleDependency {
    skr::string name;    //!< The name of the dependency
    skr::string version; //!< The version of the dependency
};

/**
 * @description: Struct that contains all plugin metadata
 * @author: SaeruHikari
 */
struct ModuleInfo {
    skr::string name;         //!< name of the plugin
    skr::string prettyname;   //!< formatted name of the plugin
    skr::string core_version; //!< version of the engine
    skr::string version;      // !< version of the plugin
    skr::string linking;      // !< linking of the plugin
    skr::string license;      //!< license of the plugin
    skr::string url;          //!< url of the plugin
    skr::string copyright;    //!< copyright of the plugin
    // Dependencies array
    eastl::vector<ModuleDependency> dependencies;
};

struct RUNTIME_API ModuleSubsystemBase
{
    using CreatePFN = ModuleSubsystemBase*(*)();
    virtual ~ModuleSubsystemBase() SKR_NOEXCEPT = default;
    virtual void Initialize() = 0;
    virtual void Finalize() = 0;
    virtual void BeginReload() {}
    virtual void EndReload() {}
};

/**
 * @description: Base of all plugins
 * @author: SaeruHikari
 */
struct RUNTIME_API IModule {
    friend class ModuleManagerImpl;

public:
    IModule() = default;
    IModule(const IModule& rhs) = delete;
    IModule& operator=(const IModule& rhs) = delete;
    virtual ~IModule(){};
    virtual void on_load(int argc, char8_t** argv) = 0;
    virtual void on_unload() = 0;
    virtual int main_module_exec(int argc, char8_t** argv) { return 0; }
    virtual const char8_t* get_meta_data(void) = 0;
    virtual bool reloadable() { return false; }
    virtual const ModuleInfo* get_module_info()
    {
        return &information;
    }

protected:
    ModuleInfo information;
    eastl::vector<ModuleSubsystemBase*> subsystems;
};

struct RUNTIME_API IDynamicModule : public IModule {
    eastl::unique_ptr<SharedLibrary> sharedLib;
    virtual const char8_t* get_meta_data(void) override
    {
        skr::string symbolname = u8"__skr_module_meta__";
        symbolname.append(information.name);
        const char8_t* symbol_str = symbolname.u8_str();
        return sharedLib->get<const char8_t*>(symbol_str);
    }
};
struct IStaticModule : public IModule {
};
struct RUNTIME_API IHotfixModule : public IDynamicModule {
    void* state = nullptr;
    virtual void on_reload_begin() = 0;
    virtual void on_reload_finish() = 0;
    virtual bool reloadable() override final
    {
        return true;
    }
};
} // namespace skr

#define IMPLEMENT_STATIC_MODULE(ModuleImplClass, ModuleName) \
    inline static const skr::SStaticallyLinkedModuleRegistrant<ModuleImplClass> ModuleRegistrant##ModuleName(#ModuleName);

#define IMPLEMENT_DYNAMIC_MODULE(ModuleImplClass, ModuleName)                \
    extern "C" RUNTIME_EXPORT skr::IModule* __initializeModule##ModuleName() \
    {                                                                        \
        return new ModuleImplClass();                                        \
    }
#endif // #ifdef __cplusplus

#define SKR_MODULE_METADATA(stringdec, ModuleName) RUNTIME_EXTERN_C RUNTIME_EXPORT const char8_t* __skr_module_meta__##ModuleName = stringdec;