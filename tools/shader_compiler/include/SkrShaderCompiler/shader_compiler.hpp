#pragma once
#include "SkrShaderCompiler/module.configure.h"
#include "platform/configure.h"
#include "cgpu/flags.h"
#include "containers/span.hpp"
#include "containers/string.hpp"
#include <EASTL/functional.h>
#ifndef __meta__
#include "SkrShaderCompiler/shader_compiler.generated.h"
#endif

struct skr_shader_option_instance_t;
struct skr_stable_shader_hash_t;

namespace skd sreflect
{
namespace asset sreflect
{
struct SShaderImporter;

sreflect_enum_class("guid" : "71c26ffc-9f4d-4ca5-9db6-b6479b7ff001")
EShaderSourceType : uint32_t
{
    INVALID,
    HLSL,
    SKSL,
    COUNT
};

struct ShaderSourceCode
{
    inline ShaderSourceCode(uint8_t* bytes, uint64_t size, const char* name, EShaderSourceType type) SKR_NOEXCEPT
        : bytes(bytes), size(size), source_name(name), source_type(type) {}
    ~ShaderSourceCode() SKR_NOEXCEPT;

    uint8_t* bytes = nullptr;
    uint64_t size = 0;
    skr::string source_name;
    const EShaderSourceType source_type = EShaderSourceType::INVALID;
};

struct SKR_SHADER_COMPILER_API ICompiledShader
{
    virtual ~ICompiledShader() = default;

    virtual ECGPUShaderStage GetShaderStage() const SKR_NOEXCEPT = 0;
    virtual skr::span<const uint8_t> GetBytecode() const SKR_NOEXCEPT = 0;
    virtual skr::span<const uint8_t> GetPDB() const SKR_NOEXCEPT = 0;
    virtual bool GetHashCode(uint32_t* flags, skr::span<uint32_t, 4> encoded_digits) const SKR_NOEXCEPT = 0;
};

struct SKR_SHADER_COMPILER_API IShaderCompiler
{
    virtual ~IShaderCompiler() = default;

    virtual EShaderSourceType GetSourceType() const SKR_NOEXCEPT = 0;
    virtual bool IsSupportedTargetFormat(ECGPUShaderBytecodeType format) const SKR_NOEXCEPT = 0;
    virtual void SetShaderOptions(skr::span<skr_shader_option_instance_t> options, const skr_stable_shader_hash_t& md5) SKR_NOEXCEPT= 0;
    virtual ICompiledShader* Compile(ECGPUShaderBytecodeType format, const ShaderSourceCode& source, const SShaderImporter& importer) SKR_NOEXCEPT = 0;
    virtual void FreeCompileResult(ICompiledShader* compiled) SKR_NOEXCEPT = 0;
};

IShaderCompiler* SkrShaderCompiler_CreateByType(EShaderSourceType type) SKR_NOEXCEPT;
void SkrShaderCompiler_Destroy(IShaderCompiler* compiler) SKR_NOEXCEPT;
void Util_ShaderCompilerRegister(EShaderSourceType type, IShaderCompiler*(*ctor)(), void(*dtor)(IShaderCompiler*)) SKR_NOEXCEPT;
EShaderSourceType Util_GetShaderSourceTypeWithExtensionString(const char* ext) SKR_NOEXCEPT;
} // namespace asset
} // namespace skd