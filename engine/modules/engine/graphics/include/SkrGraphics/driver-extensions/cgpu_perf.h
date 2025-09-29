#pragma once
#include "SkrBase/types.h"
#include "SkrGraphics/api.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CGPUPerfContext* CGPUPerfContextId;
typedef struct CGPUPerfSession* CGPUPerfSessionId;
typedef struct CGPUPerfCommandList* CGPUPerfCommandListId;
typedef struct CGPUPerfSampleDecoder* CGPUPerfSampleDecoderId;

typedef uint32_t CGPUPerfSampleId;
typedef uint32_t CGPUPerfCounterIndex;

typedef enum ECGPUPerfStatus
{
    CGPU_PERF_STATUS_OK = 0,
    CGPU_PERF_STATUS_RESULT_NOT_READY = 1,

    // Errors
    CGPU_PERF_STATUS_ERROR_NULL_POINTER = -1,
    CGPU_PERF_STATUS_ERROR_CONTEXT_NOT_OPEN = -2,
    CGPU_PERF_STATUS_ERROR_CONTEXT_ALREADY_OPEN = -3,
    CGPU_PERF_STATUS_ERROR_INDEX_OUT_OF_RANGE = -4,
    CGPU_PERF_STATUS_ERROR_COUNTER_NOT_FOUND = -5,
    CGPU_PERF_STATUS_ERROR_ALREADY_ENABLED = -6,
    CGPU_PERF_STATUS_ERROR_NO_COUNTERS_ENABLED = -7,
    CGPU_PERF_STATUS_ERROR_NOT_ENABLED = -8,
    CGPU_PERF_STATUS_ERROR_SESSION_NOT_FOUND = -9,
    CGPU_PERF_STATUS_ERROR_SESSION_ALREADY_STARTED = -10,
    CGPU_PERF_STATUS_ERROR_SESSION_NOT_STARTED = -11,
    CGPU_PERF_STATUS_ERROR_INCOMPATIBLE_SAMPLE_TYPES = -12,
    CGPU_PERF_STATUS_ERROR_HARDWARE_NOT_SUPPORTED = -13,
    CGPU_PERF_STATUS_ERROR_NOT_INITIALIZED = -14,
    CGPU_PERF_STATUS_ERROR_FAILED = -15,
} ECGPUPerfStatus;

typedef enum ECGPUPerfInitializeFlags
{
    CGPU_PERF_INITIALIZE_DEFAULT_BIT = 0,
    // Enable simultaneous queues (NVIDIA GPU Periodic Sampler, AMD SPM)
    CGPU_PERF_INITIALIZE_SIMULTANEOUS_QUEUES_BIT = 0x01,
    // Enable instruction trace (AMD SQTT, NVIDIA SASS)
    CGPU_PERF_INITIALIZE_INSTRUCTION_TRACE_BIT = 0x02,
} ECGPUPerfInitializeFlags;
typedef uint32_t CGPUPerfInitializeFlags;

typedef enum ECGPUPerfOpenContextFlags
{
    CGPU_PERF_OPEN_CONTEXT_DEFAULT_BIT = 0,
    // Counter visibility control
    CGPU_PERF_OPEN_CONTEXT_HIDE_DERIVED_COUNTERS_BIT = 0x01,
    CGPU_PERF_OPEN_CONTEXT_ENABLE_HARDWARE_COUNTERS_BIT = 0x80,

    // Clock mode control (AMD specific, ignored on NVIDIA)
    CGPU_PERF_OPEN_CONTEXT_CLOCK_MODE_NONE_BIT = 0x08,
    CGPU_PERF_OPEN_CONTEXT_CLOCK_MODE_PEAK_BIT = 0x10,
    CGPU_PERF_OPEN_CONTEXT_CLOCK_MODE_MIN_MEMORY_BIT = 0x20,
    CGPU_PERF_OPEN_CONTEXT_CLOCK_MODE_MIN_ENGINE_BIT = 0x40,
} ECGPUPerfOpenContextFlags;
typedef uint32_t CGPUPerfOpenContextFlags;

