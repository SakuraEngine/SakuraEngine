#pragma once
#define RUNTIME_DLL
#include "platform/configure.h"
#include "cgpu_config.h"
#define CGPU_ARRAY_LEN(array) ((sizeof(array) / sizeof(array[0])))

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

struct CGpuInstanceDescriptor;
struct CGpuDeviceDescriptor;
struct CGpuCommandEncoderDescriptor;
struct CGpuSwapChainDescriptor;

typedef uint32_t CGpuQueueIndex;
typedef const struct CGpuSurface_Dummy* CGpuSurfaceId;
typedef const int CGpuVersion;
typedef const struct CGpuInstance* CGpuInstanceId;
typedef const struct CGpuAdapter* CGpuAdapterId;
typedef const struct CGpuDevice* CGpuDeviceId;
typedef const struct CGpuQueue* CGpuQueueId;
typedef const struct CGpuCommandEncoder* CGpuCommandEncoderId;
typedef const struct CGpuCommandBuffer* CGpuCommandBufferId;
typedef const struct CGpuSwapChain* CGpuSwapChainId;

typedef enum ECGPUBackEnd
{
	ECGPUBackEnd_VULKAN = 0,
	ECGPUBackEnd_D3D12 = 1,
	ECGPUBackEnd_XBOX_D3D12 = 2,
	ECGPUBackEnd_AGC = 3,
	ECGPUBackEnd_METAL = 4,
	ECGPUBackEnd_COUNT
} ECGPUBackEnd;

typedef enum ECGpuQueueType {
	ECGpuQueueType_Graphics = 0,
	ECGpuQueueType_Compute = 1,
	ECGpuQueueType_Transfer = 2,
	ECGpuQueueType_Count
} ECGpuQueueType;

typedef struct CGpuAdapterDetail {
	const uint32_t deviceId;
	const uint32_t vendorId;
	const char* name;
} CGpuAdapterDetail;

// Device APIs
RUNTIME_API CGpuInstanceId cgpu_create_instance(const struct CGpuInstanceDescriptor* desc);
typedef CGpuInstanceId (*CGPUProcCreateInstance)(const struct CGpuInstanceDescriptor * descriptor);
RUNTIME_API void cgpu_free_instance(CGpuInstanceId instance);
typedef void (*CGPUProcFreeInstance)(CGpuInstanceId instance);

RUNTIME_API void cgpu_enum_adapters(CGpuInstanceId instance, CGpuAdapterId* const adapters, size_t* adapters_num);
typedef void (*CGPUProcEnumAdapters)(CGpuInstanceId instance, CGpuAdapterId* const adapters, size_t* adapters_num);

RUNTIME_API CGpuAdapterDetail cgpu_query_adapter_detail(const CGpuAdapterId adapter);
typedef CGpuAdapterDetail (*CGPUProcQueryAdapterDetail)(const CGpuAdapterId instance);
RUNTIME_API uint32_t cgpu_query_queue_count(const CGpuAdapterId adapter, const ECGpuQueueType type);
typedef uint32_t (*CGPUProcQueryQueueCount)(const CGpuAdapterId adapter, const ECGpuQueueType type);

RUNTIME_API CGpuDeviceId cgpu_create_device(CGpuAdapterId adapter, const struct CGpuDeviceDescriptor* desc);
typedef CGpuDeviceId (*CGPUProcCreateDevice)(CGpuAdapterId adapter, const struct CGpuDeviceDescriptor* desc);
RUNTIME_API void cgpu_free_device(CGpuDeviceId device);
typedef void (*CGPUProcFreeDevice)(CGpuDeviceId device);

RUNTIME_API CGpuQueueId cgpu_get_queue(CGpuDeviceId device, ECGpuQueueType type, uint32_t index);
typedef CGpuQueueId (*CGPUProcGetQueue)(CGpuDeviceId device, ECGpuQueueType type, uint32_t index);
RUNTIME_API void cgpu_free_queue(CGpuQueueId queue);
typedef void (*CGPUProcFreeQueue)(CGpuQueueId queue);

