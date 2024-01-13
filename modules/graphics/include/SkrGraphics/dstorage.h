#pragma once
#include "SkrBase/config.h"
#include "SkrGraphics/config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SKR_DSTORAGE_MAX_QUEUE_CAPACITY 0x2000

typedef enum ESkrDStorageAvailability
{
    SKR_DSTORAGE_AVAILABILITY_NONE = 0,
    SKR_DSTORAGE_AVAILABILITY_HARDWARE = 1,
    SKR_DSTORAGE_AVAILABILITY_SOFTWARE = 2,
    SKR_DSTORAGE_AVAILABILITY_MAX_ENUM_BIT = 0x7FFFFFFF
} ESkrDStorageAvailability;

typedef enum ESkrDStorageSource {
    SKR_DSTORAGE_SOURCE_FILE = 0,
    SKR_DSTORAGE_SOURCE_MEMORY = 1,
    SKR_DSTORAGE_SOURCE_MAX_ENUM_BIT = 0x7FFFFFFF
} ESkrDStorageSource;

typedef enum ESkrDStoragePriority {
    SKR_DSTORAGE_PRIORITY_LOW = -1,
    SKR_DSTORAGE_PRIORITY_NORMAL = 0,
    SKR_DSTORAGE_PRIORITY_HIGH = 1,
    SKR_DSTORAGE_PRIORITY_REALTIME = 2,

    SKR_DSTORAGE_PRIORITY_COUNT = 4,
    SKR_DSTORAGE_PRIORITY_MAX_ENUM_BIT = 0x7FFFFFFF
} ESkrDStoragePriority;

typedef enum ESkrDStorageCompression {
    SKR_DSTORAGE_COMPRESSION_NONE = 0,
    SKR_DSTORAGE_COMPRESSION_CUSTOM = 0x80,
    SKR_DSTORAGE_COMPRESSION_MAX_ENUM_BIT = 0xFF
} ESkrDStorageCompression;

typedef uint8_t SkrDStorageCompression;

typedef struct SkrDStorageMemoryRange
{
    const uint8_t* bytes;
    uint64_t bytes_size;
} SkrDStorageMemoryRange;

typedef struct SkrDStorageConfig
{
    bool no_bypass SKR_IF_CPP(= false);
    bool enable_cache SKR_IF_CPP(= false);
} SkrDStorageConfig;

typedef struct SkrDStorageInstance
{
    bool failed SKR_IF_CPP(= false);
} SkrDStorageInstance;
typedef struct SkrDStorageInstance* SkrDStorageInstanceId;

typedef struct SkrDStorageQueue {
    const struct CGPUDevice* device SKR_IF_CPP(= nullptr);
} SkrDStorageQueue;
typedef const struct SkrDStorageQueue* SkrDStorageQueueId;
typedef const struct CGPUFence* CGPUFenceId;

typedef struct SkrDStorageQueueDescriptor {
    const char8_t* name;
    ESkrDStorageSource source;
    uint16_t capacity;
    ESkrDStoragePriority priority;
    const struct CGPUDevice* gpu_device;
} SkrDStorageQueueDescriptor;

typedef struct SkrDStorageFile SkrDStorageFile;

typedef struct SkrDStorageFileInfo {
    uint64_t file_size;
} SkrDStorageFileInfo;
typedef const SkrDStorageFile* SkrDStorageFileId;
typedef SkrDStorageFileId SkrDStorageFileHandle;

typedef struct SkrDStorageFileRange
{
    SkrDStorageFileHandle file;
    uint64_t offset;
    uint64_t size;
} SkrDStorageFileRange;

typedef struct SkrDStorageEvent SkrDStorageEvent;
typedef struct SkrDStorageEvent* SkrDStorageEventId;

typedef struct SkrDStorageIODescriptor {
    SkrDStorageCompression compression;
    ESkrDStorageSource source_type;
    SkrDStorageMemoryRange source_memory;
    SkrDStorageFileRange source_file;
    uint64_t uncompressed_size;
    void* destination;
    SkrDStorageEventId event;
    const char8_t* name;
} SkrDStorageIODescriptor;

