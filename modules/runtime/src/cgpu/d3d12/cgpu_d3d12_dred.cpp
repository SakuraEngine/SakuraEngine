#include "d3d12_utils.h"
#include "utils/make_zeroed.hpp"
#include "containers/span.hpp"
#include <EASTL/vector_map.h>

// Should match all valid values from D3D12_DRED_ALLOCATION_TYPE
static const TCHAR* D3D12_AllocTypesNames[] =
{
    TEXT("CommandQueue"),
    TEXT("CommandAllocator"),
    TEXT("PipelineState"),
    TEXT("CommandList"),
    TEXT("Fence"),
    TEXT("DescriptorHeap"),
    TEXT("Heap"),
    TEXT("Unknown"),				// Unknown type - missing enum value in D3D12_DRED_ALLOCATION_TYPE
    TEXT("QueryHeap"),
    TEXT("CommandSignature"),
    TEXT("PipelineLibrary"),
    TEXT("VideoDecoder"),
    TEXT("Unknown"),				// Unknown type - missing enum value in D3D12_DRED_ALLOCATION_TYPE
    TEXT("VideoProcessor"),
    TEXT("Unknown"),				// Unknown type - missing enum value in D3D12_DRED_ALLOCATION_TYPE
    TEXT("Resource"),
    TEXT("Pass"),
    TEXT("CryptoSession"),
    TEXT("CryptoSessionPolicy"),
    TEXT("ProtectedResourceSession"),
    TEXT("VideoDecoderHeap"),
    TEXT("CommandPool"),
    TEXT("CommandRecorder"),
    TEXT("StateObjectr"),
    TEXT("MetaCommand"),
    TEXT("SchedulingGroup"),
    TEXT("VideoMotionEstimator"),
    TEXT("VideoMotionVectorHeap"),
    TEXT("VideoExtensionCommand"),
};
static constexpr uint32_t D3D12_AllocTypesNamesCount = sizeof(D3D12_AllocTypesNames) / sizeof(D3D12_AllocTypesNames[0]);
static_assert(D3D12_AllocTypesNamesCount == D3D12_DRED_ALLOCATION_TYPE_VIDEO_EXTENSION_COMMAND - D3D12_DRED_ALLOCATION_TYPE_COMMAND_QUEUE + 1, 
    "AllocTypes array length mismatch");

// Should match all values from D3D12_AUTO_BREADCRUMB_OP
static const TCHAR* D3D12_OpNames[] =
{
    TEXT("SetMarker"),
    TEXT("BeginEvent"),
    TEXT("EndEvent"),
    TEXT("DrawInstanced"),
    TEXT("DrawIndexedInstanced"),
    TEXT("ExecuteIndirect"),
    TEXT("Dispatch"),
    TEXT("CopyBufferRegion"),
    TEXT("CopyTextureRegion"),
    TEXT("CopyResource"),
    TEXT("CopyTiles"),
    TEXT("ResolveSubresource"),
    TEXT("ClearRenderTargetView"),
    TEXT("ClearUnorderedAccessView"),
    TEXT("ClearDepthStencilView"),
    TEXT("ResourceBarrier"),
    TEXT("ExecuteBundle"),
    TEXT("Present"),
    TEXT("ResolveQueryData"),
    TEXT("BeginSubmission"),
    TEXT("EndSubmission"),
    TEXT("DecodeFrame"),
    TEXT("ProcessFrames"),
    TEXT("AtomicCopyBufferUint"),
    TEXT("AtomicCopyBufferUint64"),
    TEXT("ResolveSubresourceRegion"),
    TEXT("WriteBufferImmediate"),
    TEXT("DecodeFrame1"),
    TEXT("SetProtectedResourceSession"),
    TEXT("DecodeFrame2"),
    TEXT("ProcessFrames1"),
    TEXT("BuildRaytracingAccelerationStructure"),
    TEXT("EmitRaytracingAccelerationStructurePostBuildInfo"),
    TEXT("CopyRaytracingAccelerationStructure"),
    TEXT("DispatchRays"),
    TEXT("InitializeMetaCommand"),
    TEXT("ExecuteMetaCommand"),
    TEXT("EstimateMotion"),
    TEXT("ResolveMotionVectorHeap"),
    TEXT("SetPipelineState1"),
    TEXT("InitializeExtensionCommand"),
    TEXT("ExecuteExtensionCommand"),
};
static constexpr uint32_t D3D12_OpNamesCount = sizeof(D3D12_OpNames) / sizeof(D3D12_OpNames[0]);
static_assert(D3D12_OpNamesCount == D3D12_AUTO_BREADCRUMB_OP_EXECUTEEXTENSIONCOMMAND + 1, "OpNames array length mismatch");

