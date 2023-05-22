#pragma once
#include "platform/configure.h"

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
} SkrDStorageQueueDescriptor;

typedef struct SkrDStorageFile SkrDStorageFile;

typedef struct SkrDStorageFileInfo {
    uint64_t file_size;
} SkrDStorageFileInfo;

typedef const SkrDStorageFile* SkrDStorageFileId;
typedef SkrDStorageFileId SkrDStorageFileHandle;

RUNTIME_API SkrDStorageInstanceId skr_create_dstorage_instance(SkrDStorageConfig* config);
RUNTIME_API SkrDStorageInstanceId skr_get_dstorage_instnace();
RUNTIME_API ESkrDStorageAvailability skr_query_dstorage_availability();
RUNTIME_API void skr_free_dstorage_instance(SkrDStorageInstanceId inst);

RUNTIME_API SkrDStorageQueueId skr_create_dstorage_queue(const SkrDStorageQueueDescriptor* desc);
RUNTIME_API void skr_free_dstorage_queue(SkrDStorageQueueId queue);

RUNTIME_API SkrDStorageFileHandle skr_dstorage_open_file(SkrDStorageQueueId queue, const char* abs_path);
RUNTIME_API void skr_dstorage_query_file_info(SkrDStorageQueueId queue, SkrDStorageFileHandle file, SkrDStorageFileInfo* info);
RUNTIME_API void skr_dstorage_close_file(SkrDStorageQueueId queue, SkrDStorageFileHandle file);


#ifdef __cplusplus
} // extern "C"
#endif