typedef enum ECGPUPerfSessionSampleType
{
    // Discrete counter sampling (per draw/dispatch)
    CGPU_PERF_SESSION_SAMPLE_TYPE_DISCRETE_COUNTER = 0,
    // Streaming counter sampling (continuous)
    CGPU_PERF_SESSION_SAMPLE_TYPE_STREAMING_COUNTER = 1,
    // Trace (SQTT/SASS instruction trace)
    CGPU_PERF_SESSION_SAMPLE_TYPE_TRACE = 2,
} ECGPUPerfSessionSampleType;

typedef enum ECGPUPerfDataType
{
    CGPU_PERF_DATA_TYPE_FLOAT64,
    CGPU_PERF_DATA_TYPE_UINT64,
} ECGPUPerfDataType;

typedef enum ECGPUPerfUsageType
{
    CGPU_PERF_USAGE_TYPE_RATIO,
    CGPU_PERF_USAGE_TYPE_PERCENTAGE,
    CGPU_PERF_USAGE_TYPE_CYCLES,
    CGPU_PERF_USAGE_TYPE_MILLISECONDS,
    CGPU_PERF_USAGE_TYPE_BYTES,
    CGPU_PERF_USAGE_TYPE_ITEMS,
    CGPU_PERF_USAGE_TYPE_KILOBYTES,
    CGPU_PERF_USAGE_TYPE_NANOSECONDS,
} ECGPUPerfUsageType;

typedef enum ECGPUPerfCommandListType
{
    CGPU_PERF_COMMAND_LIST_NONE = 0,
    CGPU_PERF_COMMAND_LIST_PRIMARY = 1,
    CGPU_PERF_COMMAND_LIST_SECONDARY = 2,
} ECGPUPerfCommandListType;

typedef struct CGPUPerfCounterDescriptor
{
    skr::GUID guid;
    CGPUPerfCounterIndex index;
    const char8_t* name;
    const char8_t* group;
    const char8_t* description;
    ECGPUPerfDataType data_type;
    ECGPUPerfUsageType usage_type;

    // Vendor-specific extension data
    union
    {
        struct
        {
            uint32_t hardware_unit; // SM, L2, VRAM, etc.
            uint32_t domain;        // Raw counter domain
        } nvidia;
        struct
        {
            uint32_t hw_generation; // GFX10, GFX11, etc.
            skr::GUID uuid;       // Counter UUID
        } amd;
    } vendor_ext;
} CGPUPerfCounterDescriptor;

// Initialize the performance library
CGPU_API ECGPUPerfStatus cgpu_perf_initialize(CGPUPerfInitializeFlags flags);
CGPU_API ECGPUPerfStatus cgpu_perf_destroy(void);

// Context APIs
CGPU_API ECGPUPerfStatus cgpu_perf_open_context(CGPUDeviceId device, CGPUPerfOpenContextFlags flags, CGPUPerfContextId* context);
CGPU_API ECGPUPerfStatus cgpu_perf_close_context(CGPUPerfContextId context);
CGPU_API ECGPUPerfStatus cgpu_perf_get_supported_sample_types(CGPUPerfContextId context, ECGPUPerfSessionSampleType* sample_types);
CGPU_API ECGPUPerfStatus cgpu_perf_get_num_counters(CGPUPerfContextId context, uint32_t* count);
CGPU_API ECGPUPerfStatus cgpu_perf_get_counter_descriptor(CGPUPerfContextId context, CGPUPerfCounterIndex index, CGPUPerfCounterDescriptor* descriptor);
CGPU_API ECGPUPerfStatus cgpu_perf_get_counter_index(CGPUPerfContextId context, const char8_t* counter_name, CGPUPerfCounterIndex* index);