template<typename T>
void D3D12Util_LogDREDPageFaultImpl(const T* pageFault)
{
    cgpu_error("Gathered page fault allocation output.");
    
    D3D12_GPU_VIRTUAL_ADDRESS OutPageFaultGPUAddress = pageFault->PageFaultVA;
    cgpu_error("DRED: PageFault at VA GPUAddress \"0x%llX\"", (long long)OutPageFaultGPUAddress);
    
    const auto* Node = pageFault->pHeadExistingAllocationNode;
    if (Node)
    {
        cgpu_error("DRED: Active objects with VA ranges that match the faulting VA:");
        while (Node)
        {
            // When tracking all allocations then empty named dummy resources (heap & buffer)
            // are created for each texture to extract the GPUBaseAddress so don't write these out
            if (Node->ObjectNameW)
            {
                int32_t alloc_type_index = Node->AllocationType - D3D12_DRED_ALLOCATION_TYPE_COMMAND_QUEUE;
                const TCHAR* AllocTypeName = (alloc_type_index < D3D12_AllocTypesNamesCount) ? D3D12_AllocTypesNames[alloc_type_index] : TEXT("Unknown Alloc");
                if constexpr (std::is_same_v<std::remove_reference_t<T>, D3D12_DRED_PAGE_FAULT_OUTPUT1>)
                {
                    cgpu_error("\tObject: %p, Name: %ls (Type: %ls)", Node->pObject, Node->ObjectNameW, AllocTypeName);
                }
                else
                {
                    cgpu_error("\tName: %ls (Type: %ls)", Node->ObjectNameW, AllocTypeName);
                }
            }
            Node = Node->pNext;
        }
    }

    Node = pageFault->pHeadRecentFreedAllocationNode;
    if (Node)
    {
        cgpu_error("DRED: Recent freed objects with VA ranges that match the faulting VA:");
        while (Node)
        {
            // See comments above
            if (Node->ObjectNameW)
            {
                int32_t alloc_type_index = Node->AllocationType - D3D12_DRED_ALLOCATION_TYPE_COMMAND_QUEUE;
                const TCHAR* AllocTypeName = (alloc_type_index < D3D12_AllocTypesNamesCount) ? D3D12_AllocTypesNames[alloc_type_index] : TEXT("Unknown Alloc");
                if constexpr (std::is_same_v<std::remove_reference_t<T>, D3D12_DRED_PAGE_FAULT_OUTPUT1>)
                {
                    cgpu_error("\tObject: %p, Name: %ls (Type: %ls)", Node->pObject, Node->ObjectNameW, AllocTypeName);
                }
                else
                {
                    cgpu_error("\tName: %ls (Type: %ls)", Node->ObjectNameW, AllocTypeName);
                }
            }

            Node = Node->pNext;
        }
    }
}

void D3D12Util_LogDREDPageFault(const D3D12_DRED_PAGE_FAULT_OUTPUT* pageFault)
{
    D3D12Util_LogDREDPageFaultImpl(pageFault);
}

void D3D12Util_LogDREDPageFault1(const D3D12_DRED_PAGE_FAULT_OUTPUT1* pageFault)
{
    D3D12Util_LogDREDPageFaultImpl(pageFault);
}

inline static gsl::span<D3D12_DRED_BREADCRUMB_CONTEXT> GetDREDBreadcrumbContexts(const D3D12_AUTO_BREADCRUMB_NODE* Node)
{
	return {};
}

inline static gsl::span<D3D12_DRED_BREADCRUMB_CONTEXT> GetDREDBreadcrumbContexts(const D3D12_AUTO_BREADCRUMB_NODE1* Node)
{
	return gsl::span<D3D12_DRED_BREADCRUMB_CONTEXT>(Node->pBreadcrumbContexts, Node->BreadcrumbContextsCount);
}

