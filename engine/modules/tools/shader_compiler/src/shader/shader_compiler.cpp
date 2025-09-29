#include "SkrCore/module/module.hpp"
#include "SkrContainers/vector.hpp"
#include "SkrContainers/hashmap.hpp"
#include "SkrContainers/stl_function.hpp"
#include "SkrBase/config.h"
#include "SkrShaderCompiler/shader_compiler.hpp"

namespace skr
{
struct SKR_SHADER_COMPILER_API SShaderCompilerModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char8_t** argv) override
    {
        for (auto&& [name, load_event] : on_load_events)
        {
            // SKR_LOG_DEBUG(u8"ShaderCompilerModule load event %s invoked", name.c_str());
            load_event();
        }
    }

    virtual void on_unload() override
    {
        for (auto&& [name, unload_event] : on_unload_events)
        {
            // SKR_LOG_DEBUG(u8"ShaderCompilerModule unload event %s invoked", name.c_str());
            unload_event();
        }
    }

    static skr::FlatHashMap<skr::EShaderSourceType, skr::stl_function<IShaderCompiler*()>> ctors;
    static skr::FlatHashMap<skr::EShaderSourceType, skr::stl_function<void(IShaderCompiler*)>> dtors;
    static skr::Vector<std::pair<skr::String, skr::stl_function<void()>>> on_load_events;
    static skr::Vector<std::pair<skr::String, skr::stl_function<void()>>> on_unload_events;
};

IMPLEMENT_DYNAMIC_MODULE(SShaderCompilerModule, SkrShaderCompiler);
skr::FlatHashMap<skr::EShaderSourceType, skr::stl_function<IShaderCompiler*()>> SShaderCompilerModule::ctors = {};
skr::FlatHashMap<skr::EShaderSourceType, skr::stl_function<void(IShaderCompiler*)>> SShaderCompilerModule::dtors = {};
skr::Vector<std::pair<skr::String, skr::stl_function<void()>>> SShaderCompilerModule::on_load_events = {};
skr::Vector<std::pair<skr::String, skr::stl_function<void()>>> SShaderCompilerModule::on_unload_events = {};

IShaderCompiler* SkrShaderCompiler_CreateByType(EShaderSourceType type) SKR_NOEXCEPT
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

void Util_ShaderCompilerRegister(EShaderSourceType type, IShaderCompiler* (*ctor)(), void (*dtor)(IShaderCompiler*)) SKR_NOEXCEPT
{
    SShaderCompilerModule::ctors.emplace(type, ctor);
    SShaderCompilerModule::dtors.emplace(type, dtor);
}

EShaderSourceType Util_GetShaderSourceTypeWithExtensionString(const char8_t* ext) SKR_NOEXCEPT
{
    if ((strcmp((const char*)ext, "hlsl") == 0) || (strcmp((const char*)ext, ".hlsl") == 0))
        return EShaderSourceType::HLSL;
    else if ((strcmp((const char*)ext, "sksl") == 0) || (strcmp((const char*)ext, ".sksl") == 0))
        return EShaderSourceType::SKSL;
    else
        return EShaderSourceType::INVALID;
}

} // namespace skr