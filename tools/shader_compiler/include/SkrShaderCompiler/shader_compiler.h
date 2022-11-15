#pragma once
#include "SkrShaderCompiler/module.configure.h"
#include "platform/configure.h"
#include <EASTL/functional.h>
#ifndef __meta__
#include "SkrShaderCompiler/shader_compiler.generated.h"
#endif

namespace skd sreflect
{
namespace asset sreflect
{
sreflect_enum("guid" : "71c26ffc-9f4d-4ca5-9db6-b6479b7ff001")
EShaderSourceType : uint32_t
{
    SHADER_SOURCE_TYPE_INVALID,
    SHADER_SOURCE_TYPE_HLSL,
    SHADER_SOURCE_TYPE_SKSL,
    SHADER_SOURCE_TYPE_COUNT
};
} // namespace asset

struct SKR_SHADER_COMPILER_API IShaderCompiler
{
    virtual ~IShaderCompiler() = default;


};

IShaderCompiler* SkrShaderCompiler_CreateByType(asset::EShaderSourceType type) SKR_NOEXCEPT;
void Util_ShaderCompilerRegister(asset::EShaderSourceType type, IShaderCompiler*(*ctor)()) SKR_NOEXCEPT;
void Util_ShaderCompilerEventOnLoad(const char*, const eastl::function<void()>& event) SKR_NOEXCEPT;
void Util_ShaderCompilerEventOnLoad(const char* name, void (*function)()) SKR_NOEXCEPT;
void Util_ShaderCompilerEventOnUnload(const char*, const eastl::function<void()>& event) SKR_NOEXCEPT;
void Util_ShaderCompilerEventOnUnload(const char* name, void (*function)()) SKR_NOEXCEPT;
asset::EShaderSourceType Util_GetShaderSourceTypeWithExtensionString(const char* ext) SKR_NOEXCEPT;

} // namespace skd