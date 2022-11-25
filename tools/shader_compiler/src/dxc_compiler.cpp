#include <EASTL/string.h> //sv::starts_with
#include <EASTL/unique_ptr.h>
#include "platform/memory.h"
#include "utils/log.h"
#include "utils/format.hpp"
#include "SkrShaderCompiler/dxc_compiler.hpp"
#include "SkrShaderCompiler/assets/shader_asset.hpp"
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

SDXCCompiledShader* SDXCCompiledShader::Create(ECGPUShaderStage shader_stage, ECGPUShaderBytecodeType type, IDxcBlobEncoding* source, IDxcResult* result) SKR_NOEXCEPT
{
    auto compiled = SkrNew<SDXCCompiledShader>();
    const bool is_spv = (type == CGPU_SHADER_BYTECODE_TYPE_SPIRV);
    IDxcBlobUtf8* errors = nullptr;
    IDxcBlobWide* outputName = nullptr;
    IDxcBlob* bytecode = nullptr;
    IDxcBlobWide* pdbName = nullptr;
    IDxcBlob* pdb = nullptr;
    IDxcBlob* hash = nullptr;
    IDxcBlob* reflectionData = nullptr;
    uint32_t spv_hash[4];
    
    result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
    result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&bytecode), &outputName);
    if (bytecode == nullptr)
    {
        SKR_LOG_ERROR("[DXCCompiler]Unknown Error: Failed to get bytecode!");
        if (errors != nullptr && errors->GetStringLength() != 0)
        {
            SKR_LOG_ERROR("[DXCCompiler]Warnings and Errors:\n%s\n", errors->GetStringPointer());
            goto FAIL;
        }
    }
    result->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pdb), &pdbName);
    if (auto hres = result->GetOutput(DXC_OUT_SHADER_HASH, IID_PPV_ARGS(&hash), nullptr);!SUCCEEDED(hres))
    {
        if (is_spv)
        {
            auto bytes = bytecode->GetBufferPointer();
            auto byte_size = (uint32_t)bytecode->GetBufferSize();
            const uint32_t seeds[4] = { 114u, 514u, 1919u, 810u };
            for (auto i = 0u; i < 4u; i++)
                spv_hash[i] = skr_hash32(bytes, byte_size, seeds[i]);
        }
        else
        {
            SKR_LOG_ERROR("[DXCCompiler]Unknown Error: Failed to get hash data! HRESULT: %u, Target IL: %d", hres, type);
            goto FAIL;
        }
    }
    // TODO: Demonstrate getting the hash from the PDB blob using the IDxcUtils::GetPDBContents API
    //if (SUCCEEDED(utils->GetPDBContents(pdb, &hashDigestBlob, &debugDxilContainer)));
    if (auto hres = result->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&reflectionData), nullptr);!is_spv && !SUCCEEDED(hres))
    {
        SKR_LOG_ERROR("[DXCCompiler]Unknown Error: Failed to get reflection data! HRESULT: %u", hres);
    }
    compiled->shader_stage = shader_stage;
    compiled->code_type = type;
    compiled->source = source;
    compiled->errors = errors;
    compiled->result = result;
    compiled->outputName = outputName;
    compiled->bytecode = bytecode;
    compiled->pdbName = pdbName;
    compiled->pdb = pdb;
    compiled->hash = hash;
    compiled->reflectionData = reflectionData;
    compiled->spv_hash[0] = spv_hash[0];
    compiled->spv_hash[1] = spv_hash[1];
    compiled->spv_hash[2] = spv_hash[2];
    compiled->spv_hash[3] = spv_hash[3];
    return compiled;
FAIL:
    SAFE_RELEASE(source);
    SAFE_RELEASE(result);
    SAFE_RELEASE(errors);
    SAFE_RELEASE(outputName);
    SAFE_RELEASE(bytecode);
    SAFE_RELEASE(pdbName);
    SAFE_RELEASE(pdb);
    SAFE_RELEASE(hash);
    SAFE_RELEASE(reflectionData);
    SkrDelete(compiled);
    return nullptr;
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

ECGPUShaderStage SDXCCompiledShader::GetShaderStage() const SKR_NOEXCEPT
{
    return shader_stage;
}

skr::span<const uint8_t> SDXCCompiledShader::GetBytecode() const SKR_NOEXCEPT
{
    return skr::span<const uint8_t>((const uint8_t*)bytecode->GetBufferPointer(), bytecode->GetBufferSize());
}

skr::span<const uint8_t> SDXCCompiledShader::GetPDB() const SKR_NOEXCEPT
{
    if (!pdb) return {};
    return skr::span<const uint8_t>((const uint8_t*)pdb->GetBufferPointer(), pdb->GetBufferSize());
}

