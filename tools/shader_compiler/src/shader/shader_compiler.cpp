#include "utils/log.h"
#include "containers/hashmap.hpp"
#include "module/module.hpp"

#include "SkrShaderCompiler/module.configure.h"
#include "SkrShaderCompiler/shader_compiler.hpp"

namespace skd
{
namespace asset
{
struct SKR_SHADER_COMPILER_API SShaderCompilerModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char** argv) override
    {
        for (auto&& [name, load_event] : on_load_events)
        {
            // SKR_LOG_DEBUG("ShaderCompilerModule load event %s invoked", name.c_str());
            load_event();
        }
    }

    virtual void on_unload() override
    {
        for (auto&& [name, unload_event] : on_unload_events)
        {
            // SKR_LOG_DEBUG("ShaderCompilerModule unload event %s invoked", name.c_str());
            unload_event();
        }
    }
    
    static skr::flat_hash_map<skd::asset::EShaderSourceType, eastl::function<IShaderCompiler*()>> ctors;
    static skr::flat_hash_map<skd::asset::EShaderSourceType, eastl::function<void(IShaderCompiler*)>> dtors;
    static eastl::vector<eastl::pair<skr::string, eastl::function<void()>>> on_load_events;
    static eastl::vector<eastl::pair<skr::string, eastl::function<void()>>> on_unload_events;
};

IMPLEMENT_DYNAMIC_MODULE(SShaderCompilerModule, SkrShaderCompiler);
skr::flat_hash_map<skd::asset::EShaderSourceType, eastl::function<IShaderCompiler*()>> SShaderCompilerModule::ctors = {};
skr::flat_hash_map<skd::asset::EShaderSourceType, eastl::function<void(IShaderCompiler*)>> SShaderCompilerModule::dtors = {};
eastl::vector<eastl::pair<skr::string, eastl::function<void()>>> SShaderCompilerModule::on_load_events = {};
eastl::vector<eastl::pair<skr::string, eastl::function<void()>>> SShaderCompilerModule::on_unload_events = {};

IShaderCompiler* SkrShaderCompiler_CreateByType(asset::EShaderSourceType type) SKR_NOEXCEPT
{
    auto iter = SShaderCompilerModule::ctors.find(type);
    if (iter != SShaderCompilerModule::ctors.end())
    {
        return iter->second();
    }
    return nullptr;
}

void SkrShaderCompiler_Destroy(IShaderCompiler* compiler) SKR_NOEXCEPT
{
    const auto type = compiler->GetSourceType();
    auto iter = SShaderCompilerModule::dtors.find(type);
    if (iter != SShaderCompilerModule::dtors.end())
    {
        return iter->second(compiler);
    }
}

void Util_ShaderCompilerRegister(asset::EShaderSourceType type, IShaderCompiler*(*ctor)(), void(*dtor)(IShaderCompiler*)) SKR_NOEXCEPT
{
    SShaderCompilerModule::ctors.emplace(type, ctor);
    SShaderCompilerModule::dtors.emplace(type, dtor);
}

asset::EShaderSourceType Util_GetShaderSourceTypeWithExtensionString(const char* ext) SKR_NOEXCEPT
{
    if ((strcmp(ext, "hlsl") == 0) || (strcmp(ext, ".hlsl") == 0))
        return asset::EShaderSourceType::HLSL;
    else if ((strcmp(ext, "sksl") == 0) || (strcmp(ext, ".sksl") == 0))
        return asset::EShaderSourceType::SKSL;
    else
        return asset::EShaderSourceType::INVALID;
}
    
} // namespace asset
} // namespace skd