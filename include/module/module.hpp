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
#include "platform/configure.h"
#include "platform/shared_library.h"
#include <EASTL/string.h>
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
    eastl::string name;    //!< The name of the dependency
    eastl::string version; //!< The version of the dependency
};

/**
 * @description: Struct that contains all plugin metadata
 * @author: SaeruHikari
 */
struct ModuleInfo {
    eastl::string name;         //!< name of the plugin
    eastl::string prettyname;   //!< formatted name of the plugin
    eastl::string core_version; //!< version of the engine
    eastl::string version;      // !< version of the plugin
    eastl::string linking;      // !< linking of the plugin
    eastl::string license;      //!< license of the plugin
    eastl::string url;          //!< url of the plugin
    eastl::string copyright;    //!< copyright of the plugin
    // Dependencies array
    eastl::vector<ModuleDependency> dependencies;
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
    virtual void on_load() = 0;
    virtual void on_unload() = 0;
    virtual void main_module_exec() {}
    virtual const char* get_meta_data(void) = 0;
    virtual const ModuleInfo* get_module_info()
    {
        return &information;
    }

protected:
    ModuleInfo information;
};
struct RUNTIME_API IDynamicModule : public IModule {
    eastl::unique_ptr<SharedLibrary> sharedLib;
    virtual const char* get_meta_data(void) override
    {
        eastl::string symbolname = "__skr_module_meta__";
        symbolname.append(information.name);
        return sharedLib->get<const char*>(symbolname.c_str());
    }
};
struct IStaticModule : public IModule {
};
} // namespace skr

#define SKR_MODULE_METADATA(stringdec, ModuleName) RUNTIME_EXTERN_C RUNTIME_EXPORT const char* __skr_module_meta__##ModuleName = stringdec;
