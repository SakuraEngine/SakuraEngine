#pragma once
#include "platform/guid.h"

typedef struct skr_resource_handle {
    union
    {
        skr_guid_t guid;
        void* pointer;
    };
} skr_resource_handle;

typedef struct skr_resource_header {
    uint32_t version;
    uint32_t type;
    skr_resource_handle* dependencies;
    uint32_t dependencyCount;
} skr_resource_header;