RUNTIME_API CGpuCommandEncoderId cgpu_create_command_encoder(CGpuQueueId queue, const struct CGpuCommandEncoderDescriptor* desc);
typedef CGpuCommandEncoderId (*CGPUProcCreateCommandEncoder)(CGpuQueueId queue, const struct  CGpuCommandEncoderDescriptor* desc);
RUNTIME_API void cgpu_free_command_encoder(CGpuCommandEncoderId encoder);
typedef void (*CGPUProcFreeCommandEncoder)(CGpuCommandEncoderId encoder);

RUNTIME_API CGpuSwapChainId cgpu_create_swapchain(CGpuDeviceId device, const struct CGpuSwapChainDescriptor* desc);
typedef CGpuSwapChainId (*CGPUProcCreateSwapChain)(CGpuDeviceId device, const struct CGpuSwapChainDescriptor* desc);
RUNTIME_API void cgpu_free_swapchain(CGpuSwapChainId swapchain);
typedef void (*CGPUProcFreeSwapChain)(CGpuSwapChainId swapchain);

// CMDs 
RUNTIME_API void cgpu_cmd_set_viewport(CGpuCommandBufferId cmd, float x, float y, float width, float height,
    float min_depth, float max_depth);
typedef void (*CGPUProcCmdSetViewport)(CGpuCommandBufferId cmd, float x, float y, float width, float height,
    float min_depth, float max_depth);