// Enable/Disable counters
CGPU_API ECGPUPerfStatus cgpu_perf_enable_counter(CGPUPerfSessionId session, CGPUPerfCounterIndex counter_index);
CGPU_API ECGPUPerfStatus cgpu_perf_disable_counter(CGPUPerfSessionId session, CGPUPerfCounterIndex counter_index);
CGPU_API ECGPUPerfStatus cgpu_perf_enable_counter_by_name(CGPUPerfSessionId session, const char8_t* counter_name);
CGPU_API ECGPUPerfStatus cgpu_perf_disable_counter_by_name(CGPUPerfSessionId session, const char8_t* counter_name);
CGPU_API ECGPUPerfStatus cgpu_perf_enable_all_counters(CGPUPerfSessionId session);
CGPU_API ECGPUPerfStatus cgpu_perf_disable_all_counters(CGPUPerfSessionId session);
CGPU_API ECGPUPerfStatus cgpu_perf_get_num_enabled_counters(CGPUPerfSessionId session, uint32_t* count);
CGPU_API ECGPUPerfStatus cgpu_perf_is_counter_enabled(CGPUPerfSessionId session, CGPUPerfCounterIndex counter_index);

// Session APIs
CGPU_API ECGPUPerfStatus cgpu_perf_create_session(CGPUPerfContextId context, CGPUQueueId queue, ECGPUPerfSessionSampleType sample_type, const char8_t* name, CGPUPerfSessionId* session);
CGPU_API ECGPUPerfStatus cgpu_perf_reset_session(CGPUPerfSessionId session);
CGPU_API ECGPUPerfStatus cgpu_perf_begin_session(CGPUPerfSessionId session);
CGPU_API ECGPUPerfStatus cgpu_perf_end_session(CGPUPerfSessionId session);
CGPU_API ECGPUPerfStatus cgpu_perf_is_session_ready(CGPUPerfSessionId session);
CGPU_API ECGPUPerfStatus cgpu_perf_free_session(CGPUPerfSessionId session);

// CommandList APIs
CGPU_API ECGPUPerfStatus cgpu_perf_begin_command_list(
    CGPUPerfSessionId session,
    uint32_t pass_index,
    CGPUCommandBufferId cmd, // API command list or NULL for immediate mode
    ECGPUPerfCommandListType type,
    CGPUPerfCommandListId* command_list
);
CGPU_API ECGPUPerfStatus cgpu_perf_end_command_list(CGPUPerfCommandListId command_list);

// Sample APIs
CGPU_API ECGPUPerfStatus cgpu_perf_begin_sample(CGPUPerfSampleId sample_id, CGPUPerfCommandListId command_list);
CGPU_API ECGPUPerfStatus cgpu_perf_end_sample(CGPUPerfCommandListId command_list);
CGPU_API ECGPUPerfStatus cgpu_perf_get_sample_count(CGPUPerfSessionId decoder, uint32_t* count);

CGPU_API ECGPUPerfStatus cgpu_perf_decode_samples(CGPUPerfSessionId session);
CGPU_API ECGPUPerfStatus cgpu_perf_get_sample_result_as_float64(CGPUPerfSampleId sample_id, CGPUPerfCounterIndex counter_index, double* result);
CGPU_API ECGPUPerfStatus cgpu_perf_get_sample_result_as_uint64(CGPUPerfSampleId sample_id, CGPUPerfCounterIndex counter_index, uint64_t* result);

// Multi-Pass APIs (for CGPU_PERF_SESSION_SAMPLE_TYPE_DISCRETE_COUNTER counters)
CGPU_API ECGPUPerfStatus cgpu_perf_get_pass_count(CGPUPerfSessionId session, uint32_t* num_passes);
CGPU_API ECGPUPerfStatus cgpu_perf_is_pass_complete(CGPUPerfSessionId session, uint32_t pass_index);

#ifdef __cplusplus
}
#endif
