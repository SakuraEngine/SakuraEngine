#pragma once
#include "skr_lightning_storage.configure.h"
#include "utils/types.h"

#define DECLARE_LIGHTNING_OBJECT(object) typedef struct object object; typedef struct object* object##Id;

DECLARE_LIGHTNING_OBJECT(SLightningEnvironment)
DECLARE_LIGHTNING_OBJECT(SLightningStorage)
typedef struct SLightningStorageOpenDescriptor SLightningStorageOpenDescriptor;

typedef enum ELightningStorageOpenFlag
{
    LIGHTNING_STORAGE_OPEN_READ_WRITE = 0x00000000,
    LIGHTNING_STORAGE_OPEN_READ_ONLY = 0x00000001,
    LIGHTNING_STORAGE_OPEN_CREATE = 0x00000002,
    LIGHTNING_STORAGE_OPEN_TRUNCATE = 0x00000004,

    LIGHTNING_STORAGE_OPEN_TRY_ONCE = 0x00000008,
    LIGHTNING_STORAGE_OPEN_TRY_TIMEOUT = 0x00000010,

    LIGHTNING_STORAGE_OPEN_MAX_ENUM_BIT = 0x7FFFFFFF
} ELightningStorageOpenFlag;
typedef uint32_t LightningStorageOpenFlags;

SKR_LIGHTNING_STORAGE_EXTERN_C SKR_LIGHTNING_STORAGE_API
SLightningEnvironmentId skr_lightning_storage_create_environment(const char* name);
SKR_LIGHTNING_STORAGE_EXTERN_C SKR_LIGHTNING_STORAGE_API
void skr_lightning_storage_free_environment(SLightningEnvironmentId environment);

SKR_LIGHTNING_STORAGE_EXTERN_C SKR_LIGHTNING_STORAGE_API
SLightningStorageId skr_open_lightning_storage(SLightningEnvironmentId environment, const struct SLightningStorageOpenDescriptor* desc);
SKR_LIGHTNING_STORAGE_EXTERN_C SKR_LIGHTNING_STORAGE_API
void skr_close_lightning_storage(SLightningStorageId storage);

// lightning storage objects

struct SLightningEnvironment
{
    struct MDB_env* env;
};

struct SLightningStorage
{
    uint64_t mdbi;
    struct STimer* open_timer;
    uint32_t timeout_ms;
};

// lightning storage descriptors

struct SLightningStorageOpenDescriptor
{
    const char* name;
    LightningStorageOpenFlags flags;
    uint32_t timeout_ms;
};