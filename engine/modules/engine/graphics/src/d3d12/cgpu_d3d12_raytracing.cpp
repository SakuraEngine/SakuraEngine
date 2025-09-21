#include "SkrContainersDef/string.hpp"
#include "SkrContainersDef/map.hpp"
#include "SkrContainersDef/set.hpp"
#include "SkrContainersDef/span.hpp"
#include "SkrGraphics/backend/d3d12/cgpu_d3d12_raytracing.h"
#include "d3d12_utils.h"

struct ShaderEntryStore
{
    ShaderEntryStore(const CGPUShaderEntryDescriptor& desc)
        : library(desc.library)
        , entry(desc.entry)
    {
    }
    bool operator==(const CGPUShaderEntryDescriptor& x) const
    {
        return library == x.library && entry == x.entry;
    }
    CGPUShaderLibraryId library = nullptr;
    skr::String entry = u8"";
};

namespace skr
{
template <>
struct Hash<std::wstring>
{
    inline skr_hash operator()(const std::wstring& x) const
    {
        return std::hash<std::wstring>()(x);
    }
};

template <>
struct Hash<ShaderEntryStore>
{
    inline skr_hash operator()(const ShaderEntryStore& x) const
    {
        auto entryHash = skr::Hash<skr::String>()(x.entry);
        return hash_combine(entryHash, (uint64_t)x.library);
    }
    inline skr_hash operator()(const CGPUShaderEntryDescriptor& x) const
    {
        auto entryHash = skr::Hash<skr::StringView>()(x.entry);
        return hash_combine(entryHash, (uint64_t)x.library);
    }
};
} // namespace skr

struct CGPURayPipeline_D3D12 final : public CGPURayPipelineBase_D3D12
{
public:
    CGPURayPipeline_D3D12(const CGPURayPipelineDescriptor* desc, skr::Span<const CGPURayPipelineHitGroup> hitGroups)
        : name(desc->name)
    {
        UniqueShaderEntries.add(desc->raygen);
        UniqueShaderEntries.add(desc->miss);
        for (auto hitGroup : hitGroups)
        {
            if (hitGroup.anyhit.library != nullptr && hitGroup.anyhit.entry != nullptr)
            {
                UniqueShaderEntries.add(hitGroup.anyhit);
            }
            if (hitGroup.closest.library != nullptr && hitGroup.closest.entry != nullptr)
            {
                UniqueShaderEntries.add(hitGroup.closest);
            }
            if (hitGroup.intersection.library != nullptr && hitGroup.intersection.entry != nullptr)
            {
                UniqueShaderEntries.add(hitGroup.intersection);
            }
        }
        for (auto entry : UniqueShaderEntries)
        {
            auto& symbols = ExportSymbolMap.try_add_default(entry.library).value();
            symbols.add(entry.entry);
        }
    }

    struct Builder
    {
        D3D12_RAYTRACING_SHADER_CONFIG CONFIG;
        skr::Vector<D3D12_STATE_SUBOBJECT> SUBOBJECTS;
        skr::Vector<D3D12_DXIL_LIBRARY_DESC> LIBS;
        skr::Vector<D3D12_EXPORT_DESC> EXPORTS;
        skr::Vector<D3D12_HIT_GROUP_DESC> HITGROUPS;

        std::wstring RAYGEN_NAME;
        std::wstring MISS_NAME;
        skr::Set<std::wstring> SYMBOLS;
        skr::Vector<std::wstring> HITGROUP_NAMES;
        skr::Vector<std::wstring> ANYHIT_NAMES;
        skr::Vector<std::wstring> CLOSEST_NAMES;
        skr::Vector<std::wstring> INTERSECTION_NAMES;
        skr::Set<std::wstring> GEN_MISS_HITGROUPS;
    };

