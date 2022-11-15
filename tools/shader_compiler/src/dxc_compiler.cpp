#include "SkrShaderCompiler/dxc_compiler.h"
#include "SkrShaderCompiler/assets/shader_asset.h"
#include "platform/memory.h"
#include "utils/log.h"
#include <EASTL/unique_ptr.h>

#ifdef _WIN32
#include <atlbase.h>
#endif
#include "dxc/dxcapi.h"

// helper
namespace skd
{
namespace asset
{
struct DxcCreateInstanceT
{
    static DxcCreateInstanceProc Get()
    {
        auto dxcInstance = SDXCLibrary::Get();
        return (DxcCreateInstanceProc)dxcInstance->pDxcCreateInstance;
    }
};

// compiled shader

#define SAFE_RELEASE(ptr) if (ptr) { ptr->Release(); ptr = nullptr; }

SDXCCompiledShader::SDXCCompiledShader(ECGPUShaderBytecodeType type,IDxcBlobEncoding* source, IDxcResult* result) SKR_NOEXCEPT
    : code_type(type), source(source), result(result)
{
    result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
    result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&bytecode), &outputName);
    if (bytecode == nullptr)
    {
        SKR_LOG_ERROR("[DXCCompiler]Unknown Error: Failed to get bytecode!");
        if (errors != nullptr && errors->GetStringLength() != 0)
        {
            SKR_LOG_ERROR("[DXCCompiler]Warnings and Errors:\n%s\n", errors->GetStringPointer());
        }
        return;
    }
    result->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pdb), &pdbName);
    if (auto hres = result->GetOutput(DXC_OUT_SHADER_HASH, IID_PPV_ARGS(&hash), nullptr);!SUCCEEDED(hres))
    {
        SKR_LOG_ERROR("[DXCCompiler]Unknown Error: Failed to get hash data! HRESULT: %u", hres);
        return;
    }
    // TODO: Demonstrate getting the hash from the PDB blob using the IDxcUtils::GetPDBContents API
    //if (SUCCEEDED(utils->GetPDBContents(pdb, &hashDigestBlob, &debugDxilContainer)));
    if (auto hres = result->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&reflectionData), nullptr);!SUCCEEDED(hres))
    {
        SKR_LOG_ERROR("[DXCCompiler]Unknown Error: Failed to get reflection data! HRESULT: %u", hres);
        return;
    }
}

SDXCCompiledShader::~SDXCCompiledShader() SKR_NOEXCEPT
{
    SAFE_RELEASE(source);
    SAFE_RELEASE(result);

    SAFE_RELEASE(errors);
    SAFE_RELEASE(outputName);
    SAFE_RELEASE(bytecode);
    SAFE_RELEASE(pdbName);
    SAFE_RELEASE(pdb);

    SAFE_RELEASE(hash);
    SAFE_RELEASE(reflectionData);

    SAFE_RELEASE(hashDigestBlob);
    SAFE_RELEASE(debugDxilContainer);
}

skr::span<const uint8_t> SDXCCompiledShader::GetBytecode() const SKR_NOEXCEPT
{
    return skr::span<const uint8_t>((const uint8_t*)bytecode->GetBufferPointer(), bytecode->GetBufferSize());
}

bool SDXCCompiledShader::GetHashCode(uint32_t* flags, skr::span<uint32_t, 4> encoded_digits) const SKR_NOEXCEPT
{
    if (!hash) return false;
    if (DxcShaderHash* pHashBuf = (DxcShaderHash*)hash->GetBufferPointer())
    {
        *flags = pHashBuf->Flags;
        encoded_digits[0] = *(uint32_t*)pHashBuf->HashDigest;
        encoded_digits[1] = *(uint32_t*)(pHashBuf->HashDigest + 4);
        encoded_digits[2] = *(uint32_t*)(pHashBuf->HashDigest + 8);
        encoded_digits[3] = *(uint32_t*)(pHashBuf->HashDigest + 12);
        return true;
    }
    return true;
}

// compiler

SDXCCompiler::SDXCCompiler(IDxcUtils* utils, IDxcCompiler3* compiler) SKR_NOEXCEPT
    : utils(utils), compiler(compiler)
{

}

SDXCCompiler::~SDXCCompiler() SKR_NOEXCEPT
{
    SAFE_RELEASE(utils);
    SAFE_RELEASE(compiler);
    SAFE_RELEASE(includeHandler);
}

