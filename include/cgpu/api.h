#pragma once
#define RUNTIME_DLL
#include "platform/configure.h"
#include "cgpu_config.h"
#include "format.h"
#define CGPU_ARRAY_LEN(array) ((sizeof(array) / sizeof(array[0])))

#ifdef __cplusplus
extern "C" {
#endif

struct CGpuInstanceDescriptor;
struct CGpuDeviceDescriptor;
struct CGpuCommandEncoderDescriptor;
struct CGpuShaderLibraryDescriptor;
struct CGpuPipelineShaderDescriptor;
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
typedef const struct CGpuShaderLibrary* CGpuShaderLibraryId;
typedef const struct CGpuPipelineShader* CGpuPipelineShaderId;

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
	uint32_t deviceId;
	uint32_t vendorId;
	const char* name;
} CGpuAdapterDetail;

typedef struct CGpuInstanceFeatures {
	bool specialization_constant;

} CGpuInstanceFeatures;

typedef struct CGpuConstantSpecialization {
	uint32_t constantID;
	union
	{
		uint32_t u;
		int32_t i;
		float f;
	};
} CGpuConstantSpecialization;

// Instance APIs
RUNTIME_API CGpuInstanceId cgpu_create_instance(const struct CGpuInstanceDescriptor* desc);
typedef CGpuInstanceId (*CGPUProcCreateInstance)(const struct CGpuInstanceDescriptor * descriptor);
RUNTIME_API void cgpu_query_instance_features(CGpuInstanceId instance, struct CGpuInstanceFeatures* features);
typedef void (*CGPUProcQueryInstanceFeatures)(CGpuInstanceId instance, struct CGpuInstanceFeatures* features);
RUNTIME_API void cgpu_free_instance(CGpuInstanceId instance);
typedef void (*CGPUProcFreeInstance)(CGpuInstanceId instance);

// Adapter APIs
RUNTIME_API void cgpu_enum_adapters(CGpuInstanceId instance, CGpuAdapterId* const adapters, uint32_t* adapters_num);
typedef void (*CGPUProcEnumAdapters)(CGpuInstanceId instance, CGpuAdapterId* const adapters, uint32_t* adapters_num);

RUNTIME_API void cgpu_query_adapter_detail(const CGpuAdapterId adapter, struct CGpuAdapterDetail* detail);
typedef void (*CGPUProcQueryAdapterDetail)(const CGpuAdapterId instance, struct CGpuAdapterDetail* detail);
RUNTIME_API uint32_t cgpu_query_queue_count(const CGpuAdapterId adapter, const ECGpuQueueType type);
typedef uint32_t (*CGPUProcQueryQueueCount)(const CGpuAdapterId adapter, const ECGpuQueueType type);

// Device APIs
RUNTIME_API CGpuDeviceId cgpu_create_device(CGpuAdapterId adapter, const struct CGpuDeviceDescriptor* desc);
typedef CGpuDeviceId (*CGPUProcCreateDevice)(CGpuAdapterId adapter, const struct CGpuDeviceDescriptor* desc);
RUNTIME_API void cgpu_free_device(CGpuDeviceId device);
typedef void (*CGPUProcFreeDevice)(CGpuDeviceId device);

// Queue APIs
RUNTIME_API CGpuQueueId cgpu_get_queue(CGpuDeviceId device, ECGpuQueueType type, uint32_t index);
typedef CGpuQueueId (*CGPUProcGetQueue)(CGpuDeviceId device, ECGpuQueueType type, uint32_t index);
RUNTIME_API void cgpu_free_queue(CGpuQueueId queue);
typedef void (*CGPUProcFreeQueue)(CGpuQueueId queue);

// Command APIs
RUNTIME_API CGpuCommandEncoderId cgpu_create_command_encoder(CGpuQueueId queue, const struct CGpuCommandEncoderDescriptor* desc);
typedef CGpuCommandEncoderId (*CGPUProcCreateCommandEncoder)(CGpuQueueId queue, const struct CGpuCommandEncoderDescriptor* desc);
RUNTIME_API void cgpu_free_command_encoder(CGpuCommandEncoderId encoder);
typedef void (*CGPUProcFreeCommandEncoder)(CGpuCommandEncoderId encoder);

// Shader APIs
RUNTIME_API CGpuShaderLibraryId cgpu_create_shader_library(CGpuDeviceId device, const struct CGpuShaderLibraryDescriptor* desc);
typedef CGpuShaderLibraryId (*CGPUProcCreateShaderLibrary)(CGpuDeviceId device, const struct CGpuShaderLibraryDescriptor* desc);
RUNTIME_API void cgpu_free_shader_module(CGpuShaderLibraryId shader_module);
typedef void (*CGPUProcFreeShaderLibrary)(CGpuShaderLibraryId shader_module);