template<typename T>
void D3D12Util_LogDREDBreadcrumbsImpl(const T* breadcrumbs)
{
    cgpu_error("Gathered auto-breadcrumbs output.");
    if (breadcrumbs->pHeadAutoBreadcrumbNode)
    {
        cgpu_error("DRED: Last tracked GPU operations:");

        eastl::wstring ContextStr;
        eastl::vector_map<int32_t, const wchar_t*> ContextStrings;

        uint32_t TracedCommandLists = 0;
        auto Node = breadcrumbs->pHeadAutoBreadcrumbNode;
        while (Node && Node->pLastBreadcrumbValue)
        {
            int32_t LastCompletedOp = *Node->pLastBreadcrumbValue;

            if (LastCompletedOp != Node->BreadcrumbCount && LastCompletedOp != 0)
            {
                cgpu_error("DRED: Commandlist \"%ls\" on CommandQueue \"%ls\", %d completed of %d", 
                    Node->pCommandListDebugNameW, Node->pCommandQueueDebugNameW, LastCompletedOp, Node->BreadcrumbCount);
                TracedCommandLists++;

                int32_t FirstOp = cgpu_max(LastCompletedOp - 100, 0);
                int32_t LastOp = cgpu_min(LastCompletedOp + 20, int32_t(Node->BreadcrumbCount) - 1);

                ContextStrings.clear();
                for (const D3D12_DRED_BREADCRUMB_CONTEXT& Context : GetDREDBreadcrumbContexts(Node))
                {
                    ContextStrings.emplace_back(Context.BreadcrumbIndex, Context.pContextString);
                }

                for (int32_t Op = FirstOp; Op <= LastOp; ++Op)
                {
                    D3D12_AUTO_BREADCRUMB_OP BreadcrumbOp = Node->pCommandHistory[Op];

                    auto OpContextStr = ContextStrings.find(Op);
                    if (OpContextStr)
                    {
                        ContextStr = TEXT(" [");
                        ContextStr += OpContextStr->second;
                        ContextStr += TEXT("]");
                    }
                    else
                    {
                        ContextStr.clear();
                    }

                    const TCHAR* OpName = (BreadcrumbOp < D3D12_OpNamesCount) ? D3D12_OpNames[BreadcrumbOp] : TEXT("Unknown Op");
                    cgpu_error("\tOp: %d, %ls%ls%ls", Op, OpName, ContextStr.c_str(), (Op + 1 == LastCompletedOp) ? TEXT(" - LAST COMPLETED") : TEXT(""));
                }
            }

            Node = Node->pNext;
        }

        if (TracedCommandLists == 0)
        {
            cgpu_error("DRED: No command list found with active outstanding operations (all finished or not started yet).");
        }
    }
    else
    {
        cgpu_error("DRED: No breadcrumb head found.");
    }
}

void D3D12Util_LogDREDBreadcrumbs(const D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT* breadcrumbs)
{
    D3D12Util_LogDREDBreadcrumbsImpl(breadcrumbs);
}

void D3D12Util_LogDREDBreadcrumbs1(const D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT1* breadcrumbs)
{
    D3D12Util_LogDREDBreadcrumbsImpl(breadcrumbs);
}

void D3D12Util_ReportGPUCrash(ID3D12Device* device)
{
    HRESULT removeHr = -1;
    auto failedCount = 0u;
    const auto retryCount = 4u;

    while (FAILED(removeHr) && (failedCount < retryCount))
    {
        removeHr = device->GetDeviceRemovedReason();
        if (FAILED(removeHr))
        {
            cgpu_error("Device removed, waiting for driver to come back online...");
            Sleep(500); // Wait for a few seconds to allow the driver to come
                        // back online before doing a reset.
            failedCount++;
        }
    }
    if (failedCount >= retryCount)
    {
        cgpu_fatal("Device driver get lost, unable to get removed reason from DRED.");
    }

#ifdef __ID3D12DeviceRemovedExtendedData_FWD_DEFINED__
    if (ID3D12DeviceRemovedExtendedData1* pDread1; SUCCEEDED(device->QueryInterface(IID_ARGS(&pDread1))))
    {
        auto breadcrumbs = make_zeroed<D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT1>();
        if (SUCCEEDED(pDread1->GetAutoBreadcrumbsOutput1(&breadcrumbs)))
        {
            D3D12Util_LogDREDBreadcrumbs1(&breadcrumbs);
        }
        auto pageFault = make_zeroed<D3D12_DRED_PAGE_FAULT_OUTPUT1>();
        if (SUCCEEDED(pDread1->GetPageFaultAllocationOutput1(&pageFault)))
        {
            D3D12Util_LogDREDPageFault1(&pageFault);
        }
        pDread1->Release();
    }
    else if (ID3D12DeviceRemovedExtendedData* pDread; SUCCEEDED(device->QueryInterface(IID_ARGS(&pDread))))
    {
        auto breadcrumbs = make_zeroed<D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT>();
        if (SUCCEEDED(pDread->GetAutoBreadcrumbsOutput(&breadcrumbs)))
        {
            D3D12Util_LogDREDBreadcrumbs(&breadcrumbs);
        }
        auto pageFault = make_zeroed<D3D12_DRED_PAGE_FAULT_OUTPUT>();
        if (SUCCEEDED(pDread->GetPageFaultAllocationOutput(&pageFault)))
        {
            D3D12Util_LogDREDPageFault(&pageFault);
        }
        pDread->Release();
    }
#endif
}