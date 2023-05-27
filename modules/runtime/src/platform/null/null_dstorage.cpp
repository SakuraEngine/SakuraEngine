#include "platform/dstorage.h"

RUNTIME_API SkrDStorageInstanceId skr_create_dstorage_instance(SkrDStorageConfig* config)
{
    return nullptr;
}

RUNTIME_API SkrDStorageInstanceId skr_get_dstorage_instnace()
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

RUNTIME_API SkrDStorageQueueId skr_create_dstorage_queue(const SkrDStorageQueueDescriptor* desc)
{
    return nullptr;
}

RUNTIME_API void skr_free_dstorage_queue(SkrDStorageQueueId queue)
{

}

RUNTIME_API SkrDStorageFileHandle skr_dstorage_open_file(SkrDStorageQueueId queue, const char* abs_path)
{
    return nullptr;
}

RUNTIME_API void skr_dstorage_query_file_info(SkrDStorageQueueId queue, SkrDStorageFileHandle file, SkrDStorageFileInfo* info)
{

}

RUNTIME_API void skr_dstorage_close_file(SkrDStorageQueueId queue, SkrDStorageFileHandle file)
{

}