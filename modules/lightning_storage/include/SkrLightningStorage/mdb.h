#pragma once
#include "SkrLightningStorage/module.configure.h"
#include "SkrRT/misc/types.h"

#define DECLARE_LIGHTNING_OBJECT(object) typedef struct object object; typedef struct object* object##Id;

DECLARE_LIGHTNING_OBJECT(SLightningEnvironment)
DECLARE_LIGHTNING_OBJECT(SLightningStorage)
DECLARE_LIGHTNING_OBJECT(SLightningTXN)
typedef struct SLightningStorageOpenDescriptor SLightningStorageOpenDescriptor;

typedef enum ELightningStorageOpenFlag
{
    LIGHTNING_STORAGE_OPEN_READ_WRITE = 0x00000000,
    LIGHTNING_STORAGE_OPEN_READ_ONLY = 0x00000001,
    LIGHTNING_STORAGE_OPEN_CREATE = 0x00000002,
    LIGHTNING_STORAGE_OPEN_TRUNCATE = 0x00000004,

    // LIGHTNING_STORAGE_OPEN_TRY_ONCE = 0x00000008,
    // LIGHTNING_STORAGE_OPEN_TRY_TIMEOUT = 0x00000010,

    LIGHTNING_STORAGE_OPEN_MAX_ENUM_BIT = 0x7FFFFFFF
} ELightningStorageOpenFlag;
typedef uint32_t LightningStorageOpenFlags;

typedef enum ELightningTransactionOpenFlag
{
    LIGHTNING_TRANSACATION_OPEN_READ_WRITE = 0x00000000,
    LIGHTNING_TRANSACATION_OPEN_READ_ONLY = 0x20000,

    LIGHTNING_TRANSACTION_OPEN_MAX_ENUM_BIT = 0x7FFFFFFF
} ELightningTransactionOpenFlag;
typedef uint32_t ELightningTransactionOpenFlags;

SKR_LIGHTNING_STORAGE_EXTERN_C SKR_LIGHTNING_STORAGE_API
SLightningEnvironmentId skr_lightning_environment_create(const char8_t* name);

SKR_LIGHTNING_STORAGE_EXTERN_C SKR_LIGHTNING_STORAGE_API
void skr_lightning_environment_free(SLightningEnvironmentId environment);

SKR_LIGHTNING_STORAGE_EXTERN_C SKR_LIGHTNING_STORAGE_API
SLightningStorageId skr_lightning_storage_open(SLightningEnvironmentId environment, const struct SLightningStorageOpenDescriptor* desc);

SKR_LIGHTNING_STORAGE_EXTERN_C SKR_LIGHTNING_STORAGE_API
void skr_lightning_storage_close(SLightningStorageId storage);

SKR_LIGHTNING_STORAGE_EXTERN_C SKR_LIGHTNING_STORAGE_API
SLightningTXNId skr_lightning_transaction_open(SLightningEnvironmentId env, SLightningTXNId parent, ELightningTransactionOpenFlags flags);

SKR_LIGHTNING_STORAGE_EXTERN_C SKR_LIGHTNING_STORAGE_API
bool skr_lightning_storage_read(SLightningTXNId txn, SLightningStorageId storage, const struct SLightningStorageValue* key, struct SLightningStorageValue* value);

SKR_LIGHTNING_STORAGE_EXTERN_C SKR_LIGHTNING_STORAGE_API
bool skr_lightning_storage_write(SLightningTXNId txn, SLightningStorageId storage, const struct SLightningStorageValue* key, const struct SLightningStorageValue* value);

SKR_LIGHTNING_STORAGE_EXTERN_C SKR_LIGHTNING_STORAGE_API
bool skr_lightning_storage_del(SLightningTXNId txn, SLightningStorageId storage, const struct SLightningStorageValue* key);

SKR_LIGHTNING_STORAGE_EXTERN_C SKR_LIGHTNING_STORAGE_API
bool skr_lightning_transaction_commit(SLightningTXNId txn);

// lightning storage objects
typedef struct SLightningStorageValue
{
    uint64_t size;
    const void* data;
} SLightningStorageValue;

typedef struct SLightningEnvironment
{
    struct MDB_env* env;

    inline static SLightningEnvironment* Open(const char8_t* name) SKR_NOEXCEPT { return skr_lightning_environment_create(name); } 
    inline static void Free(SLightningEnvironment* env) SKR_NOEXCEPT { return skr_lightning_environment_free(env); } 
    
    inline SLightningStorageId open_storage(const SLightningStorageOpenDescriptor* desc) SKR_NOEXCEPT { return skr_lightning_storage_open(this, desc); }
    inline void close_storage(SLightningStorageId storage) SKR_NOEXCEPT { skr_lightning_storage_close(storage); }
   
    inline SLightningTXNId open_transaction(SLightningTXNId parent, ELightningTransactionOpenFlags flags) SKR_NOEXCEPT { return skr_lightning_transaction_open(this, parent, flags); }
    inline bool commit_transaction(SLightningTXNId txn) SKR_NOEXCEPT { return skr_lightning_transaction_commit(txn); } 
} SLightningEnvironment;

typedef struct SLightningStorage
{
    SLightningEnvironment* env;
    uint64_t mdbi;

    inline SLightningStorageValue read(SLightningTXNId txn, SLightningStorageValue key) SKR_NOEXCEPT { SLightningStorageValue value; skr_lightning_storage_read(txn, this, &key, &value); return value; }
    inline bool write(SLightningTXNId txn, SLightningStorageValue key, SLightningStorageValue value) SKR_NOEXCEPT { return skr_lightning_storage_write(txn, this, &key, &value); }
    inline bool del(SLightningTXNId txn, SLightningStorageValue key) SKR_NOEXCEPT { return skr_lightning_storage_del(txn, this, &key); }
} SLightningStorage;

// lightning storage descriptors

typedef struct SLightningStorageOpenDescriptor
{
    const char8_t* name;
    LightningStorageOpenFlags flags;
} SLightningStorageOpenDescriptor;