CGPU_API SkrDStorageInstanceId skr_create_dstorage_instance(SkrDStorageConfig* config);
CGPU_API SkrDStorageInstanceId skr_get_dstorage_instnace();
CGPU_API ESkrDStorageAvailability skr_query_dstorage_availability();
CGPU_API void skr_free_dstorage_instance(SkrDStorageInstanceId inst);

CGPU_API SkrDStorageQueueId skr_create_dstorage_queue(const SkrDStorageQueueDescriptor* desc);
CGPU_API void skr_free_dstorage_queue(SkrDStorageQueueId queue);

CGPU_API SkrDStorageFileHandle skr_dstorage_open_file(SkrDStorageInstanceId instance, const char8_t* abs_path);
CGPU_API void skr_dstorage_query_file_info(SkrDStorageInstanceId instance, SkrDStorageFileHandle file, SkrDStorageFileInfo* info);
CGPU_API void skr_dstorage_close_file(SkrDStorageInstanceId instance, SkrDStorageFileHandle file);

CGPU_API SkrDStorageEventId skr_dstorage_queue_create_event(SkrDStorageQueueId queue);
CGPU_API bool skr_dstorage_event_test(SkrDStorageEventId event);
CGPU_API void skr_dstorage_queue_free_event(SkrDStorageQueueId queue, SkrDStorageEventId);

CGPU_API void skr_dstorage_queue_submit(SkrDStorageQueueId queue, SkrDStorageEventId event);
CGPU_API void skr_dstorage_enqueue_request(SkrDStorageQueueId queue, const SkrDStorageIODescriptor* desc);

#ifdef __cplusplus
} // extern "C"
#endif

#ifdef _WIN32

typedef struct skr_win_dstorage_decompress_service_t skr_win_dstorage_decompress_service_t;
typedef struct skr_win_dstorage_decompress_service_t* skr_win_dstorage_decompress_service_id;
SKR_DECLARE_TYPE_ID_FWD(skr, JobQueue, skr_job_queue)

#define SKR_WIN_DSTORAGE_COMPRESSION_TYPE_IMAGE SKR_DSTORAGE_COMPRESSION_CUSTOM + 1

typedef enum EWinDStorageDecompressionFlag {
    WIN_DSTORAGE_DECOMPRESSION_FLAG_NONE = 0x00,
    WIN_DSTORAGE_DECOMPRESSION_FLAG_DEST_IN_UPLOAD_HEAP = 0x01,

    WIN_DSTORAGE_DECOMPRESSION_FLAG_MAX_ENUM_BIT = 0xFFFFFFFF
} EWinDStorageDecompressionFlag;
typedef uint32_t WinDStorageDecompressionFlags;

typedef struct skr_win_dstorage_decompress_desc_t {
    skr_job_queue_id job_queue SKR_IF_CPP(= nullptr);
} skr_win_dstorage_decompress_desc_t;

typedef struct skr_win_dstorage_decompress_request_t {
    uint64_t id;
    SkrDStorageCompression compression;
    WinDStorageDecompressionFlags flags;
    uint64_t src_size;
    void const* src_buffer;
    uint64_t dst_size;
    void* dst_buffer;
} skr_win_dstorage_decompress_request_t;

#ifndef _HRESULT_DEFINED
#define _HRESULT_DEFINED
typedef long HRESULT;
#endif
typedef HRESULT (*skr_win_dstorage_decompress_callback_t)(skr_win_dstorage_decompress_request_t* request, void* user_data);

// This operation is unsafe and may result in driver corruption.
// Do not call this unless you are sure the backend queues are all idle.
SKR_EXTERN_C CGPU_API
void skr_win_dstorage_set_staging_buffer_size(uint64_t size);

SKR_EXTERN_C CGPU_API
skr_win_dstorage_decompress_service_id skr_win_dstorage_create_decompress_service(const skr_win_dstorage_decompress_desc_t* desc);

SKR_EXTERN_C CGPU_API
bool skr_win_dstorage_decompress_service_register_callback(skr_win_dstorage_decompress_service_id service, SkrDStorageCompression, skr_win_dstorage_decompress_callback_t, void*);

SKR_EXTERN_C CGPU_API
void skr_win_dstorage_free_decompress_service(skr_win_dstorage_decompress_service_id service);

#endif