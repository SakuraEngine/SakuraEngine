#include "SkrGraphics/dstorage.h"

#ifdef __cplusplus
extern "C" {
#endif

SkrDStorageInstanceId skr_create_dstorage_instance(SkrDStorageConfig* config)
{
    return nullptr;
}

SkrDStorageInstanceId skr_get_dstorage_instnace()
{
    return nullptr;
}

ESkrDStorageAvailability skr_query_dstorage_availability()
{
    return SKR_DSTORAGE_AVAILABILITY_NONE;
}

void skr_free_dstorage_instance(SkrDStorageInstanceId inst)
{

}

SkrDStorageQueueId skr_create_dstorage_queue(const SkrDStorageQueueDescriptor* desc)
{
    return nullptr;
}

void skr_free_dstorage_queue(SkrDStorageQueueId queue)
{

}

SkrDStorageFileHandle skr_dstorage_open_file(SkrDStorageInstanceId instance, const char8_t* abs_path)
{
    return nullptr;
}

void skr_dstorage_query_file_info(SkrDStorageInstanceId instance, SkrDStorageFileHandle file, SkrDStorageFileInfo* info)
{

}

void skr_dstorage_close_file(SkrDStorageInstanceId instance, SkrDStorageFileHandle file)
{

}

SkrDStorageEventId skr_dstorage_queue_create_event(SkrDStorageQueueId queue)
{
    return nullptr;
}

bool skr_dstorage_event_test(SkrDStorageEventId event)
{
    return false;
}

void skr_dstorage_queue_free_event(SkrDStorageQueueId queue, SkrDStorageEventId)
{

}

void skr_dstorage_queue_submit(SkrDStorageQueueId queue, SkrDStorageEventId event)
{

}

void skr_dstorage_enqueue_request(SkrDStorageQueueId queue, const SkrDStorageIODescriptor* desc)
{

}

#ifdef TRACY_PROFILE_DIRECT_STORAGE
void skr_dstorage_queue_trace_submit(SkrDStorageQueueId queue)
{

}
#endif

#ifdef __cplusplus
} // extern "C"
#endif