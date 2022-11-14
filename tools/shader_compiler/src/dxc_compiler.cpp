#include "SkrShaderCompiler/dxc_compiler.h"
#include "platform/configure.h"
#include "platform/memory.h"
#include <EASTL/unique_ptr.h>

#include <atlbase.h> 
#include <dxcapi.h>

// helper
namespace skd
{
struct DxcCreateInstanceT
{
    static DxcCreateInstanceProc Get()
    {
        auto dxcInstance = skd::SDXCLibrary::Get();
        return (DxcCreateInstanceProc)dxcInstance->pDxcCreateInstance;
    }
};
}
// compiler

skd::SDXCCompiler::SDXCCompiler(IDxcUtils* utils, IDxcCompiler3* compiler) SKR_NOEXCEPT
    : utils(utils), compiler(compiler)
{

}

skd::SDXCCompiler::~SDXCCompiler() SKR_NOEXCEPT
{
    utils->Release();
    compiler->Release();
    if (includeHandler) includeHandler->Release();
}

void skd::SDXCCompiler::SetIncludeHandler(IDxcIncludeHandler* handler) SKR_NOEXCEPT
{
    includeHandler = handler; 
}

// library

skd::SDXCCompiler* skd::SDXCLibrary::CreateCompiler() SKR_NOEXCEPT
{
    IDxcUtils* pUtils = nullptr;
    IDxcCompiler3* pCompiler = nullptr;
    DxcCreateInstanceT::Get()(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));
    DxcCreateInstanceT::Get()(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));
    auto compierInstance = SkrNew<skd::SDXCCompiler>(pUtils, pCompiler);

    IDxcIncludeHandler* pIncludeHandler = nullptr;
    pUtils->CreateDefaultIncludeHandler(&pIncludeHandler);
    compierInstance->SetIncludeHandler(pIncludeHandler);

    return compierInstance;
}

void skd::SDXCLibrary::FreeCompiler(skd::SDXCCompiler* compiler) SKR_NOEXCEPT
{
    SkrDelete(compiler);
}

skd::SDXCLibrary* skd::SDXCLibrary::Get() SKR_NOEXCEPT
{
    static eastl::unique_ptr<SDXCLibrary> _this = eastl::make_unique<SDXCLibrary>();
    return _this.get();
}

eastl::function<void()> skd::SDXCLibrary::DXCLoader() SKR_NOEXCEPT
{
    return []() -> void {
        eastl::string filename;
        auto dxcInstance = SDXCLibrary::Get();
        auto& dxc_library = dxcInstance->dxc_library;
        filename.append(skr::SharedLibrary::GetPlatformFilePrefixName())
                .append("dxcompiler")
                .append(skr::SharedLibrary::GetPlatformFileExtensionName());
        if (auto result = dxc_library.load(filename.c_str()));
        else
        {
            SKR_LOG_ERROR("failed to load dxc library!");
        }
        auto pDxcCreateInstance = SKR_SHARED_LIB_LOAD_API(dxc_library, DxcCreateInstance);
        dxcInstance->pDxcCreateInstance = (void*)pDxcCreateInstance;
        SKR_ASSERT(dxcInstance->pDxcCreateInstance && "Fatal: PFN DxcCreateInstance is not found!");

        // Do API tests

        IDxcUtils* pTestUtils = nullptr;
        IDxcCompiler3* pTestCompiler = nullptr;
        DxcCreateInstanceT::Get()(CLSID_DxcUtils, IID_PPV_ARGS(&pTestUtils));
        DxcCreateInstanceT::Get()(CLSID_DxcCompiler, IID_PPV_ARGS(&pTestCompiler));
        
        IDxcIncludeHandler* pIncludeHandler = nullptr;
        pTestUtils->CreateDefaultIncludeHandler(&pIncludeHandler);
        SKR_ASSERT(pTestUtils && "Fatal: Failed to create default include handler for dxc!");

        pIncludeHandler->Release();
        pTestUtils->Release();
        pTestCompiler->Release();
    };
}

eastl::function<void()> skd::SDXCLibrary::DXILLoader() SKR_NOEXCEPT
{
    return []() -> void {
        eastl::string filename;
        auto dxcInstance = SDXCLibrary::Get();
        filename.append(skr::SharedLibrary::GetPlatformFilePrefixName())
                .append("dxil")
                .append(skr::SharedLibrary::GetPlatformFileExtensionName());
        if (auto result = dxcInstance->dxil_library.load(filename.c_str()));
        else
        {
            SKR_LOG_ERROR("failed to load dxil library!"
            "no correct signature will be assigned to the dxil files that shaders will be rejected by runtime driver!");
        }
    };
}

eastl::function<void()> skd::SDXCLibrary::LibrariesUnloader() SKR_NOEXCEPT
{
    return []() -> void {
        auto dxcInstance = SDXCLibrary::Get();
        dxcInstance->dxil_library.unload();
        dxcInstance->dxc_library.unload();
    };
}