    bool CreateDummyRootSignatures(CGPUDeviceId device)
    {
        const CGPUDevice_D3D12* D = (const CGPUDevice_D3D12*)(device);
        auto Device = D->pDxDevice;
        // Creation of the global root signature
        D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
        rootDesc.NumParameters = 0;
        rootDesc.pParameters = nullptr;
        // A global root signature is the default, hence this flag
        rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

        HRESULT hr = 0;

        ID3DBlob* serializedRootSignature;
        ID3DBlob* error;

        // Create the local root signature, reusing the same descriptor but altering the creation flag
        rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
        hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSignature, &error);
        if (FAILED(hr))
        {
            cgpu_assert(0 && "Could not serialize the local root signature");
        }
        hr = Device->CreateRootSignature(0, serializedRootSignature->GetBufferPointer(), serializedRootSignature->GetBufferSize(), IID_PPV_ARGS(&DummyLocalRootSignature));

        serializedRootSignature->Release();
        if (FAILED(hr))
        {
            cgpu_assert(0 && "Could not create the local root signature");
        }
        return true;
    }

    bool CreateRootSignature(CGPUDeviceId device, const CGPURayPipelineDescriptor* desc)
    {
        CreateDummyRootSignatures(device);

        skr::Vector<CGPUShaderEntryDescriptor> entries;
        for (auto entry : UniqueShaderEntries)
        {
            CGPUShaderEntryDescriptor desc = {
                .library = entry.library,
                .entry = entry.entry.c_str()
            };
            entries.add(desc);
        }

        CGPURootSignatureDescriptor RSDesc = {
            .shaders = entries.data(),
            .shader_count = (uint32_t)entries.size(),
            .static_samplers = desc->static_samplers,
            .static_sampler_names = desc->static_sampler_names,
            .static_sampler_count = desc->static_sampler_count,
            .push_constant_names = desc->push_constant_names,
            .push_constant_count = desc->push_constant_count,
            .name = name.c_str(),
        };
        super.root_signature = cgpu_create_root_signature(device, &RSDesc);
        return super.root_signature;
    }

    bool CreateShaderSubObjects(Builder& builder)
    {
        for (auto&& [DXIL, Symbols] : ExportSymbolMap)
        {
            const CGPUShaderLibrary_D3D12* SL = (const CGPUShaderLibrary_D3D12*)DXIL;
            auto& LIB = builder.LIBS.add_default().ref();
            LIB.DXILLibrary.pShaderBytecode = D3D12Util_GetShaderBlobData(SL->pShaderBlob);
            LIB.DXILLibrary.BytecodeLength = D3D12Util_GetShaderBlobSize(SL->pShaderBlob);
            LIB.NumExports = static_cast<UINT>(Symbols.size());
            LIB.pExports = builder.EXPORTS.data() + builder.EXPORTS.size();

            for (auto Symbol : Symbols)
            {
                std::wstring _WSYMBOL;
                _WSYMBOL.resize(Symbol.to_u16_length());
                Symbol.to_u16((skr_char16*)_WSYMBOL.data());
                auto& WSYMBOL = builder.SYMBOLS.add(_WSYMBOL).ref();

                D3D12_EXPORT_DESC& EXPORT = builder.EXPORTS.add_default().ref();
                EXPORT.Name = WSYMBOL.c_str();
                EXPORT.ExportToRename = nullptr;
                EXPORT.Flags = D3D12_EXPORT_FLAG_NONE;
            }

            D3D12_STATE_SUBOBJECT& SUBOBJECT = builder.SUBOBJECTS.add_default().ref();
            SUBOBJECT.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
            SUBOBJECT.pDesc = &LIB;
        }

        return true;
    }

    bool CreateHitGroupSubObjects(Builder& builder, const CGPURayPipelineDescriptor* desc, skr::Span<const CGPURayPipelineHitGroup> hitGroups)
    {
        skr::StringView RayGenName = desc->raygen.entry;
        builder.RAYGEN_NAME.resize(RayGenName.to_u16_length());
        RayGenName.to_u16((skr_char16*)builder.RAYGEN_NAME.data());
        builder.GEN_MISS_HITGROUPS.add(builder.RAYGEN_NAME);

        skr::StringView MissName = desc->miss.entry;
        builder.MISS_NAME.resize(MissName.to_u16_length());
        MissName.to_u16((skr_char16*)builder.MISS_NAME.data());
        builder.GEN_MISS_HITGROUPS.add(builder.MISS_NAME);

        for (auto HitGroup : hitGroups)
        {
            skr::StringView HitGroupName = HitGroup.name;
            auto& HITGROUP_NAME = builder.HITGROUP_NAMES.add_default().ref();
            HITGROUP_NAME.resize(HitGroupName.to_u16_length());
            HitGroupName.to_u16((skr_char16*)HITGROUP_NAME.data());
            builder.GEN_MISS_HITGROUPS.add(HITGROUP_NAME);

            skr::StringView AnyHitName = HitGroup.anyhit.entry;
            auto& ANYHIT_NAME = builder.ANYHIT_NAMES.add_default().ref();
            ANYHIT_NAME.resize(AnyHitName.to_u16_length());
            AnyHitName.to_u16((skr_char16*)ANYHIT_NAME.data());

            skr::StringView ClosestHitName = HitGroup.closest.entry;
            auto& CLOSEST_NAME = builder.CLOSEST_NAMES.add_default().ref();
            CLOSEST_NAME.resize(ClosestHitName.to_u16_length());
            ClosestHitName.to_u16((skr_char16*)CLOSEST_NAME.data());

            skr::StringView IntersectionName = HitGroup.intersection.entry;
            auto& INTERSECTION_NAME = builder.INTERSECTION_NAMES.add_default().ref();
            INTERSECTION_NAME.resize(IntersectionName.to_u16_length());
            IntersectionName.to_u16((skr_char16*)INTERSECTION_NAME.data());

            auto& HITGROUP = builder.HITGROUPS.add_default().ref();
            HITGROUP.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
            HITGROUP.HitGroupExport = HITGROUP_NAME.empty() ? nullptr : HITGROUP_NAME.c_str();
            HITGROUP.AnyHitShaderImport = ANYHIT_NAME.empty() ? nullptr : ANYHIT_NAME.c_str();
            HITGROUP.ClosestHitShaderImport = CLOSEST_NAME.empty() ? nullptr : CLOSEST_NAME.c_str();
            HITGROUP.IntersectionShaderImport = INTERSECTION_NAME.empty() ? nullptr : INTERSECTION_NAME.c_str();

            D3D12_STATE_SUBOBJECT& SUBOBJECT = builder.SUBOBJECTS.add_default().ref();
            SUBOBJECT.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
            SUBOBJECT.pDesc = &HITGROUP;
        }
        return true;
    }

    bool Create(CGPUDeviceId device, const CGPURayPipelineDescriptor* desc, skr::Span<const CGPURayPipelineHitGroup> hitGroups)
    {
        uint64_t subobjectCount =
            hitGroups.size() +       // 1. Hit group declarations
            1 +                      // 2. Shader configuration
            1 +                      // 3. Shader payload
            2 +                      // 4. global and local root signatures
            1 + ExportSymbolMap.size() + // 5. DXIL libraries & Export from RS 
            1;                       // 6. Pipeline config
        Builder builder;
        builder.LIBS.reserve(ExportSymbolMap.size());
        builder.EXPORTS.reserve(UniqueShaderEntries.size());
        builder.SYMBOLS.reserve(UniqueShaderEntries.size());
        builder.HITGROUPS.reserve(hitGroups.size());
        builder.HITGROUP_NAMES.reserve(hitGroups.size());
        builder.ANYHIT_NAMES.reserve(hitGroups.size());
        builder.CLOSEST_NAMES.reserve(hitGroups.size());
        builder.INTERSECTION_NAMES.reserve(hitGroups.size());
        builder.SUBOBJECTS.reserve(subobjectCount);

        const CGPUDevice_D3D12* D = (const CGPUDevice_D3D12*)(device);
        bool Success = true;

        // 1. HITGROUPS
        Success |= CreateHitGroupSubObjects(builder, desc, hitGroups);

        // 2. SHADER CONFIG
        builder.CONFIG.MaxPayloadSizeInBytes = desc->max_payload_size;
        builder.CONFIG.MaxAttributeSizeInBytes = desc->max_attribute_size;
        D3D12_STATE_SUBOBJECT& shaderConfigObject = builder.SUBOBJECTS.add_default().ref();
        shaderConfigObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
        shaderConfigObject.pDesc = &builder.CONFIG;

        // 3. SHADER PAYLOAD
        D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION shaderPayloadAssociation = {};
        skr::Vector<LPCWSTR> exportedSymbolPointers;
        exportedSymbolPointers.reserve(builder.GEN_MISS_HITGROUPS.size());
        for (const auto& name : builder.GEN_MISS_HITGROUPS)
        {
            exportedSymbolPointers.push_back(name.c_str());
        }
        shaderPayloadAssociation.NumExports = static_cast<UINT>(exportedSymbolPointers.size());
        shaderPayloadAssociation.pExports = exportedSymbolPointers.data();
        shaderPayloadAssociation.pSubobjectToAssociate = &shaderConfigObject;

        D3D12_STATE_SUBOBJECT& shaderPayloadAssociationObject = builder.SUBOBJECTS.add_default().ref();
        shaderPayloadAssociationObject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
        shaderPayloadAssociationObject.pDesc = &shaderPayloadAssociation;

        // 4. Root Signatrue
        Success |= CreateRootSignature(device, desc);
        const CGPURootSignature_D3D12* RS = (const CGPURootSignature_D3D12*)super.root_signature;
        D3D12_STATE_SUBOBJECT& globalRootSig = builder.SUBOBJECTS.add_default().ref();
        globalRootSig.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
        ID3D12RootSignature* dgSig = RS->pDxRootSignature;
        globalRootSig.pDesc = &dgSig;

        D3D12_STATE_SUBOBJECT& dummyLocalRootSig = builder.SUBOBJECTS.add_default().ref();
        dummyLocalRootSig.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
        ID3D12RootSignature* dlSig = DummyLocalRootSignature;
        dummyLocalRootSig.pDesc = &dlSig;

        // 5. Export from DXIL + RS
        Success |= CreateShaderSubObjects(builder);
        // Add a subobject for the association between the exported shader symbols and the root signature
        D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION assoc = {};
        assoc.NumExports = static_cast<UINT>(exportedSymbolPointers.size());
        assoc.pExports = exportedSymbolPointers.data();
        assoc.pSubobjectToAssociate = &globalRootSig;
        D3D12_STATE_SUBOBJECT& rootSigAssociationObject = builder.SUBOBJECTS.add_default().ref();
        rootSigAssociationObject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
        rootSigAssociationObject.pDesc = &assoc;

        // 6. Add a subobject for the ray tracing pipeline configuration
        D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
        pipelineConfig.MaxTraceRecursionDepth = desc->max_recursion_depth;

        D3D12_STATE_SUBOBJECT& pipelineConfigObject = builder.SUBOBJECTS.add_default().ref();
        pipelineConfigObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
        pipelineConfigObject.pDesc = &pipelineConfig;

        // Describe the ray tracing pipeline state object
        D3D12_STATE_OBJECT_DESC pipelineDesc = {};
        pipelineDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
        pipelineDesc.NumSubobjects = builder.SUBOBJECTS.size(); // static_cast<UINT>(subobjects.size());
        pipelineDesc.pSubobjects = builder.SUBOBJECTS.data();
        cgpu_assert(subobjectCount == builder.SUBOBJECTS.size());

        // Create the state object
        HRESULT hr = D->pDxDevice5->CreateStateObject(&pipelineDesc, IID_PPV_ARGS(&StateObject));
        if (FAILED(hr))
        {
            cgpu_assert(0 && "Could not create the raytracing state object");
        }

        hr = StateObject->QueryInterface(IID_PPV_ARGS(&StateObjectProps));
        if (FAILED(hr))
        {
            cgpu_assert(0 && "Could not query props from the raytracing state object");
        }

        // TODO: BINDING?
        const auto kTempTableSizeInBytes = CGPU_ALIGN((8 + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES), 64);
        CGPUBufferDescriptor buffer_desc = {
            .size = kTempTableSizeInBytes * (2 + hitGroups.size()),
            .usages = CGPU_BUFFER_USAGE_SHADER_READ,
            .memory_usage = CGPU_MEM_USAGE_CPU_TO_GPU,
            .flags = CGPU_BUFFER_FLAG_PERSISTENT_MAP_BIT
        };
        SBT = cgpu_create_buffer(device, &buffer_desc);
        uint8_t* pData = (uint8_t*)SBT->info->cpu_mapped_address;
        memcpy(pData, StateObjectProps->GetShaderIdentifier(builder.RAYGEN_NAME.c_str()), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
        memcpy(pData + kTempTableSizeInBytes, StateObjectProps->GetShaderIdentifier(builder.MISS_NAME.c_str()), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
        
        auto pHitGroupsStart = pData + kTempTableSizeInBytes * 2;
        for (uint32_t i = 0; i < hitGroups.size(); i++)
        {
            auto pHitGroup = pHitGroupsStart + (i * kTempTableSizeInBytes);        
            memcpy(pHitGroup, StateObjectProps->GetShaderIdentifier(builder.HITGROUP_NAMES[i].c_str()), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
        }
        
        const CGPUBuffer_D3D12* B = (const CGPUBuffer_D3D12*)SBT;
        RayGenerationShaderRecord.StartAddress = B->pDxResource->GetGPUVirtualAddress();
        RayGenerationShaderRecord.SizeInBytes = kTempTableSizeInBytes;

        MissShaderTable.StartAddress = RayGenerationShaderRecord.StartAddress + kTempTableSizeInBytes;
        MissShaderTable.StrideInBytes = kTempTableSizeInBytes;
        MissShaderTable.SizeInBytes = kTempTableSizeInBytes;

        HitGroupTable.StartAddress = MissShaderTable.StartAddress + kTempTableSizeInBytes;
        HitGroupTable.StrideInBytes = kTempTableSizeInBytes;
        HitGroupTable.SizeInBytes = kTempTableSizeInBytes * hitGroups.size();
        
        return true;
    }

    skr::String name = u8"";
    skr::Vector<ShaderEntryStore> UniqueShaderEntries;
    skr::Map<CGPUShaderLibraryId, skr::Vector<skr::StringView>> ExportSymbolMap;
    ID3D12RootSignature* DummyLocalRootSignature = nullptr;

};

CGPURayPipelineId cgpu_create_ray_pipeline_d3d12(CGPUDeviceId device, const struct CGPURayPipelineDescriptor* desc)
{
    auto hitGroups = skr::Span<const CGPURayPipelineHitGroup>(desc->hit_groups, desc->hit_groups_count);
    CGPURayPipeline_D3D12* pipeline = cgpu_new<CGPURayPipeline_D3D12>(desc, hitGroups);
    pipeline->Create(device, desc, hitGroups);
    return &pipeline->super;
}

void cgpu_free_ray_pipeline_d3d12(CGPURayPipelineId pipeline)
{
    CGPURayPipeline_D3D12* P = (CGPURayPipeline_D3D12*)pipeline;
    cgpu_free_buffer(P->SBT);
    P->StateObjectProps->Release();
    P->StateObject->Release();
    P->DummyLocalRootSignature->Release();
    cgpu_free_root_signature(P->super.root_signature);
    cgpu_delete(P);
}