#pragma once
#include "SkrGraphics/flags.h"
#include "SkrBase/config.h"
#include "SkrContainers/sptr.hpp"
#include "SkrContainers/span.hpp"
#include "SkrContainers/string.hpp"
#ifndef __meta__
    #include "SkrShaderCompiler/shader_compiler.generated.h" // IWYU pragma: export
#endif

SKR_DECLARE_TYPE_ID_FWD(skr::renderer, ShaderOptionInstance, skr_shader_option_instance);
SKR_DECLARE_TYPE_ID_FWD(skr::renderer, ShaderOptionTemplate, skr_shader_option_template);
SKR_DECLARE_TYPE_ID_FWD(skr::io, IRAMService, skr_ram_service);
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

sreflect_struct("guid": "12df1e9b-3aa9-4d63-88f6-c3fcb41b4e45")
ShaderSourceCode {
    inline ShaderSourceCode(skr::BlobId blob, const char8_t* name, EShaderSourceType type) SKR_NOEXCEPT
        : blob(blob),
          source_name(name),
          source_type(type) {}
    ~ShaderSourceCode() SKR_NOEXCEPT;

    skr::BlobId             blob = nullptr;
    skr::String             source_name;
    const EShaderSourceType source_type = EShaderSourceType::INVALID;
};

sreflect_struct("guid": "95f58c54-c21b-442a-b138-d14672e104d5")
SKR_SHADER_COMPILER_API ICompiledShader {
    virtual ~ICompiledShader() = default;

    virtual ECGPUShaderStage         GetShaderStage() const SKR_NOEXCEPT                                                    = 0;
    virtual skr::span<const uint8_t> GetBytecode() const SKR_NOEXCEPT                                                       = 0;
    virtual skr::span<const uint8_t> GetPDB() const SKR_NOEXCEPT                                                            = 0;
    virtual bool                     GetHashCode(uint32_t* flags, skr::span<uint32_t, 4> encoded_digits) const SKR_NOEXCEPT = 0;
};

sreflect_struct("guid": "72cfb3fb-507c-4d29-97f6-4499301daab6")
SKR_SHADER_COMPILER_API IShaderCompiler {
    virtual ~IShaderCompiler() = default;

    virtual EShaderSourceType GetSourceType() const SKR_NOEXCEPT                                                                                                                                      = 0;
    virtual bool              IsSupportedTargetFormat(ECGPUShaderBytecodeType format) const SKR_NOEXCEPT                                                                                              = 0;
    virtual void              SetShaderSwitches(skr::span<skr_shader_option_template_t> opt_defs, skr::span<skr_shader_option_instance_t> options, const skr_stable_shader_hash_t& hash) SKR_NOEXCEPT = 0;
    virtual void              SetShaderOptions(skr::span<skr_shader_option_template_t> opt_defs, skr::span<skr_shader_option_instance_t> options, const skr_stable_shader_hash_t& hash) SKR_NOEXCEPT  = 0;
    virtual ICompiledShader*  Compile(ECGPUShaderBytecodeType format, const ShaderSourceCode& source, const SShaderImporter& importer) SKR_NOEXCEPT                                                   = 0;
    virtual void              FreeCompileResult(ICompiledShader* compiled) SKR_NOEXCEPT                                                                                                               = 0;
};

IShaderCompiler*  SkrShaderCompiler_CreateByType(EShaderSourceType type) SKR_NOEXCEPT;
void              SkrShaderCompiler_Destroy(IShaderCompiler* compiler) SKR_NOEXCEPT;
void              Util_ShaderCompilerRegister(EShaderSourceType type, IShaderCompiler* (*ctor)(), void (*dtor)(IShaderCompiler*)) SKR_NOEXCEPT;
EShaderSourceType Util_GetShaderSourceTypeWithExtensionString(const char8_t* ext) SKR_NOEXCEPT;
} // namespace asset sreflect
} // namespace skd sreflect