IShaderCompiler* SDXCCompiler::Create() SKR_NOEXCEPT
{
    IDxcUtils* pUtils = nullptr;
    IDxcCompiler3* pCompiler = nullptr;
    DxcCreateInstanceT::Get()(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));
    DxcCreateInstanceT::Get()(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));
    auto compierInstance = SkrNew<SDXCCompiler>(pUtils, pCompiler);

    IDxcIncludeHandler* pIncludeHandler = nullptr;
    pUtils->CreateDefaultIncludeHandler(&pIncludeHandler);
    compierInstance->SetIncludeHandler(pIncludeHandler);

    return compierInstance;
}

void SDXCCompiler::Free(skd::asset::IShaderCompiler* compiler) SKR_NOEXCEPT { SkrDelete(compiler); }

EShaderSourceType SDXCCompiler::GetSourceType() const SKR_NOEXCEPT { return EShaderSourceType::HLSL; }

bool SDXCCompiler::IsSupportedTargetFormat(ECGPUShaderBytecodeType format) const SKR_NOEXCEPT
{
    return (format == CGPU_SHADER_BYTECODE_TYPE_DXIL) || (format == CGPU_SHADER_BYTECODE_TYPE_SPIRV);
}

ICompiledShader* SDXCCompiler::Compile(ECGPUShaderBytecodeType format, const ShaderSourceCode& source, const SShaderImporter& importer) SKR_NOEXCEPT
{
    IDxcBlobEncoding* pSourceBlob = nullptr;
    IDxcResult* pDxcResult = nullptr;
    if (auto hr = utils->CreateBlobFromPinned(source.bytes, (uint32_t)source.size, DXC_CP_ACP, &pSourceBlob);!SUCCEEDED(hr))
    {
        SKR_LOG_ERROR("DXC Compiler: Failed to create blob from pinned memory, HRESULT: %u!", hr);
    }
    DxcBuffer SourceBuffer;
    SourceBuffer.Ptr = pSourceBlob->GetBufferPointer();
    SourceBuffer.Size = pSourceBlob->GetBufferSize();
    SourceBuffer.Encoding = DXC_CP_ACP; // Assume BOM says UTF8 or UTF16 or this is ANSI text.
    
    const auto& targetString = importer.target;
    const auto wTargetString = fmt::detail::utf8_to_utf16(targetString.c_str()).str();
    LPCWSTR pszArgs[] =
    {
        L"myshader.hlsl",       // Optional shader source file name for error reporting
                                    // and for PIX shader source view.  
        L"-E", L"main",              // Entry point.
        L"-T", wTargetString.c_str(),            // Target.
        L"-Zs",                      // Enable debug information (slim format)
        // L"-D", L"MYDEFINE=1",        // A single define.
        L"-Fo", L"myshader.bin",     // Optional. Stored in the pdb. 
        L"-Fd", L"myshader.pdb",     // The file name of the pdb. This must either be supplied
                                            // or the autogenerated file name must be used.
        // L"-Qstrip_reflect",          // Strip reflection into a separate blob. 
    };

    auto hres = compiler->Compile(
        &SourceBuffer,                // Source buffer.
        pszArgs,                // Array of pointers to arguments.
        _countof(pszArgs),      // Number of arguments.
        includeHandler,        // User-provided interface to handle #include directives (optional).
        IID_PPV_ARGS(&pDxcResult) // Compiler output status, buffer, and errors.
    );

    if (!SUCCEEDED(hres))
    {
        switch (hres)
        {
        default:
            SKR_UNREACHABLE_CODE();
        }
        return nullptr;
    }
    return SkrNew<SDXCCompiledShader>(format, pSourceBlob, pDxcResult);
}

void SDXCCompiler::FreeCompileResult(ICompiledShader* compiled) SKR_NOEXCEPT { SkrDelete(compiled); } 

void SDXCCompiler::SetIncludeHandler(IDxcIncludeHandler* handler) SKR_NOEXCEPT
{
    includeHandler = handler; 
}

// library

SDXCLibrary* SDXCLibrary::Get() SKR_NOEXCEPT
{
    static eastl::unique_ptr<SDXCLibrary> _this = eastl::make_unique<SDXCLibrary>();
    return _this.get();
}

void SDXCLibrary::LoadDXCLibrary() SKR_NOEXCEPT
{
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
}

void SDXCLibrary::LoadDXILLibrary() SKR_NOEXCEPT
{
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
}

void SDXCLibrary::UnloadLibraries() SKR_NOEXCEPT
{
    auto dxcInstance = SDXCLibrary::Get();
    dxcInstance->dxil_library.unload();
    dxcInstance->dxc_library.unload();
}

}
}