// Swapchain APIs
RUNTIME_API CGpuSwapChainId cgpu_create_swapchain(CGpuDeviceId device, const struct CGpuSwapChainDescriptor* desc);
typedef CGpuSwapChainId (*CGPUProcCreateSwapChain)(CGpuDeviceId device, const struct CGpuSwapChainDescriptor* desc);
RUNTIME_API void cgpu_free_swapchain(CGpuSwapChainId swapchain);
typedef void (*CGPUProcFreeSwapChain)(CGpuSwapChainId swapchain);

// CMDs 
RUNTIME_API void cgpu_cmd_set_viewport(CGpuCommandBufferId cmd, float x, float y, float width, float height, float min_depth, float max_depth);
typedef void (*CGPUProcCmdSetViewport)(CGpuCommandBufferId cmd, float x, float y, float width, float height, float min_depth, float max_depth);

RUNTIME_API void cgpu_cmd_set_scissor(CGpuCommandBufferId cmd, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
typedef void (*CGPUProcCmdSetScissor)(CGpuCommandBufferId cmd, uint32_t x, uint32_t y, uint32_t width, uint32_t height);


// Types
typedef struct CGpuBuffer {const CGpuDeviceId device;} CGpuBuffer;
typedef CGpuBuffer* CGpuBufferId;


typedef struct CGpuProcTable {
    const CGPUProcCreateInstance        create_instance;
	const CGPUProcQueryInstanceFeatures query_instance_features;
    const CGPUProcFreeInstance          free_instance;

    const CGPUProcEnumAdapters       enum_adapters;
    const CGPUProcQueryAdapterDetail query_adapter_detail;
    const CGPUProcQueryQueueCount    query_queue_count;

    const CGPUProcCreateDevice       create_device;
    const CGPUProcFreeDevice         free_device;
    
    const CGPUProcGetQueue           get_queue;
    const CGPUProcFreeQueue          free_queue;

    const CGPUProcCreateCommandEncoder  create_command_encoder;
    const CGPUProcFreeCommandEncoder    free_command_encoder;

	const CGPUProcCreateShaderLibrary   create_shader_library;
	const CGPUProcFreeShaderLibrary     free_shader_module;
	
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
//RUNTIME_API CGpuSurfaceId cgpu_surface_from_ui_view(CGpuDeviceId device, UIView* window);
//typedef CGpuSurfaceId (*CGPUSurfaceProc_CreateFromUIView)(CGpuDeviceId device, UIView* window);
typedef struct NSView NSView;
RUNTIME_API CGpuSurfaceId cgpu_surface_from_ns_view(CGpuDeviceId device, NSView* window);
typedef CGpuSurfaceId (*CGPUSurfaceProc_CreateFromNSView)(CGpuDeviceId device, NSView* window);
#endif
typedef struct CGpuSurfacesProcTable {
#if defined(_WIN32) || defined(_WIN64)
    const CGPUSurfaceProc_CreateFromHWND from_hwnd;
#endif
#ifdef __APPLE__
    //const CGPUSurfaceProc_CreateFromUIView from_ui_view;
    const CGPUSurfaceProc_CreateFromNSView from_ns_view;
#endif
    const CGPUSurfaceProc_Free free_surface;
} CGpuSurfacesProcTable;

//Objects
typedef struct CGpuInstance {
    const CGpuProcTable* proc_table;
    const CGpuSurfacesProcTable* surfaces_table;
} CGpuInstance;

typedef struct CGpuAdapter {
	const struct CGpuInstance* instance;
    const CGpuProcTable* proc_table_cache;
} CGpuAdapter;

typedef struct CGpuDevice {
	const CGpuAdapterId adapter;
    const CGpuProcTable* proc_table_cache;
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

typedef struct CGpuShaderLibrary {
	CGpuDeviceId device;
	const char8_t* name;
} CGpuShaderLibrary;

typedef struct CGpuSwapChain {
	CGpuDeviceId device;
} CGpuSwapChain;

// Descriptors
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
	bool disable_pipeline_cache;
	CGpuQueueGroupDescriptor* queueGroups;
	uint32_t                  queueGroupCount;
} CGpuDeviceDescriptor;

typedef struct CGpuCommandEncoderDescriptor {
	uint32_t ___nothing_and_useless__;
} CGpuCommandEncoderDescriptor;

typedef struct CGpuShaderLibraryDescriptor {
	const char8_t*     name;
	const uint32_t*    code;
	size_t             code_size;
	ECGpuShaderStage   stage;
} CGpuShaderLibraryDescriptor;

typedef struct CGpuPipelineShaderDescriptor {
	CGpuShaderLibraryId library;
	const char8_t*      entry;
	// ++ constant_specialization
	const CGpuConstantSpecialization* constants;
	size_t num_constants;
	// -- constant_specialization
} CGpuPipelineShaderDescriptor;

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
