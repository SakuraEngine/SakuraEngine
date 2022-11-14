#pragma once
#include "SkrShaderCompiler/module.configure.h"
#include "platform/configure.h"
#include "utils/log.h"

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

void Util_ShaderCompilerEventOnLoad(const char*, const eastl::function<void()>& event);
void Util_ShaderCompilerEventOnUnload(const char*, const eastl::function<void()>& event);

sreflect_struct("guid" : "fef60053-e3d6-4296-8aae-5c508896930b")
SKR_SHADER_COMPILER_API SDXCCompiler
{
public:
    SDXCCompiler(IDxcUtils* utils, IDxcCompiler3* compiler) SKR_NOEXCEPT;
    ~SDXCCompiler() SKR_NOEXCEPT;

    void SetIncludeHandler(IDxcIncludeHandler* includeHandler) SKR_NOEXCEPT;

protected:
    IDxcUtils* utils = nullptr;
    IDxcCompiler3* compiler = nullptr;
    IDxcIncludeHandler* includeHandler = nullptr;
};

sreflect_struct("guid" : "ae28a9e5-39cf-4eab-aa27-6103f42cbf2d")
SKR_SHADER_COMPILER_API SDXCLibrary
{
    friend struct DxcCreateInstanceT;
public:
    static SDXCLibrary* Get() SKR_NOEXCEPT;    
    static eastl::function<void()> DXCLoader() SKR_NOEXCEPT;
    static eastl::function<void()> DXILLoader() SKR_NOEXCEPT;
    static eastl::function<void()> LibrariesUnloader() SKR_NOEXCEPT;

    SDXCCompiler* CreateCompiler() SKR_NOEXCEPT;
    void FreeCompiler(SDXCCompiler* compiler) SKR_NOEXCEPT;

protected:
    skr::SharedLibrary dxc_library;
    skr::SharedLibrary dxil_library;
    void* pDxcCreateInstance = nullptr; 
}
// load dxc dll
sstatic_ctor(skd::Util_ShaderCompilerEventOnLoad("LoadDXC", skd::SDXCLibrary::DXCLoader()))
sstatic_ctor(skd::Util_ShaderCompilerEventOnLoad("LoadDXIL", skd::SDXCLibrary::DXILLoader()))
sstatic_ctor(skd::Util_ShaderCompilerEventOnUnload("LoadDXIL", skd::SDXCLibrary::LibrariesUnloader()));

} // end namespace skd
#endif