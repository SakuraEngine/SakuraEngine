#pragma once
#include "shader_compiler.hpp"
#include "SkrModule/subsystem.hpp"
#include "SkrOS/shared_library.hpp"
#include "SkrContainers/stl_string.hpp"
#include "SkrRenderer/resources/shader_resource.hpp"
#ifndef __meta__
    #include "SkrShaderCompiler/dxc_compiler.generated.h" // IWYU pragma: export
#endif

struct IDxcUtils;
struct IDxcCompiler3;
struct IDxcIncludeHandler;
struct IDxcResult;
struct IDxcBlob;
struct IDxcBlobUtf8;
struct IDxcBlobWide;
struct IDxcBlobEncoding;

namespace skd sreflect
{
namespace asset sreflect
{
struct SKR_SHADER_COMPILER_API SDXCCompiledShader : public ICompiledShader {
    friend struct SDXCCompiler;

public:
    ~SDXCCompiledShader() SKR_NOEXCEPT;

    static SDXCCompiledShader* Create(ECGPUShaderStage shader_stage, ECGPUShaderBytecodeType type, IDxcBlobEncoding* source, IDxcResult* result) SKR_NOEXCEPT;

    ECGPUShaderStage         GetShaderStage() const SKR_NOEXCEPT override;
    skr::span<const uint8_t> GetBytecode() const SKR_NOEXCEPT override;
    skr::span<const uint8_t> GetPDB() const SKR_NOEXCEPT override;
    bool                     GetHashCode(uint32_t* flags, skr::span<uint32_t, 4> encoded_digits) const SKR_NOEXCEPT override;

protected:
    ECGPUShaderStage        shader_stage;
    ECGPUShaderBytecodeType code_type;
    uint32_t                spv_hash[4];

    IDxcBlobEncoding* source = nullptr; // CreateBlobFromPinned
    IDxcResult*       result = nullptr;

    IDxcBlobUtf8* errors         = nullptr;
    IDxcBlobWide* outputName     = nullptr;
    IDxcBlob*     bytecode       = nullptr;
    IDxcBlobWide* pdbName        = nullptr;
    IDxcBlob*     pdb            = nullptr;
    IDxcBlob*     hash           = nullptr;
    IDxcBlob*     reflectionData = nullptr;

    IDxcBlob* hashDigestBlob     = nullptr;
    IDxcBlob* debugDxilContainer = nullptr;
};

sreflect_struct("guid" : "fef60053-e3d6-4296-8aae-5c508896930b")
SKR_SHADER_COMPILER_API SDXCCompiler : public IShaderCompiler {
public:
    SDXCCompiler(IDxcUtils* utils, IDxcCompiler3* compiler) SKR_NOEXCEPT;
    ~SDXCCompiler() SKR_NOEXCEPT;
    static IShaderCompiler* Create() SKR_NOEXCEPT;
    static void             Free(IShaderCompiler* compiler) SKR_NOEXCEPT;

    EShaderSourceType GetSourceType() const SKR_NOEXCEPT override;
    bool              IsSupportedTargetFormat(ECGPUShaderBytecodeType format) const SKR_NOEXCEPT override;

    void SetShaderSwitches(skr::span<skr_shader_option_template_t> opt_defs, skr::span<skr_shader_option_instance_t> options, const skr_stable_shader_hash_t& hash) SKR_NOEXCEPT override;
    void SetShaderOptions(skr::span<skr_shader_option_template_t> opt_defs, skr::span<skr_shader_option_instance_t> options, const skr_stable_shader_hash_t& hash) SKR_NOEXCEPT override;

    ICompiledShader* Compile(ECGPUShaderBytecodeType format, const ShaderSourceCode& source, const SShaderImporter& importer) SKR_NOEXCEPT override;
    void             FreeCompileResult(ICompiledShader* compiled) SKR_NOEXCEPT override;

    void SetIncludeHandler(IDxcIncludeHandler* includeHandler) SKR_NOEXCEPT;

protected:
    void createDefArgsFromOptions(skr::span<skr_shader_option_template_t> opt_defs, skr::span<skr_shader_option_instance_t> options, skr::Vector<skr::stl_wstring>& def_args) SKR_NOEXCEPT;

    IDxcUtils*          utils          = nullptr;
    IDxcCompiler3*      compiler       = nullptr;
    IDxcIncludeHandler* includeHandler = nullptr;

    skr::Vector<skr_shader_option_template_t> switch_defs;
    skr::Vector<skr_shader_option_instance_t> switches;
    skr_stable_shader_hash_t                    switches_hash = {};

    skr::Vector<skr_shader_option_template_t> option_defs;
    skr::Vector<skr_shader_option_instance_t> options;
    skr_stable_shader_hash_t                    options_hash = {};
};

sreflect_struct("guid" : "ae28a9e5-39cf-4eab-aa27-6103f42cbf2d", "rttr": { "reflect_bases": false })
SKR_SHADER_COMPILER_API SDXCLibrary : public skr::ModuleSubsystem {
    friend struct DxcCreateInstanceT;

public:
    static SDXCLibrary* Get() SKR_NOEXCEPT;
    static void         LoadDXCLibrary() SKR_NOEXCEPT;
    static void         LoadDXILLibrary() SKR_NOEXCEPT;
    static void         UnloadLibraries() SKR_NOEXCEPT;

    virtual void Initialize() override;
    virtual void Finalize() override;

protected:
    skr::SharedLibrary dxc_library;
    skr::SharedLibrary dxil_library;
    void*              pDxcCreateInstance = nullptr;
};
} // namespace asset sreflect
} // namespace skd sreflect