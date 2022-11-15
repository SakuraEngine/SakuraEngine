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

namespace skd sreflect
{

sreflect_struct("guid" : "fef60053-e3d6-4296-8aae-5c508896930b")
SKR_SHADER_COMPILER_API SDXCCompiler : public IShaderCompiler
{
public:
    SDXCCompiler(IDxcUtils* utils, IDxcCompiler3* compiler) SKR_NOEXCEPT;
    ~SDXCCompiler() SKR_NOEXCEPT;
    static IShaderCompiler* Create() SKR_NOEXCEPT;

    void SetIncludeHandler(IDxcIncludeHandler* includeHandler) SKR_NOEXCEPT;

protected:
    IDxcUtils* utils = nullptr;
    IDxcCompiler3* compiler = nullptr;
    IDxcIncludeHandler* includeHandler = nullptr;
}
sstatic_ctor(skd::Util_ShaderCompilerRegister(skd::asset::EShaderSourceType::SHADER_SOURCE_TYPE_HLSL, &skd::SDXCCompiler::Create));

sreflect_struct("guid" : "ae28a9e5-39cf-4eab-aa27-6103f42cbf2d")
SKR_SHADER_COMPILER_API SDXCLibrary
{
    friend struct DxcCreateInstanceT;
public:
    static SDXCLibrary* Get() SKR_NOEXCEPT;    
    static void LoadDXCLibrary() SKR_NOEXCEPT;
    static void LoadDXILLibrary() SKR_NOEXCEPT;
    static void UnloadLibraries() SKR_NOEXCEPT;

    SDXCCompiler* CreateCompiler() SKR_NOEXCEPT;
    void FreeCompiler(SDXCCompiler* compiler) SKR_NOEXCEPT;

protected:
    skr::SharedLibrary dxc_library;
    skr::SharedLibrary dxil_library;
    void* pDxcCreateInstance = nullptr; 
}
// load dxc dll
sstatic_ctor(skd::Util_ShaderCompilerEventOnLoad("LoadDXC", &skd::SDXCLibrary::LoadDXCLibrary))
sstatic_ctor(skd::Util_ShaderCompilerEventOnLoad("LoadDXIL", &skd::SDXCLibrary::LoadDXILLibrary))
sstatic_ctor(skd::Util_ShaderCompilerEventOnUnload("LoadDXIL", &skd::SDXCLibrary::UnloadLibraries));

} // end namespace skd
#endif