RUNTIME_API void cgpu_cmd_set_scissor(CGpuCommandBufferId cmd, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
typedef void (*CGPUProcCmdSetScissor)(CGpuCommandBufferId cmd, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

typedef struct CGpuBuffer {const CGpuDeviceId device;} CGpuBuffer;
typedef CGpuBuffer* CGpuBufferId;


typedef struct CGpuProcTable {
    const CGPUProcCreateInstance     create_instance;
    const CGPUProcFreeInstance       free_instance;

    const CGPUProcEnumAdapters       enum_adapters;
    const CGPUProcQueryAdapterDetail query_adapter_detail;
    const CGPUProcQueryQueueCount    query_queue_count;

    const CGPUProcCreateDevice       create_device;
    const CGPUProcFreeDevice         free_device;
    
    const CGPUProcGetQueue           get_queue;
    const CGPUProcFreeQueue          free_queue;

    const CGPUProcCreateCommandEncoder  create_command_encoder;
    const CGPUProcFreeCommandEncoder    free_command_encoder;

    const CGPUProcCreateSwapChain    create_swapchain;
    const CGPUProcFreeSwapChain      free_swapchain;

    const CGPUProcCmdSetViewport     cmd_set_viewport;
    const CGPUProcCmdSetScissor      cmd_set_scissor;
} CGpuProcTable;


// surfaces
RUNTIME_API void cgpu_free_surface(CGpuDeviceId device, CGpuSurfaceId surface);
typedef void (*CGPUSurfaceProc_Free)(CGpuDeviceId device, CGpuSurfaceId surface);

#if defined(_WIN32) || defined(_WIN64)
typedef struct HWND__* HWND;
RUNTIME_API CGpuSurfaceId cgpu_surface_from_hwnd(CGpuDeviceId device, HWND window);
typedef CGpuSurfaceId (*CGPUSurfaceProc_CreateFromHWND)(CGpuDeviceId device, HWND window);
#endif
#ifdef __APPLE__
RUNTIME_API CGpuSurfaceId cgpu_surface_from_ui_view(CGpuDeviceId device, UIView* window);
typedef CGpuSurfaceId (*CGPUSurfaceProc_CreateFromUIView)(CGpuDeviceId device, UIView* window);

RUNTIME_API CGpuSurfaceId cgpu_surface_from_ns_view(CGpuDeviceId device, NSView* window);
typedef CGpuSurfaceId (*CGPUSurfaceProc_CreateFromNSView)(CGpuDeviceId device, NSView* window);
#endif
typedef struct CGpuSurfacesProcTable {
#if defined(_WIN32) || defined(_WIN64)
    const CGPUSurfaceProc_CreateFromHWND from_hwnd;
#endif
#ifdef __APPLE__
    const CGPUSurfaceProc_CreateFromUIView from_ui_view;
    const CGPUSurfaceProc_CreateFromNSView from_ns_view;
#endif
    const CGPUSurfaceProc_Free free_surface;
} CGpuSurfacesProcTable;

typedef struct CGpuInstance {
    const CGpuProcTable* proc_table;
    const CGpuSurfacesProcTable* surfaces_table;
} CGpuInstance;


// enums
typedef enum ECGpuVertexFormat {
	VF_UCHAR2,
	VF_UCHAR4,
	VF_CHAR2,
	VF_CHAR4,
	VF_UCHAR2_NORM,
	VF_UCHAR4_NORM,
	VF_CHAR2_NORM,
	VF_CHAR4_NORM,
	VF_USHORT2,
	VF_USHORT4,
	VF_SHORT2,
	VF_SHORT4,
	VF_USHORT2_NORM,
	VF_USHORT4_NORM,
	VF_SHORT2_NORM,
	VF_SHORT4_NORM,
	VF_HALF2,
	VF_HALF4,
	VF_FLOAT,
	VF_FLOAT2,
	VF_FLOAT3,
	VF_FLOAT4,
	VF_UINT,
	VF_UINT2,
	VF_UINT3,
	VF_UINT4,
	VF_INT,
	VF_INT2,
	VF_INT3,
	VF_INT4,
	VF_COUNT,

	VF_R8G8_UNORM = VF_UCHAR2_NORM,
	VF_R8G8_NORM = VF_CHAR2_NORM,
	VF_R8G8B8A8_UNORM = VF_UCHAR4_NORM,
	VF_R8G8B8A8_NORM = VF_CHAR4_NORM,
	VF_R16G16_UNORM = VF_USHORT2_NORM,
	VF_R16G16_NORM = VF_SHORT2_NORM,
	VF_R16G16B16A16_UNORM = VF_USHORT4_NORM,
	VF_R16G16B16A16_NORM = VF_SHORT4_NORM,
	VF_R32G32B32A32_UINT = VF_UINT4,
	VF_R32G32B32A32_INT = VF_INT4,
	VF_R32G32B32A32_SINT = VF_INT4,
	VF_R32G32_UINT = VF_UINT2,
	VF_R32G32_INT = VF_INT2,
	VF_R32G32_SINT = VF_INT2,
} ECGpuVertexFormat;

typedef enum ECGpuPixelFormat {
	PF_R32G32B32A32_UINT = 0,
	PF_R32G32B32A32_SINT = 1,
	PF_R32G32B32A32_FLOAT = 2,
	PF_R32G32B32_UINT = 3,
	PF_R32G32B32_SINT = 4,
	PF_R32G32B32_FLOAT = 5,
	PF_R16G16B16A16_UNORM = 6,
	PF_R16G16B16A16_SNORM = 7,
	PF_R16G16B16A16_UINT = 8,
	PF_R16G16B16A16_SINT = 9,
	PF_R16G16B16A16_FLOAT = 10,
	PF_R32G32_UINT = 11,
	PF_R32G32_SINT = 12,
	PF_R32G32_FLOAT = 13,
	PF_R10G10B10A2_UNORM = 14,
	PF_R10G10B10A2_UINT = 15,
	PF_R9G9B9E5_UFLOAT = 16,
	PF_R8G8B8A8_UNORM = 17,
	PF_R8G8B8A8_SNORM = 18,
	PF_R8G8B8A8_UINT = 19,
	PF_R8G8B8A8_SINT = 20,
	PF_R8G8B8A8_UNORM_SRGB = 21,
	PF_B8G8R8A8_UNORM = 22,
	PF_B8G8R8A8_UNORM_SRGB = 23,
	PF_R11G11B10_FLOAT = 24,
	PF_R16G16_UNORM = 25,
	PF_R16G16_SNORM = 26,
	PF_R16G16_UINT = 27,
	PF_R16G16_SINT = 28,
	PF_R16G16_FLOAT = 29,
	PF_R32_UINT = 30,
	PF_R32_SINT = 31,
	PF_R32_FLOAT = 32,
	PF_B5G5R5A1_UNORM = 33,
	PF_B5G6R5_UNORM = 34,
	PF_R8G8_UNORM = 35,
	PF_R8G8_SNORM = 36,
	PF_R8G8_UINT = 37,
	PF_R8G8_SINT = 38,
	PF_R16_UNORM = 39,
	PF_R16_SNORM = 40,
	PF_R16_UINT = 41,
	PF_R16_SINT = 42,
	PF_R16_FLOAT = 43,
	PF_R8_UNORM = 44,
	PF_R8_SNORM = 45,
	PF_R8_UINT = 46,
	PF_R8_SINT = 47,
	PF_D24_UNORM_S8_UINT = 49,
	PF_D32_FLOAT = 50,
	PF_D16_UNORM = 51,
	PF_BC1_UNORM = 52,
	PF_BC1_UNORM_SRGB = 53,
	PF_BC2_UNORM = 54,
	PF_BC2_UNORM_SRGB = 55,
	PF_BC3_UNORM = 56,
	PF_BC3_UNORM_SRGB = 57,
	PF_BC4_UNORM = 58,
	PF_BC4_SNORM = 59,
	PF_BC5_UNORM = 60,
	PF_BC5_SNORM = 61,
	PF_BC6H_UF16 = 62,
	PF_BC6H_SF16 = 63,
	PF_BC7_UNORM = 64,
	PF_BC7_UNORM_SRGB = 65,
	PF_Count = PF_BC7_UNORM_SRGB + 1
} ECGpuPixelFormat;




typedef struct CGpuAdapter {
	const struct CGpuInstance* instance;
} CGpuAdapter;

typedef struct CGpuDevice {
	const CGpuAdapterId adapter;
#ifdef __cplusplus
	CGpuDevice() :adapter(CGPU_NULLPTR) {}
#endif
} CGpuDevice;

typedef struct CGpuQueue {
	CGpuDeviceId   device;
	ECGpuQueueType type;
	CGpuQueueIndex index;
} CGpuQueue;

typedef struct CGpuCommandEncoder {
	CGpuQueueId queue;
} CGpuCommandEncoder;

typedef struct CGpuCommandBuffer {
	CGpuCommandEncoderId pool;
} CGpuCommandBuffer;

typedef struct CGpuSwapChain {
	CGpuDeviceId device;
} CGpuSwapChain;

typedef struct CGpuInstanceDescriptor {
	ECGPUBackEnd backend;
	bool         enableDebugLayer;
	bool         enableGpuBasedValidation;
} CGpuInstanceDescriptor;

typedef struct CGpuQueueGroupDescriptor {
	ECGpuQueueType queueType;
	uint32_t queueCount;
} CGpuQueueGroupDescriptor;

typedef struct CGpuDeviceDescriptor {
	CGpuQueueGroupDescriptor* queueGroups;
	uint32_t                  queueGroupCount;
} CGpuDeviceDescriptor;

typedef struct CGpuCommandEncoderDescriptor {
	uint32_t ___nothing_and_useless__;
} CGpuCommandEncoderDescriptor;

typedef struct CGpuSwapChainDescriptor {
	/// Present Queues
	CGpuQueueId*  presentQueues;
	/// Present Queues Count
	uint32_t      presentQueuesCount;
	/// Surface to Create SwapChain on
	CGpuSurfaceId surface;
	/// Number of backbuffers in this swapchain
	uint32_t      imageCount;
	/// Width of the swapchain
	uint32_t      width;
	/// Height of the swapchain
	uint32_t      height;
	/// Set whether swap chain will be presented using vsync
	bool          enableVsync;
	/// We can toggle to using FLIP model if app desires
	bool          useFlipSwapEffect;
	/// Clear Value.
	float         clearValue[4];
	/// format
	ECGpuPixelFormat format;
} CGpuSwapChainDescriptor;


#ifdef __cplusplus
} // end extern "C"
#endif
