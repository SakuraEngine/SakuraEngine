#include "platform/dstorage.h"

RUNTIME_API SkrDStorageInstanceId skr_create_dstorage_instance(SkrDStorageConfig* config)
{
    return nullptr;
}

RUNTIME_API ESkrDStorageAvailability skr_query_dstorage_availability()
{
    return SKR_DSTORAGE_AVAILABILITY_NONE;
}

RUNTIME_API void skr_free_dstorage_instance(SkrDStorageInstanceId inst)
{

}