bool SDXCCompiledShader::GetHashCode(uint32_t* flags, skr::span<uint32_t, 4> encoded_digits) const SKR_NOEXCEPT
{
    if ((code_type == CGPU_SHADER_BYTECODE_TYPE_SPIRV) && !hash)
    {
        encoded_digits[0] = spv_hash[0];
        encoded_digits[1] = spv_hash[1];
        encoded_digits[2] = spv_hash[2];
        encoded_digits[3] = spv_hash[3];
        return true;
    }
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

inline static ECGPUShaderStage getShaderStageFromTargetString(const char* target)
{
    eastl::string_view sv = target;
    if (sv.starts_with("vs")) return CGPU_SHADER_STAGE_VERT;
    if (sv.starts_with("gs")) return CGPU_SHADER_STAGE_GEOM;
    if (sv.starts_with("ds")) return CGPU_SHADER_STAGE_DOMAIN;
    if (sv.starts_with("hs")) return CGPU_SHADER_STAGE_HULL;
    if (sv.starts_with("ps")) return CGPU_SHADER_STAGE_FRAG;
    if (sv.starts_with("cs")) return CGPU_SHADER_STAGE_COMPUTE;
    if (sv.starts_with("lib")) return CGPU_SHADER_STAGE_RAYTRACING;
    return CGPU_SHADER_STAGE_NONE;
}

void SDXCCompiler::SetShaderOptions(skr::span<skr_shader_option_instance_t> options_view, const skr_shader_options_md5_t& md5) SKR_NOEXCEPT
{
    options = eastl::vector<skr_shader_option_instance_t>(options_view.data(), options_view.data() + options_view.size());
    options_md5 = md5;
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
    
    // calculate compile arguments
    using utf8_to_utf16 = fmt::detail::utf8_to_utf16;
    const auto wTargetString = utf8_to_utf16(importer.target.c_str());
    const auto wEntryString = utf8_to_utf16(importer.entry.c_str());
    const auto wNameString = utf8_to_utf16(source.source_name.c_str());
    const auto shader_stage = getShaderStageFromTargetString(importer.target.c_str());
    eastl::vector<eastl::wstring> allArgs;
    allArgs.emplace_back(wNameString.c_str());
    if (format == CGPU_SHADER_BYTECODE_TYPE_DXIL)
    {
        allArgs.emplace_back(L"-Wno-ignored-attributes");
        allArgs.emplace_back(DXC_ARG_ALL_RESOURCES_BOUND);
    }
    if (format == CGPU_SHADER_BYTECODE_TYPE_SPIRV)
    {
        allArgs.emplace_back(L"-spirv");
        allArgs.emplace_back(L"-fspv-target-env=vulkan1.1");
    }
    // entry point
    allArgs.emplace_back(L"-E");
    allArgs.emplace_back(wEntryString.c_str()); 
    // target profile
    allArgs.emplace_back(L"-T");
    allArgs.emplace_back(wTargetString.c_str()); 
    // optimization
#if _DEBUG
    allArgs.emplace_back(DXC_ARG_DEBUG);
    allArgs.emplace_back(DXC_ARG_SKIP_OPTIMIZATIONS);
#else
    allArgs.emplace_back(DXC_ARG_OPTIMIZATION_LEVEL3);
#endif
    allArgs.emplace_back(L"-Qstrip_debug"); 

    for (auto&& option : options)
    {
        auto prefix = eastl::wstring(L"-D") + utf8_to_utf16(option.key.c_str()).c_str();
        if (option.value == "on") allArgs.emplace_back(prefix);
        else if (option.value == "off") continue;//allArgs.emplace_back(prefix);
        else
        {
            auto wvalue =  eastl::wstring(utf8_to_utf16(option.value.c_str()).c_str());
            auto defination = prefix + L"=" + wvalue; 
            allArgs.emplace_back(prefix);  
        }
    }

    // do compile
    {
        eastl::vector<LPCWSTR> pszArgs;
        pszArgs.reserve(allArgs.size());
        for (auto& arg : allArgs)
        {
            pszArgs.emplace_back(arg.c_str());
        }
        auto hres = compiler->Compile(
            &SourceBuffer,                // Source buffer.
            pszArgs.data(),                // Array of pointers to arguments.
            (UINT32)pszArgs.size(),      // Number of arguments.
            includeHandler,        // User-provided interface to handle #include directives (optional).
            IID_PPV_ARGS(&pDxcResult) // Compiler output status, buffer, and errors.
        );
        if (!SUCCEEDED(hres))
        {
            switch (hres)
            {
            case 1:
            default:
                SKR_UNREACHABLE_CODE();
            }
            return nullptr;
        }
    }
    return SDXCCompiledShader::Create(shader_stage, format, pSourceBlob, pDxcResult);
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
    skr::string filename;
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
    skr::string filename;
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

void SDXCLibrary::Initialize()
{
    SDXCLibrary::LoadDXCLibrary();
    SDXCLibrary::LoadDXILLibrary();

    Util_ShaderCompilerRegister(EShaderSourceType::HLSL, &SDXCCompiler::Create, &SDXCCompiler::Free);
}

void SDXCLibrary::Finalize()
{
    SDXCLibrary::UnloadLibraries();
}
}
}

SKR_MODULE_SUBSYSTEM(skd::asset::SDXCLibrary, SkrShaderCompiler);