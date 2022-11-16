#pragma once
#include "shader_compiler.h"
#ifdef __cplusplus
#include "platform/shared_library.hpp"
#ifndef __meta__
#include "SkrShaderCompiler/dxc_compiler.generated.h"
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
struct SKR_SHADER_COMPILER_API SDXCCompiledShader : public ICompiledShader
{
    friend struct SDXCCompiler;
public:
    SDXCCompiledShader(ECGPUShaderBytecodeType type, IDxcBlobEncoding* source, IDxcResult* result) SKR_NOEXCEPT;
    ~SDXCCompiledShader() SKR_NOEXCEPT;

    skr::span<const uint8_t> GetBytecode() const SKR_NOEXCEPT override;
    skr::span<const uint8_t> GetPDB() const SKR_NOEXCEPT override;
    bool GetHashCode(uint32_t* flags, skr::span<uint32_t, 4> encoded_digits) const SKR_NOEXCEPT override;

protected:
    ECGPUShaderBytecodeType code_type;
    uint32_t spv_hash[4];
    
    IDxcBlobEncoding* source = nullptr;// CreateBlobFromPinned
    IDxcResult* result = nullptr;

    IDxcBlobUtf8* errors = nullptr;
    IDxcBlobWide* outputName = nullptr;
    IDxcBlob* bytecode = nullptr;
    IDxcBlobWide* pdbName = nullptr;
    IDxcBlob* pdb = nullptr;
    IDxcBlob* hash = nullptr;
    IDxcBlob* reflectionData   = nullptr;
    
    IDxcBlob* hashDigestBlob  = nullptr;
    IDxcBlob* debugDxilContainer   = nullptr;
};

sreflect_struct("guid" : "fef60053-e3d6-4296-8aae-5c508896930b")
SKR_SHADER_COMPILER_API SDXCCompiler : public IShaderCompiler
{
public:
    SDXCCompiler(IDxcUtils* utils, IDxcCompiler3* compiler) SKR_NOEXCEPT;
    ~SDXCCompiler() SKR_NOEXCEPT;
    static IShaderCompiler* Create() SKR_NOEXCEPT;
    static void Free(IShaderCompiler* compiler) SKR_NOEXCEPT;

    EShaderSourceType GetSourceType() const SKR_NOEXCEPT override;
    bool IsSupportedTargetFormat(ECGPUShaderBytecodeType format) const SKR_NOEXCEPT override;
    ICompiledShader* Compile(ECGPUShaderBytecodeType format, const ShaderSourceCode& source, const SShaderImporter& importer) SKR_NOEXCEPT override;
    void FreeCompileResult(ICompiledShader* compiled) SKR_NOEXCEPT override;

    void SetIncludeHandler(IDxcIncludeHandler* includeHandler) SKR_NOEXCEPT;

protected:
    IDxcUtils* utils = nullptr;
    IDxcCompiler3* compiler = nullptr;
    IDxcIncludeHandler* includeHandler = nullptr;
}
sstatic_ctor(Util_ShaderCompilerRegister(EShaderSourceType::HLSL, &SDXCCompiler::Create, &SDXCCompiler::Free));

sreflect_struct("guid" : "ae28a9e5-39cf-4eab-aa27-6103f42cbf2d")
SKR_SHADER_COMPILER_API SDXCLibrary
{
    friend struct DxcCreateInstanceT;
public:
    static SDXCLibrary* Get() SKR_NOEXCEPT;    
    static void LoadDXCLibrary() SKR_NOEXCEPT;
    static void LoadDXILLibrary() SKR_NOEXCEPT;
    static void UnloadLibraries() SKR_NOEXCEPT;

protected:
    skr::SharedLibrary dxc_library;
    skr::SharedLibrary dxil_library;
    void* pDxcCreateInstance = nullptr; 
}
// load dxc dll
sstatic_ctor(Util_ShaderCompilerEventOnLoad("LoadDXC", &SDXCLibrary::LoadDXCLibrary))
sstatic_ctor(Util_ShaderCompilerEventOnLoad("LoadDXIL", &SDXCLibrary::LoadDXILLibrary))
sstatic_ctor(Util_ShaderCompilerEventOnUnload("LoadDXIL", &SDXCLibrary::UnloadLibraries));
} // namespace asset
} // namespace skd
#endif