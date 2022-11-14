#pragma once
#include "SkrShaderCompiler/module.configure.h"
#include "platform/configure.h"

#ifdef __cplusplus
#include "platform/shared_library.hpp"
#ifndef __meta__
#include "SkrShaderCompiler/dxc_compiler.generated.h"
#endif 

SKR_SHADER_COMPILER_EXTERN_C 
void Util_ShaderCompilerEventOnLoad(const char*, eastl::function<void()> event);

sreflect_struct("guid" : "ae28a9e5-39cf-4eab-aa27-6103f42cbf2d")
sattr("rtti" : true)
SKR_SHADER_COMPILER_API SDXCLibrary
{
    sattr("no-rtti" : true)
    skr::SharedLibrary dxc_library;
    sattr("no-rtti" : true)
    skr::SharedLibrary dxil_library;
}
sstatic_ctor(Util_ShaderCompilerEventOnLoad("LoadDXC", []() -> void {

}))
sstatic_ctor(Util_ShaderCompilerEventOnLoad("LoadDXIL", []() -> void {

}));

#endif